#include "Lte/Core/Correlator/SssCorrelator.h"
#include <System/DebugInfo.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <numeric>
#include <string>
#include <Math/Fftwf.h>

namespace Lte {


SssCorrelator::SssCorrelator():
      startPos0(SyncCode::FFTLEN / 2 - LTESyncCodeHalfLen ),
      startPos1(SyncCode::FFTLEN / 2 + 1 ),
      endPos0(SyncCode::FFTLEN/2  ),
      endPos1(SyncCode::FFTLEN/2 + LTESyncCodeHalfLen ),
      fft128(SyncCode::FFTLEN),
      sssCode( {SecondarySyncCode::sss0Even,SecondarySyncCode::sss0Odd, SecondarySyncCode::sss5Odd, SecondarySyncCode::sss5Even}),
      debug(System::DebugInfo::Locate())
{

	corrRes.resize(LTESyncCodeHalfLen);
	fftCorrRes.resize(LTESyncCodeHalfLen);

    absCorrRes.resize(LTESyncCodeHalfLen);
    tempEven.resize(LTESyncCodeHalfLen);
    tempOdd.resize(LTESyncCodeHalfLen);

	sssSignal.resize(SyncCode::FFTLEN);
	sssSpectrum.resize(SyncCode::FFTLEN);

	searchParams.reserve(VariantsCount);
	for(auto& sp : sssParts)	sp.resize(LTESyncCodeHalfLen);
}

SssCorrelator::~SssCorrelator()
{}

SssCorrelator::SearchResult	SssCorrelator::Do(const ComplexFloat* data, uint32_t nid2_, uint32_t pssPos)
{
	nid2 = nid2_;
	float maxCorr = 0.0;
	int32_t resNid1 = 0;
	uint32_t offset = 0;
	uint32_t sfNum  = 0;
	CyclicPrefix cp = lteCP_Short;
	Duplex		 dx = lteFDD;
	for( const auto& p : searchParams) {
		ExtractSignalSss( data, p, pssPos);

		auto ret = Correlate(nid2);
		int32_t nid1 = m0m1.GetNid1(M0M1(ret.M0,ret.M1));
		if(nid1 != -1) {
			if( ret.corrRes > maxCorr ) {
				maxCorr = ret.corrRes;
				resNid1 = nid1;
				offset  = p.shiftToFramePos;
				sfNum	= ret.subframeNum;
				cp		= p.cp;
				dx		= p.duplex;
			}
		}
	}

	return SearchResult(resNid1, offset, sfNum, cp,dx );
}

SssCorrelator::SssCorrRes	SssCorrelator::Correlate( uint32_t nid2 )
{
    SssCorrRes corr;

    searchRes resultEven;
    searchRes resultOdd;

    auto evenPart = sssParts[0].data();
    sss.DemodCt(evenPart, nid2);
    Math::Fftwf::Do(Math::Fftwf::Params(evenPart, tempEven.data(), tempEven.size()) );

	auto accum = [](float acc, ComplexFloat val) { return acc + abs2(val);};
	float32 accumEven = Math::Sqrt( std::accumulate( tempEven.begin(), tempEven.end(), 0.0f, accum) );

    resultEven =  FindSeq(tempEven, 0);

    corr.M0 = CalcM0(resultEven.place);

    auto oddPart  = sssParts[1].data();
    sss.DemodCtZt(oddPart, nid2, corr.M0);
    Math::Fftwf::Do(Math::Fftwf::Params(oddPart, tempOdd.data(), tempOdd.size()) );

    float32 accumOdd = Math::Sqrt( std::accumulate( tempOdd.begin(), tempOdd.end(), 0.0f, accum) );

	resultOdd =  FindSeq(tempOdd, 1);
	corr.M1 = CalcM1(resultOdd.place);

    if (corr.M0>corr.M1) {
        auto temp1 = corr.M1;
        corr.M1 =  corr.M0;
        corr.M0 = temp1;
        corr.subframeNum = 5;
    }
    else   corr.subframeNum = 0;

    corr.corrRes = Math::Div(resultEven.val,accumEven)+Math::Div(resultOdd.val,accumOdd);
    return corr;
}
void	SssCorrelator::ExtractSignalSss(const ComplexFloat* data, const SearchParams& params, uint32_t pssPos)
{
    uint32_t sssPos = (pssPos - params.shiftPssToSss/LTEDownFactorSync +CorrCount)%CorrCount;
    const ComplexFloat* sssData = data + sssPos;

    sssSignal.clear();
    std::copy(&sssData[0], &sssData[SyncCode::FFTLEN], sssSignal.begin());
    fft128.DoIt(sssSignal.data(),sssSpectrum.data());
    fft128.Shift(sssSpectrum.data(),sssSignal.data());


	for(auto& s: sssParts)
		s.assign(s.size(), ComplexFloat(0.0f, 0.0f));

    auto evenPart = sssParts[0].data();
    auto oddPart  = sssParts[1].data();

    for( uint32_t i = startPos0; i < endPos0; i+=2)
        *evenPart++ = sssSignal[i];
    for( uint32_t i = startPos1+1; i < endPos1; i+=2 )
        *evenPart++ = sssSignal[i];
    for( uint32_t i = startPos0+1; i < endPos0; i+=2)
        *oddPart++ = sssSignal[i];
    for( uint32_t i = startPos1; i < endPos1+1; i+=2 )
        *oddPart++ = sssSignal[i];


}

SssCorrelator::searchRes SssCorrelator::FindSeq(const std::vector<ComplexFloat> &sssSpectrumPart, uint32_t evenOrOdd )
{
	searchRes ResultSpot;
	auto conjmpy = [] (ComplexFloat val1, ComplexFloat val2) { return conj_mpy(val1, val2);};
    auto calcAmp = [] (ComplexFloat val) { return abs(val);};
	auto& code = sss.GetSpecCode(sssCode[evenOrOdd]);
	std::transform(sssSpectrumPart.begin(), sssSpectrumPart.end(), code.begin(), fftCorrRes.begin(), conjmpy);
	Math::Fftwf::Do(Math::Fftwf::Params(fftCorrRes.data(), corrRes.data(), fftCorrRes.size(), Math::Fftwf::dirBackward));

	std::transform(corrRes.begin(), corrRes.end(), absCorrRes.begin(), calcAmp);
#if 0 // Для отладки, вместо 0 писать 1, будет запись в файл
	std::string corrName = "corr"+std::to_string(evenOrOdd)+"_nid2_"+std::to_string(nid2)+".dat";
	std::ofstream file(corrName, std::ofstream::binary);
	file.write(reinterpret_cast<const char*>(absCorrRes.data()), absCorrRes.size()*sizeof (float));
	file.close();
#endif
	auto maxVal = std::max_element(absCorrRes.begin(), absCorrRes.end());
	ResultSpot.val = *maxVal;
	ResultSpot.place = std::distance(absCorrRes.begin(), maxVal );

	return ResultSpot;
}

void	SssCorrelator::Configure(const SearchDepth sd)
{
	searchParams.clear();
	SearchParams param;
	if( sd.fdd_ncp ) {
		param.shiftToFramePos = LTEPssShiftFddS;
		param.shiftPssToSss	  = LTESssShiftFddS;
		param.cp		 	  = lteCP_Short;
		param.duplex	 	  = lteFDD;
		param.string		  = "Frequency-division duplex (FDD), normal CyclicPrefix";
		searchParams.push_back(param);
	}

	if( sd.fdd_ecp ) {
		param.shiftToFramePos = LTEPssShiftFddL;
		param.shiftPssToSss	  = LTESssShiftFddL;
		param.cp		 	  = lteCP_Long;
		param.duplex	 	  = lteFDD;
		param.string		  = "Frequency-division duplex (FDD), long CyclicPrefix";
		searchParams.push_back(param);
	}

	if( sd.tdd_ncp ) {
		param.shiftToFramePos = LTEPssShiftTddS;
		param.shiftPssToSss	  = LTESssShiftTddS;
		param.cp		 	  = lteCP_Short;
		param.duplex	 	  = lteTDD;
		param.string		  = "Time-division duplexing(TDD), normal CyclicPrefix ";
		searchParams.push_back(param);
	}

	if( sd.tdd_ecp ) {
		param.shiftToFramePos = LTEPssShiftTddL;
		param.shiftPssToSss	  = LTESssShiftTddL;
		param.cp		 	  = lteCP_Long;
		param.duplex	      = lteTDD;
		param.string          = "Time-division duplexing(TDD), long CyclicPrefix";
		searchParams.push_back(param);
	}
}

uint32_t SssCorrelator::GetPssOffsetFrame(CyclicPrefix cp, Duplex dx)
{
	uint32_t offset = 0;
	if( dx == lteFDD) 	{
		offset = (cp == lteCP_Short) ? LTEPssShiftFddS : LTEPssShiftFddL;
	}
	else {
		offset = (cp == lteCP_Short) ? LTEPssShiftTddS : LTEPssShiftTddL;
	}
	return offset;
}
}
