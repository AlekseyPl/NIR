/*
 * lte_correlator.cpp
 *
 *  Created on: 17.02.2015
 *      Author: dblagov
 */

#include "LteCorrelator.h"
#include "Common/SssCorrelatorSlow.h"
#include "Common/PssCorrelator.h"
#include "Common/SyncFilter.h"
#include <System/DebugInfo.h>
#include <Math/Peak.h>
#include <algorithm>
#include <cstring>

namespace Lte {

LteCorrelator::LteCorrelator() :
	  clearPosDelta( 8 ),   threshold( 0.5 ),
	  pssPos( 0 ), framePos( 0 ),
	  resNid2(0), nCellId(0),decimFactor(16),
	  pssCorrelator(new PssCorrelator(CorrCount/decimFactor)),
	  sssCorrelator(new SssCorrelator()),
	  filter(new Filter()),
	  debug(System::DebugInfo::Locate())
{
	filter->SetDecimFactor(decimFactor);
	filteredData.resize((CorrCount + LTEBaseSymbolLen)/decimFactor);
	for(auto & p : pssCorrRes)		p.resize(CorrCount/decimFactor);
}

LteCorrelator::~LteCorrelator()
{

}

void LteCorrelator::ConfigureSearchParams( uint32_t var, uint32_t decimFactor_ )
{
	SssCorrelator::SearchDepth* sd = reinterpret_cast<SssCorrelator::SearchDepth*>(&var);
	sssCorrelator->Configure(*sd);
	if( decimFactor_ != decimFactor ) {
		decimFactor = decimFactor_;
		filter->SetDecimFactor(decimFactor);
		filteredData.resize((CorrCount + LTEBaseSymbolLen)/decimFactor);
		for(auto & p : pssCorrRes)		p.resize(CorrCount/decimFactor);
	}

}

bool LteCorrelator::Process( const Complex16* data, uint32_t step)
{
	filter->Process( data, filteredData.data(), filteredData.size(), step );
	syncInfo.clear( );

	CalcSyncCorr( filteredData.data() );

	for(auto& s : syncInfo) s.framePos = (s.framePos + filter->GetFilterSize()/2)%LTEFrameLength;

	return !syncInfo.empty( );
}

bool LteCorrelator::ProcessThinCorr(const Complex16 *data, uint32_t step, Cells& cells)
{
	filter->Process( data, filteredData.data(), filteredData.size(), step );

	syncInfo.clear( );
	for(const auto& cell: cells)
		CalcSyncCorr( filteredData.data(), cell );

	for(auto& s : syncInfo) s.framePos = (s.framePos + filter->GetFilterSize()/2)%LTEFrameLength;

	return !syncInfo.empty( );
}


void LteCorrelator::CalcSyncCorr( const ComplexFloat* data )
{
	for(auto nid2 : {0,1,2}) {
		pssCorrelator->Configure(nid2);
		pssCorrelator->Correlate(data, pssCorrRes.at(nid2).data());
	}

	for( uint32_t corrCntr = 0; corrCntr < MaxCorrsCount; ++corrCntr ) {
		float pssMax = 0;
		for( uint32_t nid2 = 0 ; nid2 < SyncCode::PSS_COUNT; ++nid2 ) {
			auto res = Math::GetPeak(pssCorrRes.at(nid2).data(), pssCorrRes.at(nid2).size());
			float enom = res.max - res.mean;
			float denom = res.max;
			float thr = Math::Div(enom, denom);
			if( thr > pssMax ) {
				pssMax = thr;
				resNid2 = nid2;
				pssPos = res.pos;
			}
		}
		debug.SendText("PssCorrMax %f, pssPos %d, nid2 %d",pssMax, pssPos, resNid2);
		if( pssMax < threshold ) break; // stop if corrMax < threshold

		auto searchRes = sssCorrelator->Do(data, resNid2, pssPos);

		nCellId = GetResNid(searchRes.nid1, resNid2);
		framePos = pssPos*decimFactor - searchRes.shiftToFrame;
		framePos -= searchRes.subframeNum * LTESlotsInSubframe * LTESlotLength;
		if( framePos < 0 ) framePos += LTEFrameLength;

		debug.SendText("FramePos %d, Nid1 %d and nCellId %d sf num %d", framePos, searchRes.nid1, nCellId, searchRes.subframeNum);
		CellInfo cell;
		cell.framePos  	= framePos;
		cell.nCellId   	= nCellId;
		cell.cp			= searchRes.cp;
		cell.duplex		= searchRes.dx;
		syncInfo.push_back( cell );
		ClearCorr( pssCorrRes.at(resNid2).data(), pssPos, clearPosDelta, CorrCount/decimFactor);
	}
}

void LteCorrelator::CalcSyncCorr( const ComplexFloat* data, const CellInfo& cellInfo )
{
	uint32_t nid1 = cellInfo.nCellId / 3;
	uint32_t nid2 = cellInfo.nCellId % 3;

	pssCorrelator->Configure(nid2);
	pssCorrelator->Correlate(data, pssCorrRes[nid2].data());

	auto res = Math::GetPeak(pssCorrRes.at(nid2).data(), pssCorrRes.at(nid2).size());
	pssPos = res.pos;

	framePos = pssPos*decimFactor - sssCorrelator->GetPssOffsetFrame(cellInfo.cp, cellInfo.duplex);
	if( framePos < 0 ) framePos += LTEFrameLength;

	CellInfo bs;
	bs.framePos = framePos;
	bs.cp = cellInfo.cp;
	bs.nCellId = cellInfo.nCellId;
	bs.duplex = cellInfo.duplex;
	syncInfo.push_back( bs );
}


void LteCorrelator::ClearCorr( float* corr, uint32_t pos, uint32_t deltaPos, uint32_t size )
{
	int32_t startPos = pos - deltaPos;
	if( startPos < 0 ) startPos = 0;

	int32_t stopPos = pos + deltaPos;
	if( stopPos >= size ) stopPos = size - 1;
	memset( &corr[ startPos ], 0, ( stopPos - startPos ) * sizeof( float ) );
}


}




