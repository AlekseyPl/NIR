#include "Common/SssCorrelator.h"
#include <System/DebugInfo.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>

namespace Lte {


SssCorrelator::SssCorrelator():
      startPos0(SyncCode::FFTLEN / 2 - LTESyncCodeHalfLen ),
      startPos1(SyncCode::FFTLEN / 2 + 1 ),
      endPos0(SyncCode::FFTLEN/2  ),
      endPos1(SyncCode::FFTLEN/2 + LTESyncCodeHalfLen ),
      fft64(SssFftCorrLen/*,Common::AllocatorHeapCached::Locate()*/),
      fft128(SyncCode::FFTLEN/*, Common::AllocatorHeapCached::Locate()*/),
      sssCode( {SecondarySyncCode::sss0Even,SecondarySyncCode::sss0Odd, SecondarySyncCode::sss5Odd, SecondarySyncCode::sss5Even}),
      debug(System::DebugInfo::Locate())
{
    for (int i = 0; i < ResultAmount; ++i) {
        corrRes[i].resize(SssFftCorrLen);
        fftCorrRes[i].resize(SssFftCorrLen);
    }
    absCorrRes.resize(SssFftCorrLen);

	searchParams.reserve(VariantsCount);
	sssSpectrumPart.resize(SssFftCorrLen);
	sssSignal.resize(SyncCode::FFTLEN);
	sssSpectrum.resize(SyncCode::FFTLEN);



	for(auto& sp : sssParts)	sp.resize(SssFftCorrLen);
}

SssCorrelator::~SssCorrelator()
{}

SssCorrelator::SearchResult	SssCorrelator::Do(const ComplexFloat* data, uint32_t nid2, uint32_t pssPos)
{
	float maxCorr = 0.0;
	int32_t resNid1 = 0;
	uint32_t offset = 0;
	uint32_t sfNum  = 0;
	CyclicPrefix cp = lteCP_Short;
	Duplex		 dx = lteFDD;
	for( const auto& p : searchParams) {
		ExtractSignalSss( data, p, pssPos);

		auto ret = Correlate(nid2);
		if( ret.corrRes > maxCorr ) {
            maxCorr = ret.corrRes;
			resNid1 = m0m1.GetNid1(M0M1(ret.M0,ret.M1));
			offset  = p.shiftToFramePos;
			sfNum	= ret.subframeNum;
			cp		= p.cp;
			dx		= p.duplex;
		}
	}

	return SearchResult(resNid1, offset, sfNum, cp,dx );
}

SssCorrelator::SssCorrRes	SssCorrelator::Correlate( uint32_t nid2 )
{
    SssCorrRes corr[2];
    corr[0].subframeNum = 0;
    corr[1].subframeNum = 5;

    for( uint32_t count = 0; count < 2; ++count ) {

        searchRes temp1;
        searchRes temp2;
        uint32_t genParamM;

        auto evenPart = sssParts[2*count].data();
        sss.DemodCt(evenPart, nid2);
        fft64.DoIt(evenPart,sssSpectrumPart.data());
        temp1 =  FindSeq(&count, 0);
        if (count == 0) {
            corr[count].M0 = CalcM0( temp1.place);
            genParamM = corr[count].M0;
        }
        else {
            corr[count].M1 = CalcM1( temp1.place  + 1);
            genParamM = corr[count].M1;
        }

        auto oddPart  = sssParts[2*count+1].data();
        sss.DemodCtZt(oddPart,nid2,genParamM);
        fft64.DoIt(oddPart,sssSpectrumPart.data());
        temp2 =  FindSeq(&count, 1);
        if (count == 0) corr[count].M1 = CalcM1( temp2.place );
        else            corr[count].M0 = CalcM0( temp2.place - 1 );

        if (corr[count].M0>corr[count].M1||corr[count].M0 + 8 <corr[count].M1) {temp1.val = 0; temp2.val = 0;}

        corr[count].corrRes = temp1.val+temp2.val;

        auto a = m0m1.GetNid1(M0M1(corr[count].M0, corr[count].M1));

        std::cout<<"";

    }
    return std::max(corr[0],corr[1]);
}
void	SssCorrelator::ExtractSignalSss(const ComplexFloat* data, const SearchParams& params, uint32_t pssPos)
{
    int32_t sssPos = pssPos - params.shiftPssToSss/DecimFactor ;
    if( sssPos < 0 ) sssPos += CorrCount;
    const ComplexFloat* sssData = data + sssPos;
//    SecondarySyncCode mySss(false);

//    auto& myCode = mySss.GetCode(160, 1, 1);
    std::copy(&sssData[0], &sssData[SyncCode::FFTLEN], sssSignal.data());
    fft128.DoIt(sssSignal.data(),sssSpectrum.data());
    fft128.Shift(sssSpectrum.data(),sssSignal.data());
    auto evenPart = sssParts[0].data();
    auto oddPart  = sssParts[1].data();

    for( int32_t i = startPos0; i < endPos0; i+=2)
        *evenPart++ = sssSignal[i];
    for( int32_t i = startPos1+1; i < endPos1; i+=2 )
        *evenPart++ = sssSignal[i];
    for( int32_t i = startPos0+1; i < endPos0; i+=2)
        *oddPart++ = sssSignal[i];
    for( int32_t i = startPos1; i < endPos1+1; i+=2 )
        *oddPart++ = sssSignal[i];
    *evenPart = ComplexFloat(0,0);
    *oddPart = ComplexFloat(0,0);
    sssParts[2].assign(sssParts[0].begin(), sssParts[0].end()); // for SSS0 and SSS1 correlating
    sssParts[3].assign(sssParts[1].begin(), sssParts[1].end());

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

SssCorrelator::searchRes SssCorrelator::FindSeq(uint32_t* count, uint32_t evenOrOdd)
{
    searchRes ResultSpot;
    std::vector <searchRes> resultNum(ResultAmount);
    for (int i = 0; i < ResultAmount; ++i) {
        auto& code = sss.GetSpecCode(sssCode[2* *count + evenOrOdd]);
        for( auto sIt = sssSpectrumPart.begin(), cIt = code[i].begin(),
             fIt = fftCorrRes[i].begin();        sIt != sssSpectrumPart.end(); ++sIt, ++cIt, ++fIt)
            *fIt = conj_mpy(*sIt,*cIt);
        fft64.Undo(fftCorrRes[i].data(),corrRes[i].data());
        for (int p =  SssFftCorrLen/2 ; p < SssFftCorrLen ; ) {
            absCorrRes[p] =  abs(corrRes[i][p]);
            if (resultNum[i].val<absCorrRes[p]) {
                resultNum[i].val = absCorrRes[p];
                resultNum[i].place = (p - i*5)%(SssFftCorrLen/2);
                ++p;}
            else {++p;}
            }
        }
        ResultSpot = resultNum[0];
        for (uint32_t i = 0; i < ResultAmount;) {
            if (resultNum[i].val > ResultSpot.val) {
                ResultSpot = resultNum[i];
                ++i;
            }
            else ++i;
        }
        return ResultSpot;
    }
}
