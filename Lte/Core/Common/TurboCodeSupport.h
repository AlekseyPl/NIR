/*
 * turbocode_support.h
 *
 *  Created on: Jun 1, 2016
 *      Author: aplotnikov
 */

#ifndef LTE_TURBOCODE_SUPPORT_H_
#define LTE_TURBOCODE_SUPPORT_H_

#include "Lte/Core/Common/LteSupport.h"

namespace Lte {
// TS 36.212 5.1.3.2.3 Turbo code internal interleaver.
struct CtcInterleaver
{
	uint16_t i;
	uint16_t Ki;
	uint16_t f1;
	uint16_t f2;
};

static const uint16_t CtcInterleaverParamsCount = 188;

extern const CtcInterleaver CtcInterleaverParams[CtcInterleaverParamsCount];

uint16_t ComputeCtcInterleaverParams( uint16_t k );


}
#endif /* LTE_TURBOCODE_SUPPORT_H_ */
