/*
 * equalizer.cpp
 *
 *  Created on: 14.03.2015
 *      Author: dblagov
 */

#include "Equalizer.h"
#include "Common/LteSupport.h"
#include <string.h>
#include <algorithm>
#include <System/DebugInfo.h>
#include <complex>
#include <Math/Complex.h>
#include <Math/ComplexMath.h>
using namespace std;


namespace Lte {

Equalizer::Equalizer( TxAntPorts antPorts_, uint32_t rbCount_, CyclicPrefix cp_ ):
	scale( 16384.0 ), rbCount( rbCount_ ), antCount( 0 ), cp( cp_ ),
	carrierRSSI(0),rssi(0)
{
	fft = Rb2Fft( rbCount );

	symCount = ( cp == lteCP_Short ) ? LTESymbolsInSlotS : LTESymbolsInSlotL;
	symCount *= LTESlotsInSubframe;
	scCount = rbCount * Nsc_rb;

	for( auto& afr: afc ) {
		afr.resize( symCount );
		for( auto& a : afr ) a.resize(scCount);
	}

	for(auto& rs: rsShift)
		rs.resize(symCount);

	antCount = AntPortsCnt( antPorts_ );

	if( cp == lteCP_Short ) {
		sfPilotSym[ 0 ] = NormPilotSymb[ 0 ];
		sfPilotSym[ 1 ] = NormPilotSymb[ 1 ];
		sfPilotSym[ 2 ] = NormPilotSymb[ 0 ] + LTESymbolsInSlotS;
		sfPilotSym[ 3 ] = NormPilotSymb[ 1 ] + LTESymbolsInSlotS;
	}
	else {
		sfPilotSym[ 0 ] = ExtPilotSymb[ 0 ];
		sfPilotSym[ 1 ] = ExtPilotSymb[ 1 ];
		sfPilotSym[ 2 ] = ExtPilotSymb[ 0 ] + LTESymbolsInSlotL;
		sfPilotSym[ 3 ] = ExtPilotSymb[ 1 ] + LTESymbolsInSlotL;
	}
}

Equalizer::~Equalizer( )
{

}

void Equalizer::EstimateSymbol( const ComplexFloat* h, uint32_t hSize, TxAntPort antPort, uint32_t rsShift_, int32_t symbolNum )
{
	uint32_t port = TxAntPort2Num( antPort );
	rsShift[ port ][ symbolNum ] = rsShift_;

	auto& symbolAfc = afc[ port ][ symbolNum ];
	symbolAfc.assign(symbolAfc.size(),ComplexFloat(0,0));

	for(uint32_t i =0, p=rsShift_; i < hSize; ++i, p+=PilotStep)
		symbolAfc[p] = h[i];

	InterpolateTime( symbolAfc, rsShift_ );
}

void Equalizer::EstimeSubframe( )
{
	for( auto& a : afc ) 	InterpolateFreq( a );
}

void Equalizer::Equalize1AntPort( const ComplexFloat* src, ComplexFloat* dst, int32_t sfSym )
{
	ComplexFloat* pilot = afc[0][sfSym].data();
	ComplexFloat res(0,0);

	for( uint32_t i = 0; i < rbCount * Nsc_rb; ++i ) {
		float div = abs( pilot[i] );
		if( div > 0 )
			dst[ i ] = ( div > 0 ) ? conj_mpy(src[i],pilot[i])/div : res;

	}
}

void Equalizer::EqualizePrb2AntPort( const ComplexFloat* srcPrb, uint32_t prbIndex,  bool rs, ComplexFloat* dstPrb, int32_t sfSym )
{
	const ComplexFloat* h1 = &afc[0][ sfSym ][ prbIndex *  Nsc_rb ];
	const ComplexFloat* h2 = &afc[1][ sfSym ][ prbIndex *  Nsc_rb ];
	if ( rs ) {
		uint32_t shift = std::min( rsShift[ 0 ][ sfSym ], rsShift[ 1 ][ sfSym ] );

		for ( uint32_t i = 0; i < Nsc_rb; i += 6 ) {
			switch ( shift ) {
			case 0:
				Alamouti2x1( srcPrb[i + 1], srcPrb[i + 2], h1[i + 1], h2[i + 2], dstPrb[i + 1], dstPrb[i + 2] );
				Alamouti2x1( srcPrb[i + 4], srcPrb[i + 5], h1[i + 4], h2[i + 5], dstPrb[i + 4], dstPrb[i + 5] );
			break;

			case 1:
				Alamouti2x1( srcPrb[i + 0], srcPrb[i + 2], h1[i + 0], h2[i + 2], dstPrb[i + 0], dstPrb[i + 2] );
				Alamouti2x1( srcPrb[i + 3], srcPrb[i + 5], h1[i + 3], h2[i + 5], dstPrb[i + 3], dstPrb[i + 5] );
			break;

			case 2:
				Alamouti2x1( srcPrb[i + 0], srcPrb[i + 1], h1[i + 0], h2[i + 1], dstPrb[i + 0], dstPrb[i + 1] );
				Alamouti2x1( srcPrb[i + 3], srcPrb[i + 4], h1[i + 3], h2[i + 4], dstPrb[i + 3], dstPrb[i + 4] );
			break;
			}
		}
	}

	else {
		for ( uint32_t i = 0; i < Nsc_rb / 2; ++i ) {
			Alamouti2x1( srcPrb[2 * i], srcPrb[2 * i + 1], h1[2 * i], h2[2 * i + 1], dstPrb[2 * i], dstPrb[2 * i + 1] );
		}
	}
}
void Equalizer::InterpolateTime( std::vector<ComplexFloat>& h, uint32_t vRSShift )
{
	uint32_t p = vRSShift;
	ComplexFloat d_h;

	if ( p > 0 ) {
		d_h = Hdelta(h[p],h[p]);
		InterpolateLeft( &h[0], p, d_h );
	}
	while ( p < h.size( ) )	{
		if ( ( p + PilotStep ) < h.size( ) )	d_h = Hdelta( h[ p ], h[ p + PilotStep ] );
		else 	d_h = ComplexFloat{0,0};

		uint32_t l = std::min( PilotStep, static_cast<uint32_t>( h.size( ) - p ) );
		InterpolateRight( &h[ p ], l, d_h );
		p += PilotStep;
	}
}

void Equalizer::InterpolateFreq( Matrix& h )
{
	int32_t scCount = h[0].size();
	for( uint32_t cntr = 0; cntr < ( SF_PILOT_SYM_COUNT - 1 ); ++cntr ) {
		int32_t sym0 = sfPilotSym[ cntr ];
		int32_t sym1 = sfPilotSym[ cntr + 1 ];

		for( uint32_t sc = 0; sc < scCount; ++sc ) {
			ComplexFloat delta = Hdelta( h[ sym0 ][ sc ], h[ sym1 ][ sc ], ( sym1 - sym0 ) );
			for( int32_t sym = sym0; sym < ( sym1-1 ); ++sym ) {
				h[ sym + 1 ][ sc ] = h[ sym ][ sc ] + delta;
			}
		}
	}

	int32_t sym1 = sfPilotSym[ SF_PILOT_SYM_COUNT - 1 ];

	for( int32_t sc = 0; sc < scCount; ++sc ) {
		ComplexFloat delta{0,0};
		for( int32_t sym = sym1; sym < ( h.size( ) - 1 ); ++sym ) {
			h[ sym + 1 ][ sc ] = h[ sym ][ sc ]+delta;
		}
	}
}

void Equalizer::InterpolateLeft( ComplexFloat *v, uint32_t len, ComplexFloat delta )
{
	ComplexFloat r = v[ len ];
	for( int32_t i = ( len - 1 ); i >= 0; --i ) v[ i ] = r + delta;
}

void Equalizer::InterpolateRight( ComplexFloat *v, uint32_t len, ComplexFloat delta )
{
	ComplexFloat l = v[ 0 ];
	for ( uint32_t i = 1; i < len; ++i ) 	v[ i ] = l + delta;
}

ComplexFloat	Equalizer::Hdelta( ComplexFloat s1, ComplexFloat s2, float step)
{
	return ( s2 - s1 ) / step;
}

void Equalizer::Alamouti2x1( const ComplexFloat& x1, const ComplexFloat& x2, const ComplexFloat& h1,
							 const ComplexFloat& h2, ComplexFloat& r1, ComplexFloat& r2 )
{
	float div = Math::Div( abs( h1 ) + abs( h2 ) , 2.0);

	if( div > 0 ) {
		// alamouti
		r1 = conj_mpy(x1,h1) + conj_mpy(h2,x2);
		r2 = conj_mpy(x2,h1) - conj_mpy(h2,x1);
		r1 /= div;
		r2 /= div;
	}
	else r1 = r2 = ComplexFloat{0,0};

}

void Equalizer::Normalize( ComplexFloat* src, Complex16* dst, uint32_t count )
{
	float maxamp = 0.0;
	for( uint32_t i = 0; i < count; ++i ) {
		float curramp = abs( src[ i ] );
		if( curramp > maxamp )
			maxamp = curramp;
	}

	if( maxamp > 0 ) {
		for( uint32_t i = 0; i < count; ++i) {
			src[i] /= maxamp;
			src[i] *= scale;
		}

		Math::ConvertFloatToShort(src,dst,count);
	}
}

void Equalizer::CalcRSSI( const ComplexFloat* symbol )
{
	uint32_t currRSSI = 0;


	const ComplexFloat* currSymb = symbol + fft/2 - (rbCount*Nsc_rb)/2;

	auto sumAbs = [ &currRSSI ] (ComplexFloat v) { currRSSI += abs2(v);};

	for_each( &currSymb[ 0 ], &currSymb[ rbCount * Nsc_rb ], sumAbs );

	currRSSI /= rbCount;
	carrierRSSI = ( carrierRSSI == 0 ) ? currRSSI : ( ( carrierRSSI + currRSSI ) / 2 );

	for_each( &symbol[ 0 ], &symbol[ fft ], sumAbs);

	currRSSI /= fft;
	rssi = ( rssi == 0 ) ? currRSSI : ( ( rssi + currRSSI ) / 2 );
}

}




