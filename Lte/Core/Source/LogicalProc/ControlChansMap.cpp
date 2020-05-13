/*
 * control_chans_map.cpp
 *
 *  Created on: 22.08.2014
 *      Author: dblagov
 */

#include "LogicalProc/ControlChansMap.h"
#include "Support/LteTypes.h"
#include <math.h>
#include <algorithm>
#include <System/DebugInfo.h>

namespace Lte {

ControlChansMap::ControlChansMap()
{

}

ControlChansMap::~ControlChansMap( )
{

}

void ControlChansMap::InsertPcfich(PcfichIndex &pcfichIndex)
{
	pcfichQuad.resize( CFI_QuadrupletsCount );

	Index PCFICH( CFI_QuadrupletsCount );

	for ( uint32_t i = 0; i < CFI_QuadrupletsCount; ++i )
		PCFICH[ i ] = pcfichIndex[ i * QuadrupletSize ] / SC_WithPilots_InREG;


	QuadrupletPtrs p( PCFICH.size( ) );
	for ( uint32_t i = 0; i < p.size(); ++i )  p[ i ] = &pcfichQuad[ i ];

	Index ii( PCFICH );

	for ( uint32_t i = 0; i < ii.size( ); ++i ) {
		for ( uint32_t j = 0; j < ( ii.size( ) - 1 ); ++j ) {
			if ( ii[ j ] > ii[ j+1 ] ) {
				std::swap( ii[ j ], ii[ j+1 ] );
				std::swap( p[ j ], p[ j+1 ] );
			}
		}
	}

	for ( uint32_t i = 0; i < map.size( ); ++i ) {
		QuadrupletPtrs& v = map[ i ][ 0 ];

		for ( uint32_t k = 0; k < ii.size( ); ++k ) {
			uint32_t j = ii[ k ];

			uint32_t copy_cnt = v.size( ) - j - ( CFI_QuadrupletsCount - k );

			std::copy_backward( v.begin( ) + j, v.begin( ) + ( j + copy_cnt ), v.begin( ) + ( j + copy_cnt + 1 ) );
			v[ j ] = p[ k ];
		}
	}
}

void ControlChansMap::InsertPhich( uint32_t subframe )
{
	uint32_t 	Nphich = GetPhichGroup( );

	if( cellInfo.cp == lteCP_Short )	phichQuad.resize( PHICH_GroupREGs * Nphich );
	else								phichQuad.resize( PHICH_GroupREGs * Nphich  / 2 );

	uint32_t sw = ( cellInfo.duration == ltePHICHDuration_Extended ) ? 1 : 0;
	sw |= ( cellInfo.nDlRb <= CFI_N_DL_RB ) ? 2 : 0;

	uint32_t var, symbols;
	switch ( sw ) {
	case 0: // Duration=Normal, Ndl_rb > 10
		var     = 3;
		symbols = 1;
		break;

	case 1: // Duration=Extended, Ndl_rb > 10
		var     = 1;
		symbols = 3;
		break;

	case 2: // Duration=Normal, Ndl_rb <= 10
		var     = 3;
		symbols = 2;
		break;

	case 3: // Duration=Extended, Ndl_rb <= 10
		var     = 2;
		symbols = 3;
		break;
	}

	map.resize(var);
	TxAntPorts ap = cellInfo.PDCCHAntPorts( );

	for ( uint32_t i = 0; i < map.size( ); ++i, ++symbols ) {

		CfiVariant& v = map.at(i);
		v.resize(symbols);
		uint32_t n[ PHICH_MaxUseSymbol ];

		for ( uint32_t j = 0; j < v.size(); ++j ) {

			bool rs = cellInfo.RefSymbPresent( ap, j );
			uint32_t reg = ( rs ? 2 : 3 ) * cellInfo.nDlRb;

			v[j].resize(reg,0);

			if ( j == PCFICH_Symbol )	reg -= CFI_QuadrupletsCount;
			if ( j < PHICH_MaxUseSymbol ) n[ j ] = reg;
		}

		uint32_t reg_div = GetPhichIndexRegDiv( n[ 0 ], n[ 1 ], subframe );

		for ( uint32_t reg = 0; reg < PHICH_GroupREGs; ++reg ) {
			for ( uint32_t j = 0; j < ( phichQuad.size( ) / PHICH_GroupREGs ); ++j ) {

				uint32_t symb      = GetPhichSymbol( reg, j, subframe );

				uint32_t reg_index = GetPhichIndexReg( n[ symb ], reg_div, j, reg );

				v[ symb ][ reg_index ] = &phichQuad[ j * PHICH_GroupREGs + reg ];

			}
		}
	}

}

void ControlChansMap::InsertPdcch( )
{
	pdcchQuadCnt.clear( );
	AllocatePdcch( );

	for ( uint32_t i = 0; i < map.size( ); ++i )  {

		CfiVariant& v = map[ i ];


		PrbMap( quadMap, v.size( ));
		uint32_t regCount = AllocatedRegs( v.size( ));
		pdcchQuadCnt.push_back( regCount );
		size_t regsCntr = 0;

		for ( uint32_t prb = 0; ( prb < cellInfo.nDlRb ) && ( regsCntr < regCount ); ++prb ) {
			for ( uint32_t reg = 0; ( reg < quadMap.size( ) ) && ( regsCntr < regCount ); ++reg ) {

				QuadrupletMap& q = quadMap[ reg ];

				uint32_t l = q.Symbol;

				uint32_t k = q.REG / ( Nsc_rb / q.REGsInSymbol ) + prb * q.REGsInSymbol;

				if ( v[ l ][ k ] == 0 ) v[ l ][ k ] = &pdcchQuad[ regsCntr++ ];
			}
		}
	}
}

uint32_t ControlChansMap::GetPhichGroup( )
{
	uint32_t Nphich = 0;
	double Ng;
	switch ( cellInfo.resource ) {
		case ltePHICHResource_1_6: Ng = 1.0 / 6.0; break;
		case ltePHICHResource_1_2: Ng = 0.5;       break;
		case ltePHICHResource_1  : Ng = 1.0;       break;
		case ltePHICHResource_2  : Ng = 2.0;       break;
	}

	Nphich = static_cast<uint32_t>( ceil( Ng * ( static_cast<double>(cellInfo.nDlRb) / 8.0 ) ) );

	if ( cellInfo.cp == lteCP_Long ) Nphich *= 2;
	if ( cellInfo.duplex == lteTDD ) Nphich *= static_cast<uint32_t>(cellInfo.TDD_Config.Mi);

	return Nphich;
}

uint32_t ControlChansMap::GetPhichSymbol( uint32_t i, uint32_t m, uint32_t subframe )
{
	uint32_t symb;
	switch ( cellInfo.duration ) {
	case ltePHICHDuration_Normal:
		symb = 0;
		break;
	case ltePHICHDuration_Extended:
		symb = static_cast<uint32_t>( ( IsType2SpecSubframe( subframe ) ) ? ( ( m / 2 + i + 1 ) % 2 ) : i );
		break;
	}
	return symb;

}

uint32_t ControlChansMap::GetPhichIndexReg( uint32_t freeRegs, uint32_t n_div, uint32_t m, uint32_t i )
{
	double cell_id = static_cast<double>( cellInfo.nCellId );
	uint32_t add1  = static_cast<uint32_t>( floor( cell_id * static_cast<double>(freeRegs) / n_div ) );
	uint32_t add2  = static_cast<uint32_t>( floor( i * static_cast<double>(freeRegs) / 3.0 ) );

	return ( add1 + m + add2 ) % freeRegs;
}


uint32_t ControlChansMap::GetPhichIndexRegDiv( uint32_t n0, uint32_t n1, uint32_t subframe )
{
	uint32_t r;
	switch ( cellInfo.duration ) {
	case ltePHICHDuration_Normal:
		r = n0;
		break;

	case ltePHICHDuration_Extended:
		r = ( IsType2SpecSubframe( subframe ) ) ? n1 : n0;
		break;
	}
	return r;

}

void ControlChansMap::AllocatePdcch( )
{
	uint32_t pdcch_max_use_symbols = PDCCH_MaxUseSymbols - ( ( cellInfo.nDlRb <= CFI_N_DL_RB ) ? 0 : 1 );

	uint32_t REG = AllocatedRegs( pdcch_max_use_symbols );
	pdcchQuad.resize( REG );
}

uint32_t ControlChansMap::AllocatedRegs( uint32_t symbols )
{
	uint32_t quad = cellInfo.QuadrupletsPerPRB( symbols );

	quad *= cellInfo.nDlRb;

	quad -= pcfichQuad.size( ) + phichQuad.size( );

	return quad;
}

void ControlChansMap::PrbMap( QuadrupletMaps& quadMap, uint32_t symbols )
{
	quadMap.clear();
	TxAntPorts ports = cellInfo.PDCCHAntPorts();
	for ( uint32_t i = 0; i < symbols; ++i ) {
		bool rs       = cellInfo.RefSymbPresent( ports, i );
		uint32_t cnt  = rs ? 2 : 3;
		uint32_t step = rs ? 6 : 4; //-V112
		for ( uint32_t j = 0; j < cnt; ++j )
			quadMap.push_back( QuadrupletMap( i, j * step, cnt ) );
	}
	// PDCCH (3GPP TS 36.211 6.8.5 Mapping to resource elements)
	std::sort( quadMap.begin( ), quadMap.end( )  );

}

}





