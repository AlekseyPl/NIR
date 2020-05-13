/*
 * MarkerDeinterleaver.h
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_MARKERDEINTERLEAVER_H_
#define LTE_MARKERDEINTERLEAVER_H_

#include "Lte/Core/Uplink/MarkerConst.h"

namespace Lte {

class MarkerDeinterleaver {
public:
	MarkerDeinterleaver( );
	virtual ~MarkerDeinterleaver( );

	void	Process( const int8_t* in, int8_t* out );
	void    Process16( const int16_t* in, int16_t* out );
private:
	const 	uint32_t 	rowCount;
	const	uint32_t 	colCount;

	uint8_t				table[ EncodedSymCount ];

};

}

#endif /* MARKERDEINTERLEAVER_H_ */
