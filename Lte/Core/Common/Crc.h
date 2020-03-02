/**
 *  @file   lte_crc.h
 *  @author Dolgov Sergey
 *  @date   12/03/2012
 *  @brief  ������� ���������� CRC �������� 3GPP TS 36.212.
 */
#ifndef LTE_CRC_H
#define LTE_CRC_H

#include "Lte/Core/Common/LteTypes.h"
#include <string.h>

namespace Lte
{
enum CrcType
{
  lteCRC8,
  lteCRC16,
  lteCRC24A,
  lteCRC24B
};

uint8_t crc8( const uint8_t *bitVector, uint32_t size );

uint16_t crc16( const uint8_t *bitVector, uint32_t size );

uint32_t crc24a( const uint8_t *bitVector, uint32_t size );

uint32_t crc24b( const uint8_t *bitVector, uint32_t size );

bool CheckCrc( const uint8_t *bits, CrcType crc, uint32_t rv, uint32_t size);

uint16_t BchCheckCrc( const uint8_t* bits );

} // namespace LTE

#endif
