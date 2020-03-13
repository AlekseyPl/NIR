/*
 * Deinterleaver.cpp
 *
 *  Created on: May 15, 2017
 *      Author: aplotnikov
 */

#include "Common/Deinterleaver.h"
#include "Common/Quadruplet.h"

namespace Lte {

template uint32_t Deinterleaver::CcSubBlockDeinterleaver( const int8_t *src, uint32_t len, int8_t *dst, uint32_t offset, const SbInfo& info  );
template uint32_t Deinterleaver::CcSubBlockDeinterleaver( const SoftDecision *src, uint32_t len, SoftDecision *dst, uint32_t offset, const SbInfo& info  );

Deinterleaver::Deinterleaver():
	gCcPermPattern(
	{ 1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31,
	  0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30 }),
	gCtcPermPattern(
	{  0, 16, 8, 24, 4, 20, 12, 28, 2, 18, 10, 26, 6, 22, 14, 30,
	  1, 17, 9, 25, 5, 21, 13, 29, 3, 19, 11, 27, 7, 23, 15, 31})
{

}


void 	Deinterleaver::InterleaveFillerBits1( float *dk, uint32_t fillerBits, const SbInfo& info  )
{
	for( uint32_t i = 0; i < ( fillerBits + info.Nd ); ++i) {
		uint32_t c = gCtcPermPattern[ i % C_tc ];  // Interleaved column
		uint32_t r = i / C_tc;                     // Row
		 uint32_t index =  c * info.R_tc + r;
		dk[index] = 1.0;             // Filler bit
	}
}

void 	Deinterleaver::InterleaveFillerBits2( float *dk, uint32_t fillerBits, const SbInfo& info )
{
	uint32_t i = 0, j = info.Kp - 1;
	while ( i < ( fillerBits + info.Nd ) ) 	{
		uint32_t r      = gCtcPermPattern[ j % C_tc ];
		uint32_t c      = j / C_tc;
		uint32_t index  = r * info.R_tc + c;
		dk[index]   = 1;
		++i;
		j = i - 1;
	}
}

void Deinterleaver::SubBlockInterleaverInfo( uint32_t size, SbInfo& info )
{
	info.D    = size;
	info.R_tc = DivideToPlusInf(info.D, C_tc);
	info.Kp   = info.R_tc * C_tc;
	info.Nd   = info.Kp - info.D;
}

template<typename T>
uint32_t	Deinterleaver::CcSubBlockDeinterleaver( const T *src, uint32_t len, T *dst, uint32_t offset, const SbInfo& info  )
{
	uint32_t p = 0;

	for ( uint32_t k_div = 0; ( k_div < C_tc ) && ( p < len ); ++k_div )
		for ( uint32_t k_mod = 0; ( k_mod < info.R_tc ) && ( p < len ); ++k_mod )  {
			uint32_t index = gCcPermPattern[ k_div ] + C_tc * k_mod;
			if ( index >= info.Nd )
				dst[ ConvCodeRate * ( index - info.Nd ) + offset ] = src[ p++ ];
		}
	return p;
}


uint32_t	Deinterleaver::CtcSubBlockDeinterleaver1( const float *src, float *dst, const SbInfo& info   )
{
	uint32_t p = 0;
	uint32_t len = info.Kp;

	for ( uint32_t k_div = 0; ( k_div < C_tc ) && ( p < len ); ++k_div ) {
		for ( uint32_t k_mod = 0; ( k_mod < info.R_tc ) && ( p < len ); ++k_mod ) {
			uint32_t index = gCtcPermPattern[ k_div ] + C_tc * k_mod;
			if ( index >= info.Nd )		dst[ index-info.Nd] = src[p];
			p++;
		}
	}
	return p;
}

uint32_t	Deinterleaver::CtcSubBlockDeinterleaver2( const float *src, float *dst, const SbInfo& info   )
{
	uint32_t p = 0;

	// Fast
	// FOR TCP need minus sign
	for ( uint32_t k_div = 0; k_div < C_tc; ++k_div ) {
		for ( uint32_t k_mod = 0; k_mod < info.R_tc; ++k_mod ) {
			uint32_t index = ( gCtcPermPattern[ k_div ] + C_tc * k_mod + 1 ) % info.Kp;
			if ( index >= info.Nd )		dst[ index-info.Nd] = src[p];
			p++;
		}
	}
	return p;
}

void Deinterleaver::PdcchQuadDeinterleaver( const Quadruplet* src, uint32_t len, uint32_t nCellId, Quadruplet* dst  )
{
  SbInfo	info;
  SubBlockInterleaverInfo( len, info );

  nCellId %= len;
  uint32_t p = 0;

  for ( uint32_t k_div = 0; ( k_div < C_tc ) && ( p < len ); ++k_div )
	  for ( uint32_t k_mod = 0; ( k_mod < info.R_tc ) && ( p < len ); ++k_mod ) {

		  uint32_t index = gCcPermPattern[ k_div ] + C_tc * k_mod;
		  if ( index >= info.Nd )  {
			  uint32_t s_i = ( p >= nCellId ) ? ( p - nCellId ) : ( len - nCellId + p );
			  dst[ index - info.Nd ] = src[ s_i ];
			  p++;
		  }
	  }
}



}
