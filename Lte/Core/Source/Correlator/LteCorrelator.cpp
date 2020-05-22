/*
 * lte_correlator.cpp
 *
 *  Created on: 17.02.2015
 *      Author: dblagov
 */

#include "Correlator/LteCorrelator.h"
#include "Correlator/SssCorrelator.h"
#include "Correlator/SssCorrelatorSlow.h"
#include "Correlator/PssCorrelator.h"
#include "Correlator/SyncFilter.h"
#include <System/DebugInfo.h>
#include <Math/Peak.h>
#include <algorithm>

namespace Lte {

LteCorrelator::LteCorrelator() :
	  threshold(0.51f),
      pssPos( 0 ), framePos( 0 ), maxCorrCntr(1),
	  nCellId(0), decimFactor(LTEDownFactorSync),
	  rdmGap(0), isNeedRdm(false),
	  pssCorrelator(new PssCorrelator(CorrCount/decimFactor)),
	  sssCorrelator(new SssCorrelator()),
	  sssCorrelatorSlow(new SssCorrelatorSlow()),
	  filter(new Filter()),
	  debug(System::DebugInfo::Locate())
{
	filter->SetDecimFactor(decimFactor);

	filteredData.resize((CorrCount + LTESlotLength)/decimFactor);
}

LteCorrelator::~LteCorrelator()
{

}

void LteCorrelator::ConfigureSearchParams( uint32_t var, uint32_t decimFactor_)
{
	SssCorrelator::SearchDepth* sd = reinterpret_cast<SssCorrelator::SearchDepth*>(&var);
	sssCorrelator->Configure(*sd);
	SssCorrelatorSlow::SearchDepth* sdS = reinterpret_cast<SssCorrelatorSlow::SearchDepth*>(&var);

	sssCorrelatorSlow->Configure(*sdS);
	if( decimFactor_ != decimFactor ) {
		decimFactor = decimFactor_;
		filter->SetDecimFactor(decimFactor);
		filteredData.resize((CorrCount + LTESlotLength)/decimFactor);
	}
}

void LteCorrelator::ConfigureRdm(uint32_t rdmGap_)
{
	isNeedRdm = true;
	rdmGap = rdmGap_;
	rdmGapDiv16 = ((rdmGap + 15) / 16)*16;
	rdmCorr.resize(rdmGapDiv16);
	uint32_t rdmVecSize = LTEBaseSymbolLen + (rdmGapDiv16<<1);

	rdmSlice.resize(rdmVecSize);
}

bool LteCorrelator::Process( const Complex16* data, uint32_t step)
{
	filter->Process( data, step );
	filter->GetFilteredData(filteredData.data(), 0, filteredData.size(), LTEDownFactorSync);
	rdmCells.clear();
	syncInfo.clear();

	CalcSyncCorr( filteredData.data() );

	for(auto& s : syncInfo) s.framePos = (s.framePos + filter->GetFilterSize()/2)%LTEFrameLength;

	return !syncInfo.empty( );
}

void LteCorrelator::CalcSyncCorr( const ComplexFloat* data )
{
	pssCorrelator->Correlate(data);

	float pssMax = 0.0f;
    uint32_t resNid2 = 0;
    bool doProcess = true;
    while (doProcess) {

        auto pssRes = FindCorrMax(resNid2, doProcess);
        SetCondition(resNid2);

        pssPos = pssRes.pos;
        pssMax = pssRes.val;
        debug.SendText("Nid2 %d :PssCorrMax %f, pssPos %d, trh %f", resNid2, pssMax, pssPos, threshold);
        if( !doProcess ) break; // stop if no maximum are found
        //auto searchRes = sssCorrelatorSlow->Do(data, resNid2, pssPos);
        auto searchRes = sssCorrelator->Do(data, resNid2, pssPos);
        if(searchRes.nid1 == -1) continue;

        nCellId = GetResNid(searchRes.nid1, resNid2);
        framePos = (pssPos*decimFactor - searchRes.shiftToFrame - searchRes.subframeNum * LTESubframeLength + LTEFrameLength)%LTEFrameLength ;


        pssCorrelator->ClearCorr(resNid2, pssPos, 16);

        debug.SendText("Nid1 %d : FramePos %d, nCellId %d, sfNum %d", searchRes.nid1, framePos, nCellId, searchRes.subframeNum);
        CellInfo cell;
        cell.framePos  	= framePos;
        cell.nCellId   	= nCellId;
        cell.cp			= searchRes.cp;
        cell.duplex		= searchRes.dx;
        syncInfo.push_back( cell );

        //		break;
        if(isNeedRdm) {
            SetRdmCorr(pssPos, resNid2);

            RdmCell rdmCell;
            uint32_t rdmCorrBegin = rdmGapDiv16-rdmGap;
            uint32_t rdmCorrEnd = rdmGapDiv16*2-rdmCorrBegin;
            for(uint32_t i = rdmCorrBegin; i < rdmCorrEnd; ++i )
                rdmCell.rdmCorr.push_back(static_cast<uint32_t>(rdmCorr[i]));
            rdmCell.peakPosition = (rdmOffset*decimFactor - searchRes.shiftToFrame - searchRes.subframeNum * LTESubframeLength + LTEFrameLength)%LTEFrameLength;
            rdmCells.emplace(std::make_pair(nCellId, rdmCell));
        }
    }
}

void LteCorrelator::CalcSyncCorr( const ComplexFloat* data, const CellInfo& cellInfo )
{
    uint32_t nid1 = cellInfo.nCellId / 3;
    uint32_t nid2 = cellInfo.nCellId % 3;

    bool doThis;

    pssCorrelator->Correlate(data);
    auto res = FindCorrMax(nid2, doThis);
    pssPos = res.pos;

    framePos = (pssPos*decimFactor - sssCorrelator->GetPssOffsetFrame(cellInfo.cp, cellInfo.duplex)
				+ LTEFrameLength)%LTEFrameLength;

	CellInfo bs;
	bs.framePos = framePos;
	bs.cp = cellInfo.cp;
	bs.nCellId = cellInfo.nCellId;
	bs.duplex = cellInfo.duplex;
	syncInfo.push_back( bs );
}

void LteCorrelator::SetRdmCorr( uint32_t pssPos, uint32_t nid2 )
{
	uint32_t widePssPos = pssPos*decimFactor;
	uint32_t startGap = (widePssPos - rdmGapDiv16/2 + CorrCount)%CorrCount;
	uint32_t stopGap = (widePssPos + rdmGapDiv16/2 + LTEBaseSymbolLen+ CorrCount)%CorrCount;

	rdmSlice.assign(rdmSlice.size(), ComplexFloat{.0f, .0f});
	filter->GetFilteredData(rdmSlice.data(), startGap, stopGap, LTEDownFactor_20_MHz);
	pssCorrelator->CorrelateThin(rdmSlice.data(), rdmCorr.data(), nid2, rdmGap);

	auto ret = std::max_element(rdmCorr.begin(), rdmCorr.end());
	uint32_t pos = std::distance(rdmCorr.begin(), ret);
	rdmOffset = pssPos - (SyncCode::FFTLEN/2-pos);

}

void LteCorrelator::SetCondition(int setNid2){

    switch (setNid2) {
    case nid0Ch:
        ++nid2Cond.cnt0;
        break;
    case nid1Ch:
        ++nid2Cond.cnt1;
        break;
    case nid2Ch:
        ++nid2Cond.cnt2;
        break;
    }

}
bool LteCorrelator::CheckCondition(int checkNid2){

   // switchNids;

    switch (checkNid2) {
    case nid0Ch:
        if(nid2Cond.cnt0==maxCorrCntr) return false;
        else return true;
        break;
    case nid1Ch:
        if(nid2Cond.cnt1==maxCorrCntr) return false;
        else return true;
        break;
    case nid2Ch:
        if(nid2Cond.cnt2==maxCorrCntr) return false;
        else return true;
        break;
    }

}
const PssCorrelator::PssRes LteCorrelator::FindCorrMax(uint32_t& resNid2, bool& findRes){


    PssCorrelator::PssRes res;

    const auto &absCorr = pssCorrelator->GetCorrRes();
    //float pssMax = threshold;
    int tempCounter = 0;
    for( uint32_t nid2 = 0 ; nid2 < SyncCode::PSS_COUNT; ++nid2 ) {

        if(!CheckCondition(nid2)) continue;

        auto maxptr = max_element( absCorr.at(nid2).begin(), absCorr.at(nid2).end() );

        if( *maxptr >= threshold) {
            res.val = *maxptr;
            resNid2 = nid2;
            res.pos = std::distance(absCorr.at(nid2).begin(), maxptr);
            tempCounter++;//если ни одного максимума = 0
        }

    }

    if(tempCounter==0) {
        findRes = false;

    }

    return res;
}

}




