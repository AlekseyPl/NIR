/*
 * MarkerDecoder.h
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_MARKERDECODER_H_
#define LTE_MARKERDECODER_H_

#include <stdint.h>
#include <Common/Buffer.h>
#include <IntMath/Complex.h>

namespace Chip {
	class Vcp;
}

namespace Lte {

class MarkerDecoder {
public:
	MarkerDecoder( );
	virtual ~MarkerDecoder( );

	void	Process( const int8_t* in, uint32_t* out );

private:
	uint32_t						metricSize;
	Common::Buffer< int8_t >		metric;

	Chip::Vcp*						vcp;
};

}


#endif /* MARKERDECODER_H_ */
