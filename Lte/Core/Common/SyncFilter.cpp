/*
 * filter.cpp
 *
 *  Created on: 25.02.2015
 *      Author: dblagov
 */
#include "Common/SyncFilter.h"
#include <Math/ComplexMath.h>
#include <System/DebugInfo.h>

namespace Lte {

Filter::Filter( ) :
	decimFactor(16), debug(System::DebugInfo::Locate())
{

}

void Filter::SetDecimFactor(uint32_t decimFactor_)
{
	decimFactor = decimFactor_;
	iq.resize(LTEFrameLength/2 + LTEFFTLen_20_MHz + FilterAfr::mFilterSize);
	iq.shrink_to_fit();
}

void Filter::Process( const Complex16* in, ComplexFloat* out, int32_t size, int32_t step )
{
	Complex16* inPtr = const_cast<Complex16*>(in);
	uint32_t iqSize = iq.size();
	Math::ConvertShortToFloat( inPtr, iq.data(), iqSize, step);

	for( uint32_t pos = 0, cntr = 0; cntr < size; pos += decimFactor, ++cntr) {

		ComplexFloat iqf{ 0.0, 0.0 };
		ComplexFloat even{0.0,0.0};
		ComplexFloat odd{0.0,0.0};
		for( uint32_t sym = 0; sym < FilterAfr::mFilterSize; sym+=2 ) {
			even += (iq[ pos + sym ] * afr.filterCoef[ sym ]);
			odd += (iq[ pos + sym + 1] * afr.filterCoef[ sym + 1 ]);
		}

		iqf = even+odd;
		out[cntr] = iqf;
	}
}

uint32_t Filter::GetFilterSize() const
{
	return FilterAfr::mFilterSize;
}

}

