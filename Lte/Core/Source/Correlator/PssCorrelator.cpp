#include "Correlator/PssCorrelator.h"
#include <Math/FftSP.h>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <System/DebugInfo.h>
#include <fstream>
namespace Lte {

PssCorrelator::PssCorrelator(uint32_t corrSize):
	  zero(ComplexFloat{0.0,0.0}),
	  fft(std::make_shared<Math::FftSP>()),
	  sliceCount(0), winSize(SyncCode::FFTLEN),
	  corrSize(corrSize), fftSize(2048),
	  blockSize(fftSize-2*winSize+2),
	  sigFftSlice(fftSize),
	  sigSlice(fftSize), sliceResT(fftSize), sliceResF(fftSize)
{
	fft->Init(fftSize);
	for(auto& win: winFftSlice)		win.resize(fftSize);

	PrepareWin();

	sliceCount = (corrSize)/ (blockSize)+1;
	for(auto& c : corr)
		c.resize(corrSize);

	for(auto& a : absCorr)
		a.resize(corrSize);

	sigAmps.resize(corrSize + SyncCode::FFTLEN);
}

//const PssCorrelator::PssRes PssCorrelator::GetCorrMaxPos(uint32_t& resNid2)
//{
//    PssRes res;

//    float pssMax = 0;
//    for( uint32_t nid2 = 0 ; nid2 < SyncCode::PSS_COUNT; ++nid2 ) {
//        auto maxptr = max_element( absCorr.at(nid2).begin(), absCorr.at(nid2).end() );
//        if( *maxptr > pssMax ) {
//            res.val = *maxptr;
//            resNid2 = nid2;
//            res.pos = std::distance(absCorr.at(nid2).begin(), maxptr);
//        }
//    }


//	return res;
//}

const std::array<std::vector<float>, SyncCode::PSS_COUNT>& PssCorrelator::GetCorrRes() {

    return absCorr;
}

void PssCorrelator::ClearCorr(const uint32_t &resNid2,const uint32_t &pos, int32_t deltaPos)
{
    int32_t startPos = pos - deltaPos;
     if( startPos < 0 ) startPos = 0;

     int32_t stopPos = pos + deltaPos;
     if( stopPos >= corrSize ) stopPos = corrSize - 1;



     memset( absCorr.at(resNid2).data() + startPos, 0, ( stopPos - startPos ) * sizeof( float ) );
}

void PssCorrelator::Correlate( const ComplexFloat *sig, uint32_t nid2 )
{
	for(auto& c : corr)
		ClearSlice(c);

	CalcPowers(sig);

	std::vector<uint32_t> nid2Vec;
	if(nid2 != SyncCode::PSS_COUNT)		nid2Vec.push_back(nid2);
	else		nid2Vec = {0, 1, 2};


	uint32_t progress = 0;
	for( uint32_t s = 0; s < sliceCount; ++s ) {

		uint32_t copySize = ((corrSize-progress) >= blockSize ) ? blockSize : (corrSize-progress);
		PrepareSigHalfZero(&sig[progress], copySize);

		ClearSlice(sigFftSlice);
		fft->DoIt(sigSlice.data(), sigFftSlice.data());

		for(const auto& nid2 : nid2Vec)
			CorrSpecifyNid(s, nid2, progress);

		progress += blockSize;
	}

	for(const auto& nid2 : nid2Vec) {
		const auto& c = corr.at(nid2);
		auto& absCorrRes = absCorr.at(nid2);

		auto codeAmp = pss.GetAmp(nid2);
		std::transform(c.begin(), c.end(), sigAmps.begin(), absCorrRes.begin(),
					   [codeAmp](ComplexFloat v1, float v2) { return Math::Div(abs(v1), v2*codeAmp);} );
	}
}


void PssCorrelator::CorrSpecifyNid(uint32_t sliceCntr, uint32_t nid2, uint32_t progress)
{
	const auto& win = winFftSlice.at(nid2);
	auto& cor = corr.at(nid2);

	ClearSlice(sliceResF);
	std::transform(sigFftSlice.begin(), sigFftSlice.end(), win.begin(), sliceResF.begin(),
				   [](ComplexFloat v1, ComplexFloat v2) { return conj_mpy(v1, v2);} );

	ClearSlice(sliceResT);
	fft->Undo(sliceResF.data(), sliceResT.data());

	if( sliceCntr == 0 )
		std::memcpy(cor.data(), sliceResT.data(), (blockSize+winSize-1)*sizeof(ComplexFloat));
	else if( sliceCntr == sliceCount - 1) {
		uint32_t idx = progress-(winSize-1);
		for( uint32_t i = 0; i < winSize-1; ++i) // overlap and add
			cor[idx+i] += sliceResT[i];

		uint32_t lastCorrCount = corrSize-progress;

		std::memcpy(cor.data()+progress, sliceResT.data()+winSize-1, (lastCorrCount)*sizeof(ComplexFloat));

	}
	else {
		uint32_t idx = progress-(winSize-1);
		for( uint32_t i = 0; i < winSize-1; ++i) // overlap and add
			cor[idx+i] += sliceResT[i];

		std::memcpy(cor.data()+progress, sliceResT.data()+winSize-1, (blockSize)*sizeof(ComplexFloat));
	}
}

void PssCorrelator::CorrelateThin(const ComplexFloat *signal, float* corr, uint32_t nid2, uint32_t gap)
{
	const auto& code = pss.GetCode(nid2);
	auto codeAmp = pss.GetAmp(nid2);
	for(uint32_t track = 0; track < LTEDownFactorSync; ++track) {
		for(uint32_t iq = 0; iq < gap/LTEDownFactorSync; ++iq) {

			ComplexFloat corrVal{.0f, .0f};
			float power = 0.0f;
			for(uint32_t i = 0; i < SyncCode::FFTLEN;++i) {
				uint32_t idx = (iq + i) * LTEDownFactorSync + track;
				corrVal += conj_mpy( signal[idx], code[i]);
				power += abs2(signal[idx]);
			}
			uint32_t idx = iq*LTEDownFactorSync + track;
			corr[idx] = abs(corrVal)/(Math::Sqrt(power)*codeAmp);
		}
	}
}


void PssCorrelator::PrepareWin()
{
	// prepare win fft slice
	std::vector<ComplexFloat> winSlice(fftSize);
	for(uint32_t i = 0; i < SyncCode::PSS_COUNT; ++i) {
		auto& win = winFftSlice.at(i);
		auto& code = pss.GetCode( i );

		ClearSlice(winSlice);
		std::copy( code.begin(), code.end(), winSlice.begin());

		ClearSlice(win);
		fft->DoIt(winSlice.data(), win.data());
	}
}

void PssCorrelator::PrepareSigHalfZero(const ComplexFloat *sig, uint32_t count)
{
	sigSlice.assign(sigSlice.size(), zero);
	std::memcpy( sigSlice.data()+winSize-1, sig, count*sizeof(ComplexFloat));
}


inline void PssCorrelator::ClearSlice(std::vector<ComplexFloat>& v)
{
	v.assign(v.size(),zero);
}

void PssCorrelator::CalcPowers(const ComplexFloat *sig)
{
	for(auto& p : sigAmps)
		p = abs2(*sig++);

	auto accum = [](float acc, float val) { return acc + val;};
	float ampSum = std::accumulate( sigAmps.begin(), sigAmps.begin()+SyncCode::FFTLEN, 0.0f, accum);

	float prevAmp = sigAmps[0];
	sigAmps[ 0 ] = ampSum;
	for( uint32_t sum = 1; sum < corrSize; ++sum ) {
		ampSum = sigAmps[ sum - 1 ] - prevAmp + sigAmps[ sum + SyncCode::FFTLEN - 1 ];
		prevAmp = sigAmps[ sum ];
		sigAmps[ sum ] = ampSum;
	}

	for(auto& p : sigAmps)
		p = Math::Sqrt(p);
}

}
