#include "Common/SssCorrelator.h"
#include <System/DebugInfo.h>
#include <algorithm>
#include <iostream>
#include <fstream>


namespace Lte {

SssCorrelator::SssCorrelator():
	  startPos0(SyncCode::FFTLEN / 2 - LTESyncCodeHalfLen),
	  startPos1(SyncCode::FFTLEN / 2 + 1),
	  endPos0(SyncCode::FFTLEN/2),
	  endPos1(SyncCode::FFTLEN/2 + LTESyncCodeHalfLen),
      fft32(SssFftCorrLen/*,Common::AllocatorHeapCached::Locate()*/),
      fft128(SyncCode::FFTLEN/*, Common::AllocatorHeapCached::Locate()*/),
	  sssCode( {SecondarySyncCode::sss0Even,SecondarySyncCode::sss0Odd, SecondarySyncCode::sss1Odd, SecondarySyncCode::sss1Even}),
	  debug(System::DebugInfo::Locate())
{
	corrRes.resize(SssFftCorrLen);
	fftCorrRes.resize(SssFftCorrLen);
	searchParams.reserve(VariantsCount);

	sssSpectrumPart.resize(SssFftCorrLen);

	sssSignal.resize(SyncCode::FFTLEN);
	sssSpectrum.resize(SyncCode::FFTLEN);

	for(auto& sp : sssParts)	sp.resize(SssFftCorrLen);
}

SssCorrelator::~SssCorrelator()
{

}

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
// 2 sss в одном куске чётные и нечётный. эталон сделан для нид 1 = 0.
	for( uint32_t count = 0; count < 2; ++count ) {
		auto& code0 = sss.GetSpecCode(sssCode[2*count]);
		auto& code1 = sss.GetSpecCode(sssCode[2*count+1]);

//        std::ofstream output("/home/stepan/matlab_scripts/Spec_code_data.dat", std::ios::binary);
//        output.write(reinterpret_cast<char*>(pbchStore.symbols), pbchStore.symb_capacity * sizeof(pbchStore.symbols[0]));
//        output.close();
//        std::cout<<"1 halfCP"<<std::endl;
//        std::cout<<"=========================="<<std::endl;

		auto evenPart = sssParts[2*count].data();
		auto oddPart  = sssParts[2*count+1].data();

		sss.DemodCt(evenPart, nid2);
		fft32.DoIt(evenPart,sssSpectrumPart.data());

//        for( auto sIt = sssSpectrumPart.begin(), cIt = code0.begin(),fIt = fftCorrRes.begin(); sIt != sssSpectrumPart.end(); ++sIt, ++cIt, ++fIt)

//            *fIt = conj_mpy(*sIt,*cIt);

        std::transform(sssSpectrumPart.begin(),sssSpectrumPart.end(),code0.begin(), fftCorrRes.begin(),
                                   [](ComplexFloat v1, ComplexFloat v2) { return conj_mpy(v1, v2);} );


		fft32.Undo(fftCorrRes.data(),corrRes.data());

		auto maxComplexFloat = [](ComplexFloat a, ComplexFloat b) {return abs(a)<abs(b);};
		auto maxEl0 = std::max_element(corrRes.begin(),corrRes.end(),maxComplexFloat);

                std::ofstream output("/home/stepan/matlab_scripts/corrres_code_data.dat", std::ios::binary);
                output.write(reinterpret_cast<char*>(corrRes.data()),corrRes.size() * sizeof(corrRes[0]));
                output.close();
                std::cout<<"1 halfCP"<<std::endl;
                std::cout<<"=========================="<<std::endl;



corr[count].M0 = CalcM0(std::distance(corrRes.begin(),maxEl0));
//        if (corr[count].M0 == 20) {
//            std::cout<<20<<"+++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
//            break;

//        }
		sss.DemodCtZt(oddPart,nid2,corr[count].M0);
		fft32.DoIt(oddPart,sssSpectrumPart.data());

//		for( auto sIt = sssSpectrumPart.begin(), cIt = code1.begin(),fIt = fftCorrRes.begin(); sIt != sssSpectrumPart.end(); ++sIt, ++cIt, ++fIt)

//			*fIt = conj_mpy(*sIt,*cIt);

        std::transform(sssSpectrumPart.begin(),sssSpectrumPart.end(),code1.begin(), fftCorrRes.begin(),
                                   [](ComplexFloat v1, ComplexFloat v2) { return conj_mpy(v1, v2);} );

		fft32.Undo(fftCorrRes.data(),corrRes.data());


        auto maxComplexFloat2 = [](ComplexFloat a, ComplexFloat b) {return abs(a)<abs(b);};

        std::ofstream output2("/home/stepan/matlab_scripts/corrres2_code_data.dat", std::ios::binary);
        output2.write(reinterpret_cast<char*>(corrRes.data()),corrRes.size() * sizeof(corrRes[0]));
        output2.close();
        std::cout<<"1 halfCP"<<std::endl;
        std::cout<<"=========================="<<std::endl;



        auto maxEl1 = std::max_element(corrRes.begin(),corrRes.end(),maxComplexFloat2);
		corr[count].M1 = CalcM1(std::distance(corrRes.begin(),maxEl1));
		corr[count].corrRes = abs(*maxEl0) + abs(*maxEl1);
	}

	return std::max(corr[0],corr[1]);
}

void	SssCorrelator::ExtractSignalSss(const ComplexFloat* data, const SearchParams& params, uint32_t pssPos)
{
	int32_t sssPos = pssPos - params.shiftPssToSss/DecimFactor ;
	if( sssPos < 0 ) sssPos += CorrCount;
	const ComplexFloat* sssData = data + sssPos;

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

	sssParts[2].assign(sssParts[1].begin(), sssParts[1].end()); // for SSS0 and SSS1 correlating
	sssParts[3].assign(sssParts[0].begin(), sssParts[0].end());
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
