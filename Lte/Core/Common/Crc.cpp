/**
 *  @file   lte_crc.c
 *  @author Dolgov Sergey
 *  @date   12/03/2012
 *  @brief  ������� ���������� CRC �������� 3GPP TS 36.212.
 */
#include "Lte/Core/Common/LteSupport.h"
#include "Lte/Core/Common/LteConst.h"
#include "Lte/Core/Common/Crc.h"
#include "System/DebugInfo.h"

namespace Lte
{

uint32_t univ_crc( const uint8_t *bitVector, uint32_t size, uint32_t poly, uint32_t polyDeg )
{
	uint32_t flag = Mask1In32 << polyDeg;
	uint32_t r = 0;

	uint32_t cnt = ( size < polyDeg ) ? size : polyDeg;
	for ( uint32_t i = 0; i < cnt; ++i )
	{
		r <<= 1;
		if ( TestBit( bitVector, i ) )
			r |= 1;
	}
	for ( uint32_t i = cnt; i < size; ++i )
	{
		r <<= 1;
		if ( TestBit( bitVector, i ) )
			r |= 1;
		if ( r & flag )
			r ^= poly;
	}

	for ( uint32_t i = 0; i < polyDeg; ++i )
	{
		r <<= 1;
		if ( r & flag )
		r ^= poly;
	}
	return r;
}

uint8_t crc8( const uint8_t *bitVector, uint32_t size )
{
	const uint32_t poly     = 0x19C; // 1 1001 1011
	const uint32_t poly_deg = 8;

	return static_cast< uint8_t >( univ_crc( bitVector, size, poly, poly_deg ) );
}

uint16_t crc16( const uint8_t *bitVector, uint32_t size )
{
	const uint32_t poly     = 0x11021;  // 1 0001 0000 0010 0001
	const uint32_t poly_deg = 16;

	return static_cast< uint16_t >( univ_crc( bitVector, size, poly, poly_deg ) );
}

uint32_t crc24a(const uint8_t *bitVector, uint32_t size)
{
	const uint32_t poly     = 0x1864CFB; // 1 1000 0110 0100 1100 1111 1011
	const uint32_t poly_deg = 24;

	return static_cast< uint32_t >( univ_crc( bitVector, size, poly, poly_deg ) );
}

uint32_t crc24b( const uint8_t *bitVector, uint32_t size )
{
	const uint32_t poly     = 0x1800063; // 1 1000 0000 0000 0000 0110 0011
	const uint32_t poly_deg = 24;

	return static_cast< uint32_t >( univ_crc( bitVector, size, poly, poly_deg ) );
}

uint32_t crc( CrcType vCRCType, const uint8_t* bits, uint32_t len, uint32_t& crc_size )
{
	uint32_t cv;
	switch ( vCRCType ) {
		case lteCRC8  :
			crc_size = 8;   //-V525
			cv = crc8( bits, len - crc_size );
		break;

		case lteCRC16 :
			crc_size = 16;
			cv = crc16( bits, len - crc_size );
		break;

		case lteCRC24A:
			crc_size = 24;
			cv = crc24a( bits, len - crc_size );
		break;

		case lteCRC24B:
			crc_size = 24;
			cv = crc24b( bits, len - crc_size );
		break;
	}

	return cv;
}


bool CheckCrc( const uint8_t* bits, CrcType crcType, uint32_t rv, uint32_t size )
{
	uint32_t  cv = 0;
	uint32_t crc_size;
	cv = crc( crcType, bits, size, crc_size );
 	TopsyBits( &cv, crc_size );

	return rv == cv;
}

uint16_t BchCheckCrc( const uint8_t* bits )
{
	uint16_t  cv = 0;
	uint32_t	crc_size = 16;
	cv = crc16( bits, BCH_MIB_Size );
	TopsyBits( &cv, crc_size );
	return cv;
}

} // namespace LTE
