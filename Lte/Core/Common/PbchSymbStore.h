/*
 * pbch_symb_store.h
 *
 *  Created on: 25.06.2014
 *      Author: dblagov
 */

#ifndef LTE_PBCH_SYMB_STORE_H_
#define LTE_PBCH_SYMB_STORE_H_

#include "Common/LteTypes.h"

namespace Lte {

struct PbchSymbStore {

	PbchSymbStore( ) : pos( 0 ), count( 0 ), frameCount( 0 ), init( false ) { }

	void	Init( ) {
		pos = 0;
		count = 0;
		frameCount = 0;
		init = true;
	}

	void SymbCountIncr( ) {
		count++;
		if( count == PBCH_SymbolsPerRadioFrame ) {
			count = 0;
			frameCount++;
		}
	}

	static const uint32_t TxAntPortsHypotCount = 2;
	static const uint32_t PBCH_MaxSamplesPerRFCnt = PBCH_SubcarriersPerSymbol * PBCH_SymbolsPerRadioFrame;
	static const uint32_t PBCH_frameCount = 7;

	Complex16		symbols[ PBCH_MaxSamplesPerRFCnt * PBCH_frameCount ];
	uint32_t		pos;
	uint32_t		count;
	uint32_t		frameCount;
	bool			init;
};

}

#endif /* LTE_PBCH_SYMB_STORE_H_ */
