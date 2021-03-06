/*
 * lte_ofdm_demod.h
 *
 *  Created on: 11.02.2015
 *      Author: dblagov
 */

#ifndef LTE_OFDM_DEMOD_H_
#define LTE_OFDM_DEMOD_H_

#include <stdint.h>
#include "Common/LteTypes.h"
#include "Common/LteDemConst.h"
#include <System/DebugInfo.h>
#include "Math/IntComplex.h"
#include "Math/FftSP.h"

namespace Lte {

class LteOfdmDemod {
public:
	LteOfdmDemod( CyclicPrefix cp = lteCP_Short );
	virtual ~LteOfdmDemod( );
	CyclicPrefix GetCyclicPrefix( )
	{
		return cyclicPrefix;
	}
	std::vector< ComplexFloat >&	GetSymbols( )
	{
		return buffer;
	}

	void ProcessSubframe( Complex16* data, uint32_t step );
	void ProcessRefSymb( Complex16* data, uint32_t step );
private:

	const float			maxValue;

	CyclicPrefix cyclicPrefix;

	uint32_t symbolCount;
	uint32_t sfSymbolCount;
	uint32_t refSymbPosition[ 2 ];

	std::vector<ComplexFloat>       buffer;

	Math::FftSP			fft;
	std::vector<ComplexFloat>	infftData;
	std::vector<ComplexFloat>	outfftData;
	bool				startSymbol;

	System::DebugInfo&		debug;

	void Normalize();
	bool IsRefSymb(uint32_t sym);
};

}



#endif /* LTE_OFDM_DEMOD_H_ */
