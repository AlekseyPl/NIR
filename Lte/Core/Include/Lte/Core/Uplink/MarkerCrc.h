/*
 * MarkerCrc.h
 *
 *  Created on: May 8, 2018
 *      Author: aplotnikov
 */

#ifndef INCLUDE_LTE_MARKERCRC_H_
#define INCLUDE_LTE_MARKERCRC_H_

#include <stdint.h>

namespace Lte {

class MarkerCrc16 {
public:
	MarkerCrc16( uint16_t poly_ = 0x1021 ) : poly( poly_ ){ };
	virtual ~MarkerCrc16( ){ };

	bool	Check( const uint8_t* buffer, uint32_t bytesCount );

private:
	uint16_t	poly;
};

}

#endif /* INCLUDE_MARKERCRC_H_ */
