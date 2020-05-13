/*
 * MarkerkDeinterleaver.cpp
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#include "Lte/Core/Uplink/MarkerDeinterleaver.h"

namespace Lte {

MarkerDeinterleaver::MarkerDeinterleaver( ):
	rowCount( 8 ), colCount( 15 )
{
	for( int32_t row = 0; row < rowCount; ++row ) {
		for( int32_t col = 0; col < colCount; ++col ) {
			table[ row * colCount + col ] = row + rowCount * col;
		}
	}
}

MarkerDeinterleaver::~MarkerDeinterleaver( )
{

}

void MarkerDeinterleaver::Process( const int8_t* in, int8_t* out )
{
	for( int32_t i = 0; i < EncodedSymCount; ++i ) out[ table[ i ] ] = in[ i ];
}


void MarkerDeinterleaver::Process16( const int16_t* in, int16_t* out )
{
    for( int32_t i = 0; i < EncodedSymCount; ++i ) out[ table[ i ] ] = in[ i ];
}

}



