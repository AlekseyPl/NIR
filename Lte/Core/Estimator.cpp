/*
 * estimator.cpp
 *
 *  Created on: 24.06.2014
 *      Author: dblagov
 */

#include "Lte/Core/Estimator.h"
#include "Lte/Core/Common/LteSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

namespace Lte {

Estimator::Estimator( BandWidth bw, CyclicPrefix cp, uint32_t nid_ ) :
		rsPos( ( cp == lteCP_Long ) ? ExtRefSymb : NormRefSymb ), scale(32767.0)
{
	nid = nid_;
	nDlRb = Bw2Rb( bw );

	std::memset( x1, 0, 2 * rs_cnt *  sizeof( int32_t ) );
	std::memset( x2, 0, 2 * rs_cnt *  sizeof( int32_t ) );

	rs.resize( LTESlotsInSubframe * LTESubframeInFrame * PilotSymbCnt);
	for(auto& r :rs ) 	r.resize(rs_cnt);

	GenerateRs( cp );
}

Estimator::~Estimator( )
{

}

void Estimator::GetRsInfo( Time& time, bool* rs_, uint32_t* rsShift )
{
	for ( uint32_t i = 0; i < MaxAntennaPorts; ++i ) {
		rs_[ i ] = rsPos.IsRefSymbolPresent( i, time.symbol );
		if ( rs_[ i ] ) rsShift[ i ] = GetRsShift( i, time.slot, time.symbol );
	}
}

uint32_t Estimator::GetRsShift( uint32_t antennaPort, uint32_t slot, uint32_t symbol )
{
	uint32_t v;
	switch ( antennaPort ) {
	case 0: v = ( symbol == 0 ) ? 0 : 3; break;
	case 1: v = ( symbol == 0 ) ? 3 : 0; break;
	case 2: v = 3 * ( slot % 2 );        break;
	case 3: v = 3 + 3 * ( slot % 2 );    break;
	}

	return ( v + nid ) % 6;
}

uint32_t Estimator::EstimateH( Time& time, const ComplexFloat* iq, uint32_t len, uint32_t shift, ComplexFloat* h )
{
	uint32_t rs_len = ExtractRs( iq, len, shift, h );

	ComplexFloat* rs = GetRs( time );
	float currRSRP = 0;

	for( uint32_t i = 0; i < rs_len; ++i ) {
		h[i] *= rs[i];
		h[i] /= scale;
		currRSRP += abs2(h[ i ]);
	}

	float maxamp = 0;
	for( uint32_t i = 0; i < rs_len; ++i ) {
		if( abs(h[i]) > maxamp) maxamp = abs(h[i]);
	}

	currRSRP = Math::Div(currRSRP,rs_len);
	rsrp = ( rsrp > currRSRP ) ? currRSRP : ( ( rsrp + currRSRP ) / 2 );
	return rs_len;
}

uint32_t Estimator::ExtractRs( const ComplexFloat* iq, uint32_t len, uint32_t rsShift, ComplexFloat* rs_ )
{
	uint32_t k = ( len / 2 ) - ( nDlRb * Nsc_rb ) / 2 + rsShift;
	const ComplexFloat *src_rs = iq + k;
	k = 0;
	for ( uint32_t i = 0; i < nDlRb; ++i ) {
		for ( uint32_t j = 0; j < PilotSymbCnt; ++j ) {
			rs_[ k++ ] = *src_rs;
			src_rs += PilotStep;
		}
	}
	return k;
}


ComplexFloat* Estimator::GetRs(Time &time)
{
	uint32_t rf_slot = time.subframe * LTESlotsInSubframe + time.slot;
	uint32_t index = static_cast<uint32_t>( rf_slot * 2 + ( time.symbol ? 1 : 0 ) );
	return &rs[ index ][ N_dl_rb_max - nDlRb ];
}

void Estimator::GenerateRs( CyclicPrefix cp )
{
	const float ampl = 23169.0;//32767; // 32767 / sqrt( 2 )
	const uint32_t Nc = 1600;

	bool ext_cp = ( cp == lteCP_Long);
	uint32_t Ncp = ext_cp ? 0 : 1;
	const uint32_t* symb = ext_cp ? ExtPilotSymb : NormPilotSymb;

	GenerateX1( Nc, 2 * rs_cnt, x1 );

	for( uint32_t ns = 0; ns < ( LTESlotsInSubframe * LTESubframeInFrame ); ++ns ) {
	    for ( uint32_t l = 0; l < PilotSymbCnt; ++l ) {
			ComplexFloat* ptr  = rs[ 2 * ns + l ].data();
	        uint32_t c_init = ( 1 << 10 ) * ( 7 * ( ns + 1 ) + symb[ l ] + 1 ) * ( 2 * nid + 1 ) + 2 * nid + Ncp;
	        GenerateX2( Nc, 2 * rs_cnt, c_init, x2 );
			for ( uint32_t m = 0; m < rs_cnt; ++m ) {
				ptr[m].Real(ampl * ( 1 - 2 * ( x1[ 2 * m ]   ^ x2[ 2 * m ] ) ));
				ptr[m].Imag(ampl * ( -( 1 - 2 * ( x1[ 2 * m + 1 ] ^ x2[ 2 * m + 1 ] ) ) ));
			}
	    }
	}
}

void Estimator::GenerateX1( uint32_t start, size_t count, int32_t* x1 )
{
	const uint32_t InitState = 0x40000000;
	const uint32_t MaskExt   = 0x40000000;
	const uint32_t MaskGen   = 1;
	const unsigned R1        = 30;
	const unsigned R2        = 27;

	uint32_t R = InitState;
	for ( uint32_t i = 0; i < start; ++i ) {
		uint32_t B = ( ( R >> R1 ) ^ ( R >> R2 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}

	for ( uint32_t i = 0; i < count; ++i ) {
		x1[ i ] = ( R & MaskExt ) ? 1 : 0;
		uint32_t B = ( ( R >> R1 ) ^ ( R >> R2 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}
}

void Estimator::GenerateX2( uint32_t start, size_t count, uint32_t vInit, int32_t* x2 )
{
	const uint32_t   MaskExt = 0x40000000;
	const uint32_t   MaskGen = 1;
	const unsigned R1      = 30;
	const unsigned R2      = 29;
	const unsigned R3      = 28;
	const unsigned R4      = 27;

	uint32_t R = 0;
	for ( uint32_t i = 0; i <= 30; ++i ) {
		R <<= 1;
		if ( vInit & (1 << i) )
			R |= 1;
	}
	for ( uint32_t i = 0; i < start; ++i ) {
		uint32_t B = ( ( R >> R1 ) ^ ( R >> R2 ) ^ ( R >> R3 ) ^ ( R >> R4 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}

	for ( uint32_t i = 0; i < count; ++i ) {
		x2[ i ] = ( R & MaskExt ) ? 1 : 0;
		uint32_t B = ( ( R >> R1 ) ^ ( R >> R2 ) ^ ( R >> R3 ) ^ ( R >> R4 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}
}

}
