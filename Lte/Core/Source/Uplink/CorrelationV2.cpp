
#include <IntMath/Util.h>
#include <Math/Peak.h>
#include <Math/FftSP.h>
#include <Math/ComplexMath.h>
#include <System/DebugInfo.h>

#include "Lte/Core/Uplink/CorrelationV2.h"
#include "Lte/Core/Common/LteDemConst.h"

namespace {

void Multiply(const ComplexFloat * __restrict__ in0, const ComplexFloat * __restrict__ in1, ComplexFloat * __restrict__ out,
              uint32_t size, uint32_t startSC, uint32_t countSC);

}  // namespace

namespace Lte {

CorrelationV2::CorrelationV2(uint32_t numChannels, uint32_t gapCorr, Common::Allocator &allocator) :
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
      fftSp(new Math::FftSP(allocator)),
    file("lteCorrRes.dat")
{
    for (uint32_t i = 0; i < numChannels; ++i) {
        fftOut[i] = new Common::Buffer<ComplexFloat>(lenCorr, allocator);
    }

    for (uint32_t i = 0; i < numCorrelations; ++i) {
        corr[i] = new Common::Buffer<float>(gapCorr << 1, allocator);
    }

    fftSp->Init(lenCorr);
}

CorrelationV2::~CorrelationV2()
{
    for (uint32_t i = 0; i < numCorrelations; ++i) {
        delete corr[i];
    }

    for (uint32_t i = 0; i < fftOut.Size(); ++i) {
        delete fftOut[i];
    }
}

void CorrelationV2::Do(Complex16* in, uint32_t stride, uint32_t startSC, uint32_t countSC )
{
    Complex16* ptr = in;

    startSC = startSC * (lenCorr / Lte::LTEBaseSymbolLen) + (lenCorr >> 1); //  + (lenCorr / 2) - чтобы не делать fftshift
    countSC = countSC * (lenCorr / Lte::LTEBaseSymbolLen);

    for (uint32_t chI = 0; chI < numChannels; ++chI) {
        Complex16* ptr1 = ptr + chI;
        Math::ConvertShortToFloat(ptr1, tmp0.Ptr(), lenData, numChannels);
        tmp0.Set(ComplexFloat(), lenData, lenCorr - lenData);
        fftSp->DoIt(tmp0.Ptr(),fftOut[chI]->Ptr());
    }

    uint32_t idx = 0;
    for (uint32_t chI = 0; chI < numChannels - 1; ++chI) {
        for (uint32_t chJ = chI + 1; chJ < numChannels; ++chJ) {
            tmp1.Set(ComplexFloat());

            Multiply(fftOut[chI]->Ptr(), fftOut[chJ]->Ptr(), tmp1.Ptr(), lenCorr, startSC, countSC );
            fftSp->DoIt(tmp1.Ptr(), tmp2.Ptr());
            float *ptrCorr = corr[idx]->Ptr();
            for (uint32_t i = lenCorr - gapCorr; i < lenCorr; ++i) {
                *ptrCorr++ = abs(tmp2[i]);
            }
            for (uint32_t i = 0; i < gapCorr; ++i) {
                *ptrCorr++ = abs(tmp2[i]);
            }

            ++idx;
        }
    }

    for (uint32_t idx = 0; idx < corr.Size(); ++idx) {
        uint32_t length = corr[idx]->Size();
        Math::Peak peak = Math::GetPeak(corr[idx]->Ptr(), length);
        double corr0 = 0.0;
        double corr1 = peak.max;
        double corr2 = 0.0;

        corr0 = (peak.pos == 0) ? (*corr[idx])[length - 1] : (*corr[idx])[peak.pos - 1];
        corr2 = (peak.pos == length -1 ) ? (*corr[idx])[0] : (*corr[idx])[peak.pos + 1];

        result[idx].quality = peak.quality;
        result[idx].pos = peak.pos + Math::EstimateMaxPos(corr0, corr1, corr2) - gapCorr;
    }
}

} // namespace WiFi

namespace {



void Multiply(const ComplexFloat * __restrict__ in0, const ComplexFloat * __restrict__ in1, ComplexFloat * __restrict__ out,
              uint32_t size, uint32_t startSC, uint32_t countSC )
{
    uint32_t maskSize = size - 1;
    uint32_t idx = startSC & maskSize;
    for( uint32_t i = 0; i < countSC; ++i ) {
        out[idx] = conj_mpy(in0[idx], in1[idx]);
        idx = (++idx) & maskSize;
    }
}

}  // namespace
