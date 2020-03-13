/*
 * SoftConvDecoder.cpp
 *
 *  Created on: Sep 10, 2018
 *      Author: aplotnikov
 */

#include "SoftConvDecoder.h"
#include "Common/LteSupport.h"


namespace Lte {

SoftConvDecoder::SoftConvDecoder(  ):
	poly1( 0x6D ),  // 110 1101
	poly2( 0x4F ),  // 100 1111
	poly3( 0x57 )   // 101 0111
{
    int32_t weightsLen = ( 1 << RegLen ) << 1;
	weights.resize(weightsLen);

    EncoderTrellis( );
    DecoderTrellis( );
}

SoftConvDecoder::~SoftConvDecoder( )
{

}

void SoftConvDecoder::EncoderTrellis( )
{
    for ( uint32_t i = 0; i < TrellisNodesCount; ++i ) {
        tEncTrellisNode& n = gEncTrellisNode[ i ];
        uint8_t state = static_cast<uint8_t>( i << 1 );

        n.Out0_G0 = Polynome( state, poly1, Constraint ) ? Bit1 : Bit0;
        n.Out0_G1 = Polynome( state, poly2, Constraint ) ? Bit1 : Bit0;
        n.Out0_G2 = Polynome( state, poly3, Constraint ) ? Bit1 : Bit0;

        n.Out1_G0 = ( n.Out0_G0 == Bit0 ) ? Bit1 : Bit0;
        n.Out1_G1 = ( n.Out0_G1 == Bit0 ) ? Bit1 : Bit0;
        n.Out1_G2 = ( n.Out0_G2 == Bit0 ) ? Bit1 : Bit0;
        n.State0 = state & ( TrellisNodesCount - 1 );
        n.State1 = n.State0 | 1;
    }
}

void SoftConvDecoder::DecoderTrellis( )
{
    for ( uint32_t i = 0; i < TrellisNodesCount; ++i ) {
        tDecTrellisNode& n = gDecTrellisNode[ i ];
        uint8_t state0 = i;
        uint8_t state1 = ( state0 | ( 1 << RegLen ) );

        n.In0_G0 = Polynome( state0, poly1, Constraint ) ? SoftDec1 : SoftDec0;
        n.In0_G1 = Polynome( state0, poly2, Constraint ) ? SoftDec1 : SoftDec0;
        n.In0_G2 = Polynome( state0, poly3, Constraint ) ? SoftDec1 : SoftDec0;
        n.In1_G0 = Polynome( state1, poly1, Constraint ) ? SoftDec1 : SoftDec0;
        n.In1_G1 = Polynome( state1, poly2, Constraint ) ? SoftDec1 : SoftDec0;
        n.In1_G2 = Polynome( state1, poly3, Constraint ) ? SoftDec1 : SoftDec0;

        n.State0 = ( state0 >> 1 ) & ( TrellisNodesCount - 1 );
        n.State1 = ( state1 >> 1 ) & ( TrellisNodesCount - 1 );
    }
}


void    SoftConvDecoder::Process( const int16_t* in, uint32_t* out, uint32_t frameLen )
{
	trace.resize(frameLen);
	trace.assign(frameLen, 0);
	weights.assign(weights.size(),0);
    uint32_t prev = 0, curr = TrellisNodesCount;

    uint32_t start = ( frameLen > ( 2 * Constraint ) ) ? ( frameLen - ( 2 * Constraint ) ) : 0;
    for ( uint32_t i = start; i < frameLen; ++i ) {
        int32_t k0 = in[ ConvCodeRate * i + 0 ];
        int32_t k1 = in[ ConvCodeRate * i + 1 ];
        int32_t k2 = in[ ConvCodeRate * i + 2 ];

        for ( uint32_t j = 0; j < TrellisNodesCount; ++j ) {
			tDecTrellisNode& n = gDecTrellisNode[ j ];
            int32_t w0 = _abs( n.In0_G0 - k0 ) + _abs( n.In0_G1 - k1 ) + _abs( n.In0_G2 - k2 );
            int32_t w1 = _abs( n.In1_G0 - k0 ) + _abs( n.In1_G1 - k1 ) + _abs( n.In1_G2 - k2 );

            if ( w1 < w0 )  weights[ curr + j ] = weights[ prev + n.State1 ] + w1;
            else            weights[ curr + j ] = weights[ prev + n.State0 ] + w0;
        }
        uint32_t tmp = curr;
        curr = prev;
        prev = tmp;
    }

    for ( uint32_t i = 0; i < frameLen; ++i ) {
        int32_t k0 = in[ ConvCodeRate * i + 0 ];
        int32_t k1 = in[ ConvCodeRate * i + 1 ];
        int32_t k2 = in[ ConvCodeRate * i + 2 ];

        uint64_t& t = trace[ i ];

        for ( uint32_t j = 0; j < TrellisNodesCount; ++j ) {
			tDecTrellisNode& n = gDecTrellisNode[ j ];
            int32_t w0 = _abs( n.In0_G0 - k0 ) + _abs( n.In0_G1 - k1 ) + _abs( n.In0_G2 - k2 );
            int32_t w1 = _abs( n.In1_G0 - k0 ) + _abs( n.In1_G1 - k1 ) + _abs( n.In1_G2 - k2 );
            if ( w1 < w0 ) {
                weights[ curr + j ] = weights[ prev + n.State1 ] + w1;
                t |= Mask1In64 << j;
            }
            else weights[ curr + j ] = weights[ prev + n.State0 ] + w0;
        }
        uint32_t tmp = curr;
        curr = prev;
        prev = tmp;
    }

    int32_t min_w      = weights[ prev ];
    uint32_t min_w_pos = 0;
    for ( uint32_t i = 1, j = prev + 1; i < TrellisNodesCount; ++i, ++j ) {
        if ( weights[ j ] < min_w ) {
            min_w     = weights[ j ];
            min_w_pos = i;
        }
    }

    for ( int i = frameLen - 1; i >= 0; i-- ) {
        if( min_w_pos & 1 & 1 )     SetBit( out, i );
        else                        ResetBit( out, i );


        if ( trace[ i ] & ( Mask1In64 << min_w_pos ) ) min_w_pos |= ( 1 << RegLen );
        min_w_pos >>= 1;
    }
}

}
