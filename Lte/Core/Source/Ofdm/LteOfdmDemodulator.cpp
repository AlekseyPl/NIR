/*
 * lte_ofdm_demod.cpp
 *
 *  Created on: 11.02.2015
 *      Author: dblagov
 */


#include "Ofdm/LteOfdmDemodulator.h"
#include "Support/LteSupport.h"
#include <Math/ComplexMath.h>
#include <cmath>
#include <algorithm>

using namespace std;

namespace Lte {

LteOfdmDemod::LteOfdmDemod( CyclicPrefix cp ) :
	  maxValue( 16384.0 ), cyclicPrefix( cp ), symbolCount( 0 ),
	  fft( LTEFFTLen_20_MHz )
{
	infftData.resize(LTEFFTLen_20_MHz);
	outfftData.resize(LTEFFTLen_20_MHz);

	SetCyclicPrefix(cp);
	buffer.reserve(sfSymbolCount * LTEFFTLen_20_MHz);
}

LteOfdmDemod::~LteOfdmDemod( )
{

}

void LteOfdmDemod::SetCyclicPrefix(CyclicPrefix cp)
{
	cyclicPrefix = cp;
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
}

void LteOfdmDemod::ProcessSubframe( Complex16* data, uint32_t step )
{
	buffer.clear();
	for( uint32_t slot = 0; slot < LTESlotsInSubframe; ++slot ) {
		for( uint32_t sym = 0; sym < symbolCount; ++sym ) {
			if( cyclicPrefix == lteCP_Short )
				if( sym == 0 ){
					infftData.resize(LTEFFTLen_20_MHz + LTEShortCP0/2);
					data += step * LTEShortCP0;
					PhaseCorrect(data, LTEShortCP0, step );
					data += (LTEFFTLen_20_MHz) * step;}

				else {
					infftData.resize(LTEFFTLen_20_MHz + LTEShortCPX/2);
					data += step * LTEShortCPX;
					PhaseCorrect(data, LTEShortCPX, step);
					data += (LTEFFTLen_20_MHz) * step;}

			else {
				infftData.resize(LTEFFTLen_20_MHz + LTELongCP/2);
				data += step * LTELongCP;
				PhaseCorrect(data, LTELongCP, step);
				data += (LTEFFTLen_20_MHz) * step;
			}

			outfftData.assign(outfftData.size(), ComplexFloat{0,0});
			fft.DoIt( infftData.data(), outfftData.data() );

			std::copy( outfftData.begin()+LTEFFTLen_20_MHz/2, outfftData.end(),std::back_inserter(buffer));
			std::copy( outfftData.begin()+1,outfftData.begin() + LTEFFTLen_20_MHz/2, std::back_inserter(buffer));
			buffer.push_back( ComplexFloat( 0.0, 0.0 ) ); // because we delete " 0 " subcarrier
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

				std::copy( outfftData.begin()+LTEFFTLen_20_MHz/2, outfftData.end(), std::back_inserter(buffer));
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

void LteOfdmDemod::PhaseCorrect(Complex16* data, uint32_t CPType, uint32_t step )
{
	uint32_t SIZE =  LTEFFTLen_20_MHz + CPType/2;
	if (DoPhaseCorr)
	{
		infftData.assign(infftData.size(), ComplexFloat{0,0});
		accum.Imag(0);accum.Real(0);
		auto my_ptr2 = data-CPType/2;
		CPbuf[0].resize(CPType);
		CPbuf[1].resize(CPType);
		Math::ConvertShortToFloat(data-CPType, CPbuf[0].data(), (CPType), step, 1);

		Math::ConvertShortToFloat(data + LTEFFTLen_20_MHz - CPType, CPbuf[1].data(), CPType, step, 1);

		for (int i = 0 ; i < CPType;i++){
			accum += CPbuf[1][i] * conj(CPbuf[0][i]);}
		est = std::atan(accum.Imag()/accum.Real())/LTEFFTLen_20_MHz;

		Math::ConvertShortToFloat(my_ptr2, infftData.data(), SIZE, step, 1);
		for (int i = 0 ; i < SIZE;i++)
		{
			correct_multip.Imag(std::sin(-est*i));
			correct_multip.Real(std::cos(-est*i));
			infftData[i] = infftData[i]*correct_multip;}
	}
	else {
		auto my_ptr2 = data-CPType/2;
		Math::ConvertShortToFloat(my_ptr2, infftData.data(), SIZE, step, 1);
	}
	ActionWithCP(CPType);

}
void LteOfdmDemod::ActionWithCP(uint32_t CPType)
{
	if ((DoSwapCP && DoPhaseCorr)||(DoSwapCP && !DoPhaseCorr))
	{
		std::copy      (infftData.begin(), infftData.begin() +  CPType/2, (infftData.begin()+LTEFFTLen_20_MHz));
		infftData.erase(infftData.begin(), infftData.begin() +  CPType/2);
	}
	else if ((!DoSwapCP&&DoPhaseCorr)||(!DoSwapCP&&!DoPhaseCorr))
	{
		infftData.erase(infftData.begin(), infftData.begin() +  CPType/2);
	}
}

}



