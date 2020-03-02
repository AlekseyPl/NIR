/*
 * RateRecovery.h
 *
 *  Created on: Apr 28, 2017
 *      Author: aplotnikov
 */

#ifndef INCLUDE_LTE_COMMON_RATERECOVERY_H_
#define INCLUDE_LTE_COMMON_RATERECOVERY_H_

#include "Lte/Core/Common/Deinterleaver.h"
#include <vector>

namespace System {
	class DebugInfo;
}

namespace Lte {

class RateRecovery {
public:
	RateRecovery( );
	~RateRecovery( );

	template<typename T>
	void	CcRateRecovery( SoftDecision* ek, uint32_t eSize, T* dk, uint32_t dSize );

	void	CtcRateRecovery( SoftDecision* ek, CtcRmParams& params, uint32_t fillerBits, float* dk );

private:
	const float ctcScale = 8192.0;
	const float ccSoftScale = 16384.0;
	const float ccVcpScale = 42.0;

	std::vector<float> 		ek0;
	std::vector<float>		ek1;
	std::vector<float>		ek2;

	std::vector<float>	floatSd;

	uint32_t			TurboBlockLen;

	Deinterleaver		deinterleaver;
	System::DebugInfo&	debug;

	template<typename T>
	void			Normalize( std::vector<float>& in, T* out, uint32_t size, float scale);

	inline void		Insert( float *pDk, uint32_t& p, float* sd, uint32_t& k, uint32_t Kp )
	{
		pDk[ p ] = ( pDk[ p ] ? SoftDecPunct : sd[ k++ ] );
		p = ( p + 1 ) % Kp;
	}

};


}
#endif /* INCLUDE_LTE_COMMON_RATERECOVERY_H_ */
