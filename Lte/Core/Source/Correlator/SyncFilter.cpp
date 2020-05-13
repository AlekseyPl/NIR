/*
 * filter.cpp
 *
 *  Created on: 25.02.2015
 *      Author: dblagov
 */
#include "Correlator/SyncFilter.h"
#include <Math/ComplexMath.h>
#include <algorithm>
#include <cstring>

namespace Lte {

Filter::Filter( uint32_t nFFT ) :
	  decimFactor(LTEDownFactorSync),
	  sliceCount(0), winSize(FilterAfr::mFilterSize),
	  fullSize(LTEFrameLength/2 + LTESlotLength + FilterAfr::mFilterSize), fftSize(nFFT),
	  blockSize(fftSize-2*winSize+2),
	  winFft(fftSize), sigFftSlice(fftSize),
	  sigSlice(fftSize), sliceResT(fftSize), sliceResF(fftSize),
	  fft(std::make_shared<Math::FftSP>(fftSize))
{
	PrepareWin();
	sliceCount = (fullSize)/ (blockSize)+1;
	iq.resize(fullSize);
	filteredSig.resize(fullSize);
}

void Filter::SetDecimFactor(uint32_t decimFactor_)
{
	decimFactor = decimFactor_;
}

void Filter::Process( const Complex16* in, uint32_t step )
{
	Complex16* inPtr = const_cast<Complex16*>(in);
	Math::ConvertShortToFloat( inPtr, iq.data(), fullSize, step);

	uint32_t progress = 0;
	for( uint32_t s = 0; s < sliceCount; ++s ) {

		uint32_t copySize = ((fullSize-progress) >= blockSize ) ? blockSize : (fullSize-progress);
		PrepareSigHalfZero(iq.data()+progress, copySize);

		ClearSlice(sigFftSlice);
		fft->DoIt(sigSlice.data(), sigFftSlice.data());

		ClearSlice(sliceResF);
		std::transform(sigFftSlice.begin(), sigFftSlice.end(), winFft.begin(), sliceResF.begin(),
					   [](ComplexFloat v1, ComplexFloat v2) { return v1*v2;} );

		ClearSlice(sliceResT);
		fft->Undo(sliceResF.data(), sliceResT.data());

		if( s == 0 )
			std::memcpy(filteredSig.data(), sliceResT.data(), (blockSize+winSize-1)*sizeof(ComplexFloat));
		else if( s == sliceCount - 1) {
			uint32_t idx = progress-(winSize-1);
			for( uint32_t i = 0; i < winSize-1; ++i) // overlap and add
				filteredSig[idx+i] += sliceResT[i];

			uint32_t lastCorrCount = fullSize-progress;

			std::memcpy(filteredSig.data()+progress, sliceResT.data()+winSize-1, (lastCorrCount)*sizeof(ComplexFloat));

		}
		else {
			uint32_t idx = progress-(winSize-1);
			for( uint32_t i = 0; i < winSize-1; ++i) // overlap and add
				filteredSig[idx+i] += sliceResT[i];

			std::memcpy(filteredSig.data()+progress, sliceResT.data()+winSize-1, (blockSize)*sizeof(ComplexFloat));
		}
		progress += blockSize;
	}


}

void Filter::GetFilteredData(ComplexFloat *out, uint32_t startPos, uint32_t stopPos, uint32_t decFactor)
{
	uint32_t count = stopPos-startPos;

	if(startPos < stopPos) {
		uint32_t offset = winSize-1 + startPos;
		for(uint32_t i = 0; i < count; ++i)
			out[i] = filteredSig[offset+i*decFactor];
	}
	else {
		uint32_t startPos0 = winSize-1 + startPos;
		uint32_t count0 = fullSize - startPos0;

		uint32_t startPos1 = winSize-1 + startPos;
		uint32_t count1 = startPos1 + stopPos;

		uint32_t idx = 0;
		for(; idx < count0; ++idx)
			out[idx] = filteredSig[startPos0+idx*decFactor];

		for(; idx < count1; ++idx)
			out[idx] = filteredSig[startPos1+idx*decFactor];
	}

}

uint32_t Filter::GetFilterSize() const
{
	return FilterAfr::mFilterSize;
}

void Filter::PrepareWin()
{
	std::vector<ComplexFloat> winSlice(fftSize);

	ClearSlice(winSlice);
	std::copy( afr.filterCoef.begin(), afr.filterCoef.end(), winSlice.begin());

	ClearSlice(winFft);
	fft->DoIt(winSlice.data(), winFft.data());
}

inline void Filter::PrepareSigHalfZero(const ComplexFloat *sig, uint32_t count)
{
	sigSlice.assign(sigSlice.size(), ComplexFloat());
	std::memcpy( sigSlice.data()+winSize-1, sig, count*sizeof(ComplexFloat));
}


inline void Filter::ClearSlice(std::vector<ComplexFloat>& v)
{
	v.assign(v.size(), ComplexFloat());
}

}

