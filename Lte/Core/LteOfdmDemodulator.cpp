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
#include <fstream>
#include <iostream>

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
    for( uint32_t slot = 0; slot < LTESlotsInSubframe; ++slot )
    {
        for( uint32_t sym = 0; sym < symbolCount; ++sym )
        {
            if( cyclicPrefix == lteCP_Short )
                if( sym == 0 ){
                    infftData.resize(LTEFFTLen_20_MHz + LTEShortCP0/2);
                    data += step * LTEShortCP0;
                    PhaseCorrect(data, LTEShortCP0, step );
                    {/*std::vector <ComplexFloat> CP0buffer(LTEShortCP0);
                    Math::ConvertShortToFloat(data - LTEShortCP0, CP0buffer.data(), (LTEShortCP0), step, 1);
                    std::ofstream output("/home/stepan/matlab_scripts/1_half_prefix_data.dat", std::ios::binary);
                    output.write(reinterpret_cast<char*>(CP0buffer.data()), CP0buffer.size () * sizeof(CP0buffer[1]));
                    output.close();
                    std::cout<<"1 halfCP"<<std::endl;
                    std::cout<<"=========================="<<std::endl;



                    //записываю кусочек символа соответствующий ЦП


                    std::vector <ComplexFloat> CP01buffer(LTEShortCP0);
                    Math::ConvertShortToFloat(data + LTEFFTLen_20_MHz- LTEShortCP0, CP01buffer.data(), (LTEShortCP0), step, 1);
                    std::ofstream output1("/home/stepan/matlab_scripts/2_half_prefix_data.dat", std::ios::binary);
                    output1.write(reinterpret_cast<char*>(CP01buffer.data()), CP01buffer.size () * sizeof(CP01buffer[1]));
                    output1.close();
                    std::cout<<"2 half CP"<<std::endl;
                    std::cout<<"=========================="<<std::endl;


                    //записываю символ + CP

                    std::vector <ComplexFloat> Symbolbuffer(LTEFFTLen_20_MHz+LTEShortCP0);
                    Math::ConvertShortToFloat(data - LTEShortCP0, Symbolbuffer.data(), (LTEFFTLen_20_MHz+LTEShortCP0), step, 1);
                    std::ofstream output2("/home/stepan/matlab_scripts/symbol_data.dat", std::ios::binary);
                    output2.write(reinterpret_cast<char*>(Symbolbuffer.data()), Symbolbuffer.size () * sizeof(Symbolbuffer[1]));

                    std::cout<<"symbol"<<std::endl;
                    std::cout<<"=========================="<<std::endl;

                    output2.close();*/}
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
            fft.DoIt( infftData.data(), outfftData.data() );//БПФ входных данных

            std::copy( outfftData.begin()+LTEFFTLen_20_MHz/2, outfftData.end(),std::back_inserter(buffer));
            std::copy( outfftData.begin()+1,outfftData.begin() + LTEFFTLen_20_MHz/2, std::back_inserter(buffer));
            buffer.push_back( ComplexFloat( 0.0, 0.0 ) ); // because we delete " 0 " subcarrier
        }

    }

    Normalize();

}
/*void LteOfdmDemod::ProcessSubframe( Complex16* data, uint32_t step )
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
*/
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
        if( curramp > maxamp )maxamp = curramp;
        }
    float scaleCoeff = maxValue/maxamp;

        for(auto& b : buffer )		b *= scaleCoeff;
}

inline bool LteOfdmDemod::IsRefSymb(uint32_t sym)
{
    return (sym == refSymbPosition[ 0 ])|(sym == refSymbPosition[ 1 ]);}


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
    if ((DoSwapCP&&DoPhaseCorr)||(DoSwapCP&&!DoPhaseCorr))
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































