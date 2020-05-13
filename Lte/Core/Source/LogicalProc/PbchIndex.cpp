/*
 * pbch_index.cpp
 *
 *  Created on: 04.07.2014
 *      Author: dblagov
 */

#include "LogicalProc/PbchIndex.h"
#include "Support/LteSupport.h"
#include "Support/LteDemConst.h"
#include "Support/RsShift.h"
#include "Support/CellInfo.h"

namespace Lte {

PbchIndex::PbchIndex( )
{
	indexes.reserve(PBCH_SymbolsPerRadioFrame);
}

PbchIndex::~PbchIndex( )
{

}

void PbchIndex::Generate( CellInfo& cellInfo )
{
	indexes.clear();
	uint32_t pilot_step = RefSymbStep( lteTxAntPorts4 );
	indexes.resize( PBCH_SymbolsPerRadioFrame );
	for ( uint32_t i = 0; i < PBCH_SymbolsPerRadioFrame; ++i ) {
		Index& index = indexes[ i ];
		uint32_t rs_shift = GetRSShift( cellInfo, lteTxAntPorts4, PBCH_Slot, i );
		uint32_t sc = ( N_dl_rb_min * Nsc_rb - PBCH_SubcarriersPerSymbol ) / 2;

		bool rs = ( rs_shift != InvRSShift );
		size_t sc_in_symbol = PBCH_SubcarriersPerSymbol - ( rs ? ( PBCH_SubcarriersPerSymbol / 3 ) : 0 );
		index.reserve(sc_in_symbol);
		index.clear();

		for ( uint32_t j = 0; j < PBCH_SubcarriersPerSymbol; ++j, ++sc ) {
			if ( ( sc % pilot_step ) != rs_shift )	index.push_back(sc);
		}

	}
}

}





