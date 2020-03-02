#include "Common/SssCorrelatorSlow.h"
#include <System/DebugInfo.h>
#include <algorithm>
#include <numeric>

namespace Lte {

SssCorrelator::SssCorrelator():
	  debug(System::DebugInfo::Locate()), sss(false)
{
	sssSignal.reserve(SyncCode::FFTLEN);
	sssCorrRes.resize(2);
	for(auto& s : sssCorrRes)
		s.resize(SyncCode::SSS_COUNT);
}

SssCorrelator::~SssCorrelator()
{

}

SssCorrelator::SearchResult SssCorrelator::Do(const ComplexFloat *data, uint32_t nid2, uint32_t pssPos)
{
	uint32_t resNid1 = 0;
	uint32_t offset = 0;
	uint32_t sfNum  = 0;
	CyclicPrefix cp = lteCP_Short;
	Duplex		 dx = lteFDD;
	float corrMax = 0.0f;

	for( const auto& p : searchParams) {
		ExtractSymbolSss( data, p, pssPos);

		auto ret = ProcessParams(sssSignal.data(), p, nid2);

		if( ret.corrMax > corrMax ) {
			sfNum = ret.sfNum;
			resNid1 = ret.nid1;
			cp		= p.cp;
			dx		= p.duplex;
			corrMax = ret.corrMax;
			offset = p.shiftToFramePos;
		}

	}

	return SearchResult(resNid1, offset, sfNum, cp,dx );
}

SssCorrelator::SssCorrRes SssCorrelator::ProcessParams(const ComplexFloat *data, const SearchParams &params, uint32_t nid2)
{
	float sssMax = 0.0f;
	uint32_t sfNum = 0;
	uint32_t resNid1 = 0;

	for(uint32_t nid1 = 0 ; nid1 < SyncCode::SSS_COUNT ; ++nid1 )
		Correlate( data, nid2, nid1 );

	auto	sssMaxPtr0 = std::max_element( sssCorrRes.at(0).begin(), sssCorrRes.at(0).end());
	auto	sssMaxPtr1 = std::max_element( sssCorrRes.at(1).begin(), sssCorrRes.at(1).end());

	if( *sssMaxPtr0 > *sssMaxPtr1 ) {
		sssMax = *sssMaxPtr0;
		sfNum = 0;
		resNid1 = std::distance(sssCorrRes.at(0).begin(),sssMaxPtr0);
	}
	else {
		sssMax = *sssMaxPtr1;
		sfNum = 5;
		resNid1 = std::distance(sssCorrRes.at(1).begin(),sssMaxPtr1);
	}

	debug.SendText("duplex %s sss Max0 %f, Max1 %f", params.duplex == lteFDD ? "FDD": "TDD", *sssMaxPtr0, *sssMaxPtr1);

	return SssCorrRes(sssMax,sfNum, resNid1);
}

void SssCorrelator::Correlate( const ComplexFloat* data, uint32_t nid2, uint32_t nid1)
{
	float codeAmp0Sqrt = sss.GetAmp( nid1, nid2, 0 );
	float codeAmp1Sqrt = sss.GetAmp( nid1, nid2, 1 );

	auto& code0 = sss.GetCode( nid1, nid2, 0 );
	auto& code1 = sss.GetCode( nid1, nid2, 1 );

	float corr0Abs = CrossCorr(code0.data(), data, SyncCode::FFTLEN);
	float corr1Abs = CrossCorr(code1.data(), data, SyncCode::FFTLEN);

	sssCorrRes.at(0).at(nid1) =  Math::Div( corr0Abs, sssRxAmp * codeAmp0Sqrt );
	sssCorrRes.at(1).at(nid1) =  Math::Div( corr1Abs, sssRxAmp * codeAmp1Sqrt );
}

float SssCorrelator::CrossCorr(const ComplexFloat *code, const ComplexFloat *signal, uint32_t codeLen)
{
	ComplexFloat even{0.0,0.0};
	ComplexFloat odd{0.0,0.0};
	for( uint32_t s = 0; s < codeLen; s+=2) {
		even += conj_mpy(signal[s], code[s]);
		odd += conj_mpy(signal[s+1], code[s+1]);
	}

	ComplexFloat res = even+odd;
	return abs(res);

}


void SssCorrelator::ExtractSymbolSss(const ComplexFloat *data, const SearchParams &params, uint32_t pssPos)
{
	int32_t sssPos = pssPos - params.shiftPssToSss/DecimFactor ;
	if( sssPos < 0 ) sssPos += CorrCount;
	const ComplexFloat* sssData = data + sssPos;

	sssSignal.clear();
	sssSignal.assign(&sssData[0], &sssData[SyncCode::FFTLEN]);

	sssRxAmp = std::accumulate( sssSignal.begin(),sssSignal.end(),0.0f, [](float acc, ComplexFloat val)
						  { return acc + abs2(val);});

	sssRxAmp = Math::Sqrt(sssRxAmp);
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
