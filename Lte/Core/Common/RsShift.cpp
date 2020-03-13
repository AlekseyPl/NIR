/*
 * rs_shift.cpp
 *
 *  Created on: 04.07.2014
 *      Author: dblagov
 */

#include "Common/RsShift.h"
#include "Common/CellInfo.h"
#include <algorithm>

namespace Lte {

uint32_t	GetRSShift( CellInfo& cellInfo, TxAntPorts antPorts, uint32_t slot, uint32_t symbol )
{
	uint32_t rs_shift = InvRSShift;
	uint32_t ant_ports = AntPortsCnt( antPorts );

	for ( uint32_t ap = 0; ap < ant_ports; ++ap ) {
		if ( cellInfo.RefSymbPresent( ap, symbol ) )
			rs_shift = std::min( rs_shift, cellInfo.RefSymbShift( ap, slot, symbol ) );
	}
	return rs_shift;
}

RsIndex::RsIndex(): rsCount(N_dl_rb_max*Nsc_rb), modCID(10)
{
	for(auto& idx: index)
		idx.reserve(rsCount);
}


void RsIndex::Generate(uint32_t nCellId, uint32_t nDlRb)
{
	uint32_t modCIDnew = nCellId%PilotStep;

	if(modCIDnew != modCID) {
		for(auto& idx: index) idx.clear();

		uint32_t  rs1 = modCIDnew;
		uint32_t  rs2 = ( modCIDnew + PilotShift ) % PilotStep ;
		uint32_t  rs[ 2 ] = { rs1, rs2 };
		uint32_t offset = LTEFFTLen_20_MHz / 2 - nDlRb / 2 * Nsc_rb; // shift to central sc

		uint32_t currRsCount = nDlRb * PilotSymbCnt;

		for (uint32_t symb = 0; symb < PilotSymbCnt; ++symb) {
			for( uint32_t sc = 0, k = 0; k < currRsCount; sc += PilotStep, ++k )
				index[symb].push_back( offset + sc + rs[ symb ]);

		}
		modCID = modCIDnew;
	}
}

}

