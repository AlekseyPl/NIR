/*
 * lte_ofdm_demod.h
 *
 *  Created on: 11.02.2015
 *      Author: dblagov
 */

#ifndef LTE_OFDM_DEMOD_H_
#define LTE_OFDM_DEMOD_H_

#include <stdint.h>
#include "Lte/Core/Support/LteTypes.h"
#include "Lte/Core/Support/LteDemConst.h"
#include <Math/FftSP.h>

namespace Lte {

class LteOfdmDemod {
public:
	LteOfdmDemod( CyclicPrefix cp = lteCP_Short );
	virtual ~LteOfdmDemod( );
	CyclicPrefix GetCyclicPrefix( )
	{
		return cyclicPrefix;
	}

    void SetCyclicPrefix(CyclicPrefix cp);

	std::vector< ComplexFloat >&	GetSymbols( )
	{
		return buffer;
	}

	void ProcessSubframe( Complex16* data, uint32_t step );
	void ProcessRefSymb( Complex16* data, uint32_t step );

	void PhaseCorrect (Complex16* data, uint32_t CPType, uint32_t step);

	inline void SetConfig (bool DoPhaseCorr, bool DoSwapCP)
	{
		this->DoPhaseCorr = DoPhaseCorr;
		this->DoSwapCP = DoSwapCP;
	}


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

	std::vector <std::vector <ComplexFloat> >   CPbuf{2};

	ComplexFloat accum;
	ComplexFloat correct_multip;
	float32 est;

	bool startSymbol;
	bool DoPhaseCorr = false;
	bool DoSwapCP = false;


	void ActionWithCP (uint32_t CPType);
	void Normalize();
	bool IsRefSymb(uint32_t sym);
};

}



#endif /* LTE_OFDM_DEMOD_H_ */