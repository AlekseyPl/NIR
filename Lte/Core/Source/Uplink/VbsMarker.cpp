/*
 * VbsMarker.cpp
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#include "Lte/Core/Uplink/VbsMarker.h"
#include "Lte/Core/Uplink/MarkerConst.h"
#include "Lte/Core/Common/LteSupport.h"
#include "IntMath/Util.h"
#include <System/DebugInfo.h>
#include <string.h>
#include <Chip/Delay.h>

namespace Lte {

VbsMarker::VbsMarker( ):
	progress( 0 ), debug( System::DebugInfo::Locate( ) )
{
	demodData.resize( EncodedSymCount );
	deinterlData.resize( EncodedSymCount + EncodedTailLength );
	data.resize( IqCount );
	preamble.resize( PreambleLength );
	afc.resize( PreambleLength );
    Reset( );
	GeneratePreamble( );
}

VbsMarker::~VbsMarker( )
{
}

void VbsMarker::Reset( )
{
    memset( decodedData, 0, 2 * sizeof( uint32_t ) );

	afc.assign(PreambleLength,Complex16{0,0});
	data.assign(IqCount,Complex16{0,0});

	demodData.assign(demodData.size(),0);
	deinterlData.assign(deinterlData.size(),0);

    progress = 0;
}

bool VbsMarker::AddSymbol( const Complex16* sym )
{
	std::memcpy( &data[progress], sym, Nsc_rb * sizeof(Complex16) );
	progress += Nsc_rb;

	return ( progress == IqCount );
}

bool VbsMarker::Process( )
{
//	Complex16 freqError = CalcFreqError( data.Ptr( ) );
    Complex16 freqError( 0x7FFF, 0 );

	CompensateFreqError( data.data(), freqError );
	CalcAFC( data.data(), afc.data() );
	Equalize( data.data(), afc.data() );

#if 1 // while VCP not working, using short data type
	DemodulateSoft( data.data(), demodData.data() );
	deinterleaver.Process( demodData.data(), deinterlData.data() );
	decoder.Process( deinterlData.data(), decodedData );
#else

	demodData.Copy( (int16_t*)data.data( 2 * Nsc_rb ), EncodedSymCount );
	deinterleaver.Process16( demodData.data(), deinterlData.Ptr() );
	convDec.Process( deinterlData.data(), decodedData, PayloadCount );
#endif

	Descramble( decodedData );
	bool crcres = crc.Check( reinterpret_cast<uint8_t*>(decodedData), PayloadCount / 8 );
	if( crcres )      ParseInfo( decodedData, info );

	return crcres;
}

Complex16 VbsMarker::CalcFreqError( Complex16* marker )
{
	Complex32 freqStep( 0, 0 );

	for( int32_t i = 0; i < PreambleLength; ++i ) {
		Complex16 tmp0 = conj( marker[ i ] );
		Complex32 tmp  = ComplexMpy( marker[ i + PreambleLength ], tmp0 );

		freqStep.re += tmp.re / PreambleLength;
		freqStep.im += tmp.im / PreambleLength;
	}

	uint32_t amp = abs( freqStep );
	int32_t real = IntMath::DivFract( freqStep.re, amp ) >> 16;
	int32_t imag = IntMath::DivFract( freqStep.im, amp ) >> 16;

	return Complex16( (int16_t)real, (int16_t)imag );
}

void VbsMarker::CompensateFreqError( Complex16* marker, Complex16& freqError )
{
	Complex16 phaseOff;
	phaseOff.re = 0x7FFF;
	phaseOff.im = 0x0;

	Complex16* m = marker;
	for( int32_t sym = 0; sym < LTESymbolsInSlotS; ++sym ) {
		Complex16 resPhOff = conj( phaseOff );
		for( int32_t sc = 0; sc < Nsc_rb; ++sc ) {
			Complex32 tmp = ComplexMpy( m[ sc ], resPhOff );
			m[ sc ].re = ( int16_t )( tmp.re >> 15 );
			m[ sc ].im = ( int16_t )( tmp.im >> 15 );
		}
		m += Nsc_rb;

		Complex32 tmp = ComplexMpy( phaseOff, freqError );
		phaseOff.re = ( int16_t )( tmp.re >> 15 );
		phaseOff.im = ( int16_t )( tmp.im >> 15 );
	}
}

void VbsMarker::CalcAFC( Complex16* marker, Complex16* afc )
{
	for( int32_t sc = 0; sc < Nsc_rb; ++sc ) {
		Complex16 a1, a2;

		a1.re 	= marker[ sc ].re >> 1;
		a1.im 	= marker[ sc ].im >> 1;
		a2.re 	= marker[ sc + Nsc_rb ].re >> 1;
		a2.im 	= marker[ sc + Nsc_rb ].im >> 1;

		afc[ sc ] 	  = ComplexAdd( a1, a2 );
		Complex32 res = ComplexMpy( afc[ sc ], preamble[ sc ] );
		res.re >>= 15;
		res.im >>= 15;
		afc[ sc ] = res;
	}
}

void VbsMarker::Equalize( Complex16* marker, Complex16* afc )
{
	uint32_t amp[ Nsc_rb ];
	Complex16 conjAfc[ Nsc_rb ];

	for( int32_t sc = 0; sc < Nsc_rb; ++sc ) {
		amp[ sc ] 	  = abs2( afc[ sc ] );
		conjAfc[ sc ] = conj( afc[ sc ] );
	}

	Complex16* data = marker + 2 * Nsc_rb;
	for( int32_t sym = 2; sym < LTESymbolsInSlotS; ++sym ) {
		for( int32_t sc = 0; sc < Nsc_rb; ++sc ) {
			Complex32 res = ComplexMpy( data[ sc ], conjAfc[ sc ] );
			int32_t real = IntMath::DivFract( ( res.re >> 3 ), amp[ sc ] );
			int32_t imag = IntMath::DivFract( ( res.im >> 3 ), amp[ sc ] );

			data[ sc ].re = ( int16_t )( real >> 16 );
			data[ sc ].im = ( int16_t )( imag >> 16 );
		}
		data += Nsc_rb;
	}
}

void VbsMarker::Descramble( uint32_t* inout )
{
    inout[ 0 ] ^= MarkerDescrambler[ 0 ];
    inout[ 1 ] ^= MarkerDescrambler[ 1 ];
}

void VbsMarker::GeneratePreamble( )
{
    for( int32_t sym = 0; sym < PreambleLength; ++sym ) {
        preamble[ sym ].re = 23166 *  PreambleSym[ sym ];
        preamble[ sym ].im = -23166 * PreambleSym[ sym ];
    }
}

void VbsMarker::ParseInfo( const uint32_t* decoded, MarkerInfo& mInfo )
{
    mInfo.subframeOffset = ( decoded[ 0 ] >> 1 ) & 0x7;
    mInfo.startRB        = ( decoded[ 0 ] >> 4 ) & 0x7F;
    mInfo.countRB        = ( decoded[ 0 ] >> 11 ) & 0x7F;
}


void VbsMarker::DemodulateSoft( Complex16* marker, int8_t* demod )
{
	int16_t* data = ( int16_t* )( marker + 2 * Nsc_rb );
	uint32_t minNorm = 32;

	for( int32_t cntr = 0; cntr < EncodedSymCount; ++cntr ) {
		int32_t val = ( int32_t )data[ cntr ];
		uint32_t normCount = _norm( val );
		if( normCount < minNorm ) minNorm = normCount;
	}
	uint32_t maxNorm = 31 - minNorm;

	const uint32_t resBitCount = 5;
	int32_t shift = maxNorm - resBitCount;

	if( shift >= 0 ) {
		for( int32_t cntr = 0; cntr < EncodedSymCount; ++cntr ) {
			demod[ cntr ] = ( int8_t )( data[ cntr ] >> shift );
		}
	}
	else {
		shift = -shift;
		for( int32_t cntr = 0; cntr < EncodedSymCount; ++cntr ) {
			demod[ cntr ] = ( int8_t )( data[ cntr ] << shift );
		}
	}
}

void VbsMarker::DemodulateHard( Complex16* marker, int8_t* demod )
{
	int16_t* data = ( int16_t* )( marker + 2 * Nsc_rb );

	for( int32_t cntr = 0; cntr < EncodedSymCount; ++cntr )
        demod[ cntr ] = ( data[ cntr ] > 0 ) ? 0x01 : 0xFF;
}


}



