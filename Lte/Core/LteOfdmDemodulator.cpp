/*
 * lte_ofdm_demod.cpp
 *
 *  Created on: 11.02.2015
 *      Author: dblagov
 */


#include "LteOfdmDemodulator.h"
#include "Common/LteSupport.h"
#include <Math/ComplexMath.h>
#include <cmath>
#include <algorithm>

using namespace std;

namespace Lte {

LteOfdmDemod::LteOfdmDemod( CyclicPrefix cp ) :
	  maxValue( 16384.0 ), cyclicPrefix( cp ), symbolCount( 0 ),
	  fft( LTEFFTLen_20_MHz ),
	  debug(System::DebugInfo::Locate())
{
	if( cyclicPrefix == lteCP_Short ){
		symbolCount = LTESymbolsInSlotS;
		sfSymbolCount = LTESymbolsInSlotS * LTESlotsInSubframe;
		refSymbPosition[ 0 ] = NormPilotSymb[ 0 ];
		refSymbPosition[ 1 ] = NormPilotSymb[ 1 ];
	}
	else if( cyclicPrefix == lteCP_Long ) {
		symbolCount = LTESymbolsInSlotL;
		sfSymbolCount = LTESymbolsInSlotL * LTESlotsInSubframe;
		refSymbPosition[ 0 ] = ExtPilotSymb[ 0 ];
		refSymbPosition[ 1 ] = ExtPilotSymb[ 1 ];
	}
	infftData.resize(LTEFFTLen_20_MHz);
	outfftData.resize(LTEFFTLen_20_MHz);

	buffer.reserve(sfSymbolCount * LTEFFTLen_20_MHz);
}

LteOfdmDemod::~LteOfdmDemod( )
{

}


void LteOfdmDemod::ProcessSubframe( Complex16* data, uint32_t step )
{
	buffer.clear();
	for( uint32_t slot = 0; slot < LTESlotsInSubframe; ++slot ) {
		for( uint32_t sym = 0; sym < symbolCount; ++sym ) {
			if( cyclicPrefix == lteCP_Short )
				if( sym == 0 )  data += step * LTEShortCP0;
				else 			data += step * LTEShortCPX;
			else data += step * LTELongCP;

			infftData.assign(outfftData.size(), ComplexFloat{0,0});
			Math::ConvertShortToFloat(data, infftData.data(), LTEFFTLen_20_MHz, step, 1);


			outfftData.assign(outfftData.size(), ComplexFloat{0,0});
			fft.DoIt( infftData.data(), outfftData.data() );

			std::copy( outfftData.begin()+LTEFFTLen_20_MHz/2, outfftData.end(),std::back_inserter(buffer));
			std::copy( outfftData.begin()+1,outfftData.begin() + LTEFFTLen_20_MHz/2, std::back_inserter(buffer));
			buffer.push_back( ComplexFloat( 0.0, 0.0 ) ); // because we delete " 0 " subcarrier

			data += LTEFFTLen_20_MHz * step;
		}
	}
	Normalize();
}

void	LteOfdmDemod::ProcessRefSymb( Complex16* data, uint32_t step )
{
	buffer.clear();
	for( uint32_t slot = 0; slot < LTESlotsInSubframe; ++slot ) {
		for( uint32_t sym = 0; sym < symbolCount; ++sym ) {
			if( cyclicPrefix == lteCP_Short )
				if( sym == 0 )  data += step * LTEShortCP0;
				else 			data += step * LTEShortCPX;
			else data += step * LTELongCP;
			if( IsRefSymb(sym) ) {

				Math::ConvertShortToFloat(data, infftData.data(), LTEFFTLen_20_MHz, step, 1);

				fft.DoIt( infftData.data(), outfftData.data() );

				std::copy( outfftData.begin()+LTEFFTLen_20_MHz/2, outfftData.end(),std::back_inserter(buffer));
				std::copy( outfftData.begin()+1,outfftData.begin() + LTEFFTLen_20_MHz/2, std::back_inserter(buffer));
				buffer.push_back( ComplexFloat( 0.0, 0.0 ) ); // because we delete " 0 " subcarrier
			}
			data += LTEFFTLen_20_MHz * step;
		}
	}
	Normalize();
}

void LteOfdmDemod::Normalize()
{
	float maxamp = 0.0;
	for(const auto& b : buffer ) {
		float curramp = abs(b);
		if( curramp > maxamp ) maxamp = curramp;
	}
	float scaleCoeff = maxValue/maxamp;

	for(auto& b : buffer )		b *= scaleCoeff;
}

inline bool LteOfdmDemod::IsRefSymb(uint32_t sym)
{
	return (sym == refSymbPosition[ 0 ])|(sym == refSymbPosition[ 1 ]);
}

}



