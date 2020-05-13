/*
 * MarkerConst.h
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_MARKERCONST_H_
#define LTE_MARKERCONST_H_

#include "Lte/Core/Common/LteConst.h"
#include "Lte/Core/Common/LteDemConst.h"

namespace Lte {

const uint32_t PreambleLength 	    = 12;
const uint32_t IqCount 		  	    = LTESymbolsInSlotS * Nsc_rb;
const uint32_t DataSymbCount  	    = 5;
const uint32_t DataIqCount	 	    = DataSymbCount * Nsc_rb;

const uint32_t EncodedSymCount 	    = 2 * DataIqCount;
const uint32_t CodeTailLength	    = Constraint - 1;
const uint32_t EncodedTailLength    = CodeTailLength * ConvCodeRate;

const uint32_t PayloadCount	        = EncodedSymCount / ConvCodeRate;

const uint32_t MarkerSlot           = 1;
const uint32_t MarkerSubframe       = 0;

const uint32_t MarkerDescrambler[ 2 ] = { 0xD86B50CB, 0x0000001D };

const int8_t   PreambleSym[ PreambleLength ] = { -1, 1, -1, 1, -1, 1, -1, 1, 1, -1, 1, 1 };
}


#endif /* LTE_MARKERCONST_H_ */
