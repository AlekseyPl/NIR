/*
 * pcfich_index.cpp
 *
 *  Created on: 09.07.2014
 *      Author: dblagov
 */
#include "LogicalProc/PcfichIndex.h"
#include "Support/LteDemConst.h"
#include "Support/RsShift.h"


namespace Lte {

PcfichIndex::PcfichIndex( )
{

}

PcfichIndex::~PcfichIndex( )
{

}

void PcfichIndex::Generate( CellInfo& cellInfo )
{
	// PCFICH: 4 квадруплета
	index.resize(CFI_Symbols);
	// Позиции квадруплетов
	uint32_t ModVal = Nsc_rb * cellInfo.nDlRb;
	uint32_t k    = ( Nsc_rb / 2 ) * ( cellInfo.nCellId % ( 2 * cellInfo.nDlRb ) );

	// Информация о пилотах
	TxAntPorts ap      = cellInfo.PDCCHAntPorts( );
	uint32_t rs_shift   = GetRSShift( cellInfo, ap, PCFICH_Slot, PCFICH_Symbol );
	uint32_t pilot_step = RefSymbStep(ap);

	// Позиции символов PCFICH
	for ( uint32_t i = 0; i < CFI_QuadrupletsCount; ++i ) {
		uint32_t p = ( k + ( ( i * cellInfo.nDlRb ) / 2 ) * Nsc_rb / 2 ) % ModVal;
		uint32_t j = 0;
		while ( j < QuadrupletSize ) {
			if ( ( p % pilot_step ) != rs_shift ) {
				index[ i * QuadrupletSize + j ] = p;
				j++;
			}
			p++;
		}
	}
}

}



