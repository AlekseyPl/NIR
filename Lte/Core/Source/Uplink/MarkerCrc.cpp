/*
 * MarkerCrc.cpp
 *
 *  Created on: May 8, 2018
 *      Author: aplotnikov
 */

#include "Lte/Core/Uplink/MarkerCrc.h"

namespace Lte {

bool MarkerCrc16::Check( const uint8_t* buffer, uint32_t bytesCount )
{
	uint16_t	check;
	uint16_t    regs = 0;
	uint16_t 	calc = 0;

	uint32_t    dataBytesCount = bytesCount - 2;

	for( int32_t i = 0; i < dataBytesCount * 8; i++ ) {
		uint32_t flag = regs & 0x8000;
		regs <<= 1;
		regs |= ( buffer[ i / 8 ] >> ( i % 8 ) ) & 1;
		if( flag ) regs ^= poly;
	}

	for( int32_t i = 0; i < 16; i++ ) {
		uint32_t flag = regs & 0x8000;
		regs <<= 1;
		if( flag ) 	regs ^= poly;

	}

	uint16_t inMask = 0x8000;
	uint16_t outMask = 0x0001;
	for( int32_t i = 0; i < 16; ++i ) {
		if( inMask & regs ) calc |= outMask;
		inMask  >>= 1;
		outMask <<= 1;
	}
	check = buffer[ dataBytesCount ] | ( buffer[ dataBytesCount + 1 ] << 8 );

	return ( check == calc );
}

}



