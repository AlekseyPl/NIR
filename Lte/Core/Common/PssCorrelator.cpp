#include "Common/PssCorrelator.h"
#include <Math/FftSP.h>
#include <string>
#include <algorithm>

namespace Lte {

PssCorrelator::PssCorrelator(uint32_t corrSize):
	  zero(ComplexFloat{0.0,0.0}),
	  fft(std::make_shared<Math::FftSP>()),
	  sliceCount(0), winSize(SyncCode::FFTLEN),
	  corrSize(corrSize), fftSize(winSize<<1),
	  winSlice(fftSize), winFftSlice(fftSize), sigFftSlice(fftSize),
	  sigSlice(fftSize), sliceResT(fftSize), sliceResF(fftSize),
	  result(corrSize)
{
	fft->Init(fftSize);

	sliceCount = corrSize / (fftSize/2);// + 1;
}

void PssCorrelator::Configure(uint32_t nid2)
{
	auto& code = pss.GetCode( nid2 );
	PrepareWin(code);
}

void PssCorrelator::Correlate( const ComplexFloat *sig, float *out)
{
	// start do out cycle;
	uint32_t progress = 0;

	for( uint32_t s = 0; s < sliceCount; ++s ) {

		PrepareSig(&sig[progress]);

		ClearSlice(sigFftSlice);
		fft->DoIt(sigSlice.data(), sigFftSlice.data());

		ClearSlice(sliceResF);
		std::transform(sigFftSlice.begin(),sigFftSlice.end(),winFftSlice.begin(), sliceResF.begin(),
								   [](ComplexFloat v1, ComplexFloat v2) { return conj_mpy(v1, v2);} );


		ClearSlice(sliceResT);
		fft->Undo(sliceResF.data(), sliceResT.data());

		if( s == 0 ) {
			std::copy(sliceResT.begin(),sliceResT.end(),result.begin());
		}
		else if( s == sliceCount - 1) {
			for( uint32_t t = progress, i =0; t < progress + fftSize/2; ++t, ++i)
				result[t] += sliceResT[i];
		}
		else {
			for( uint32_t t = progress, i =0; t < progress + fftSize/2; ++t, ++i)
				result[t] += sliceResT[i];
			for( uint32_t t = progress + fftSize/2, i =fftSize/2; t < progress + fftSize; ++t, ++i)
				result[t] = sliceResT[i];

		}
		progress += fftSize/2;
	}
	for(const auto& r : result)
		*out++ = abs(r);
}


void PssCorrelator::PrepareWin(std::vector<ComplexFloat>& win)
{
	// prepare win slice
	ClearSlice(winSlice);
	std::copy( win.begin(), win.end(), winSlice.begin());

	ClearSlice(winFftSlice);
	fft->DoIt(winSlice.data(), winFftSlice.data());
}

void PssCorrelator::PrepareSig(const ComplexFloat *sig)
{
	// prepare win slice
	std::copy( sig, sig+fftSize, sigSlice.begin());
}

void PssCorrelator::PrepareSigHalfZero(const ComplexFloat *sig)
{
	sigSlice.assign(sigSlice.size(), zero);
	std::copy( sig, sig+fftSize/2, sigSlice.begin());
}

inline void PssCorrelator::ClearSlice(std::vector<ComplexFloat>& v)
{
	v.assign(v.size(),zero);
}


}
