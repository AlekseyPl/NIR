/*
 *  Correlation.cpp
 *
 *  Created on: Jul 9, 2018
 *      Author: aplotnikov
 */

#include <IntMath/Util.h>
#include <IntMath/EstimateMaxPos.h>
#include <IntMath/Peak.h>
#include <IntMath/Fft32.h>
#include <System/DebugInfo.h>
#include "Lte/Core/Uplink/Correlation.h"
#include "Lte/Core/Common/LteDemConst.h"

namespace {
uint32_t Average(const Complex16 * __restrict__ in, uint32_t logSize, uint32_t stride);
void Multiply(const Complex32 * __restrict__ in0, const Complex32 * __restrict__ in1, Complex32 * __restrict__ out,
			  uint32_t size, uint32_t startSC, uint32_t countSC );
void Shift(const Complex16 * __restrict__ in, Complex32 * __restrict__ out, uint32_t size, int32_t shift,
		   uint32_t stride);

}  // namespace

namespace Lte {

Correlation::Correlation(uint32_t numChannels, uint32_t gapCorr, Common::Allocator &allocator) :
	numCorrelations((numChannels - 1) * numChannels >> 1), numChannels(numChannels), //
	lenData(Lte::LTESlotLength * Lte::LTESlotsInSubframe),
	logLenData(IntMath::Log2(lenData)),
	logLenCorr(IntMath::CeilLog2(lenData)),
	lenCorr(1 << logLenCorr), gapCorr(gapCorr),
	tmp0(lenCorr, allocator), tmp1(lenCorr, allocator), tmp2(lenCorr, allocator), //
	fftOut(numChannels, allocator),  //
	corr(numCorrelations, allocator), //
	result(numCorrelations, allocator), //
	freqOffset(numCorrelations, allocator),
    fft(new IntMath::Fft32(lenCorr))
{
	for (uint32_t i = 0; i < numChannels; ++i) {
		fftOut[i] = new Common::Buffer<Complex32>(lenCorr, allocator);
	}

	for (uint32_t i = 0; i < numCorrelations; ++i) {
		corr[i] = new Common::Buffer<uint32_t>(gapCorr << 1, allocator);
    }
}

Correlation::~Correlation() {
	for (uint32_t i = 0; i < numCorrelations; ++i) {
		delete corr[i];
	}

	for (uint32_t i = 0; i < fftOut.Size(); ++i) {
		delete fftOut[i];
	}

	delete fft;
}

void Correlation::Do(Complex16* in, uint32_t stride, uint32_t startSC, uint32_t countSC )
{
    Complex16* ptr = in;

	startSC = startSC * (lenCorr / Lte::LTEBaseSymbolLen) + (lenCorr >> 1); //  + (lenCorr / 2) - чтобы не делать fftshift
	countSC = countSC * (lenCorr / Lte::LTEBaseSymbolLen);

	for (uint32_t chI = 0; chI < numChannels; ++chI) {
		Complex16* ptr1 = ptr + chI;
		uint32_t sigma = Average(ptr1, logLenData, stride);
		int32_t shift = 30 - IntMath::Log2(sigma) - logLenCorr;
        Shift(ptr1, tmp0.Ptr(), lenData, shift, stride);
        tmp0.Set(0, lenData, lenCorr - lenData);
        fft->DoIt(tmp0.Ptr(), fftOut[chI]->Ptr());
        IntMath::Shift(fftOut[chI]->Ptr(), lenCorr, -15);
    }

	uint32_t idx = 0;
    int32_t ifftShift = IntMath::CeilLog2( countSC );

	for (uint32_t chI = 0; chI < numChannels - 1; ++chI) {
        IntMath::Conjugate(fftOut[chI]->Ptr(), tmp0.Ptr(), lenCorr);
		for (uint32_t chJ = chI + 1; chJ < numChannels; ++chJ) {
			tmp1.Set(0);
			Multiply(tmp0.Ptr(), fftOut[chJ]->Ptr(), tmp1.Ptr(), lenCorr, startSC, countSC );
            IntMath::Shift(tmp1.Ptr(), lenCorr, -ifftShift);
            fft->Undo(tmp1.Ptr(), tmp2.Ptr());
			uint32_t *ptrCorr = corr[idx]->Ptr();
			for (uint32_t i = lenCorr - gapCorr; i < lenCorr; ++i) {
				*ptrCorr++ = IntMath::AbsGeron(tmp2[i]);
			}
			for (uint32_t i = 0; i < gapCorr; ++i) {
				*ptrCorr++ = IntMath::AbsGeron(tmp2[i]);
			}

			++idx;
		}
	}

	for (uint32_t idx = 0; idx < corr.Size(); ++idx) {
		uint32_t length = corr[idx]->Size();
		IntMath::Peak peak = IntMath::GetPeak((const int32_t *) corr[idx]->Ptr(), length);
		double corr0 = 0.0;
		double corr1 = peak.max;
		double corr2 = 0.0;
		if (peak.pos == 0) {
			corr0 = (*corr[idx])[length - 1];
		} else {
			corr0 = (*corr[idx])[peak.pos - 1];
		}
		if (peak.pos == length - 1) {
			corr2 = (*corr[idx])[0];
		} else {
			corr2 = (*corr[idx])[peak.pos + 1];
		}

		result[idx].quality = peak.quality;
		result[idx].pos = peak.pos + IntMath::EstimateMaxPos(corr0, corr1, corr2) - gapCorr;
	}
}

} // namespace WiFi

namespace {

uint32_t Average(const Complex16 * __restrict__ in, uint32_t logSize, uint32_t stride) {
	uint64_t sum = 0;
	const int32_t *inPtr = reinterpret_cast<const int32_t *>(in);
#pragma MUST_ITERATE(4,,4);
#pragma UNROLL(4);
	for (uint32_t i = 0; i < (1 << logSize); ++i) {
		int32_t val = *(inPtr);
		inPtr += stride;
		int32_t abs = _dotp2(val, val);
		sum += static_cast<uint64_t>(abs);
	}

	return IntMath::iSqrt(static_cast<uint32_t>(sum >> logSize));
}

void Shift(const Complex16 * __restrict__ in, Complex32 * __restrict__ out, uint32_t size, int32_t shift,
		   uint32_t stride) {
	if (shift > 0) {
		for (uint32_t i = 0; i < size; ++i) {
			*out++ = (Complex32(*in) << shift);
			in += stride;
		}
	} else {
		for (uint32_t i = 0; i < size; ++i) {
			*out++ = (Complex32(*in) >> -shift);
			in += stride;
		}
	}
}

void Multiply(const Complex32 * __restrict__ in0, const Complex32 * __restrict__ in1, Complex32 * __restrict__ out,
			  uint32_t size, uint32_t startSC, uint32_t countSC ) {
	uint32_t maskSize = size - 1;
	uint32_t idx = startSC & maskSize;
	for( uint32_t i = 0; i < countSC; ++i ) {
		out[idx] = in0[idx] * in1[idx];
		idx = (++idx) & maskSize;
	}
}

}  // namespace

