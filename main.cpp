#include <stdint.h>

#include <Math/IntComplex.h>
#include <Lte/Core/LteCorrelator.h>
#include <Lte/Core/Common/LteConst.h>
#include <Lte/Core/Common/LteDemConst.h>

#include "Lte/Core/LteStream.h"
#include "Lte/Core/LteOfdmDemodulator.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <memory>


int main(int argc, char *argv[])
{
	std::cout << "Start lte processing\n";
	assert( argc == 2 );

	std::ifstream in( argv[ 1 ], std::ifstream::binary );
	assert( in.is_open( ) );

	in.seekg( 0, in.end );
	int32_t actualLength = in.tellg( ) / sizeof( Complex16 );

	in.seekg( 0, in.beg );

	size_t corrSize = 1 * ( Lte::LTEFrameLength + 2048 + 2048 );

	std::vector<Complex16> corrData(corrSize);
	std::shared_ptr<Lte::LteCorrelator> correlator = std::make_shared<Lte::LteCorrelator>();
	assert( correlator );

    in.read( reinterpret_cast<char*>(corrData.data()), corrSize * sizeof( Complex16 ) );
    in.seekg( 0, in.beg );

//    in.seekg( Lte::LTEFrameLength/2 * sizeof(Complex16)/2, in.beg);

//    in.read( reinterpret_cast<char*>(corrData.data()), corrSize * sizeof( Complex16 ) );
//    in.seekg( 0, in.beg );

	const float corrThreshold = 0.5;
	const uint32_t searchDepth = 5;
    const uint32_t decimFactor = 16;
	const uint32_t numChannels = 1;

	correlator->SetThreshold( corrThreshold );
	correlator->ConfigureSearchParams( searchDepth, decimFactor );

	std::cout << "Start sync " << std::endl;
	bool res = correlator->Process( corrData.data(), numChannels);
	if(res) {

		auto& cells = correlator->GetSyncInfo( );


		int32_t sfLength = Lte::LTESubframeLength;
		std::vector<Complex16> sfBuffer(sfLength);

		std::shared_ptr<Lte::LteStream> stream = std::make_shared<Lte::LteStream>();
		std::shared_ptr<Lte::LteOfdmDemod> demodulator = std::make_shared<Lte::LteOfdmDemod>();

		std::cout << "There are " << cells.size() << " BS on syncStage " << std::endl;

		for( auto& cell : cells ) {
			uint32_t nCellId =cell.nCellId;
			auto cp = cell.cp;
			uint32_t offset = cell.framePos;
			auto dp = cell.duplex;

			std::cout << "Try to decode BS with nCellId " << nCellId << " Offset " << offset << " full length " << actualLength << std::endl;

			int32_t length = actualLength - offset;
			in.seekg( 0, in.beg );
			in.seekg( offset * sizeof( Complex16 ), in.beg );
			stream->InitCellProc(cell);


			bool mibDecoded = false;

			int32_t symCount = 0;
			int32_t sfCount = length / Lte::LTESubframeLength;

			if( cp == Lte::lteCP_Short ) symCount = Lte::LTESymbolsInSlotS;
			else symCount = Lte::LTESymbolsInSlotL;
			symCount *= Lte::LTESlotsInSubframe;

			std::cout << "symCount " << symCount << "sfCount " << sfCount << std::endl;



			for( uint32_t sf = Lte::PBCH_Subframe; sf < sfCount; sf += Lte::LTESubframeInFrame) {


				in.read( reinterpret_cast<char*>(sfBuffer.data()), sfLength * sizeof( Complex16 ) );
				in.seekg( 9 * Lte::LTESubframeLength * sizeof( Complex16 ), in.cur );
                //void SetConfig (bool DoPhaseCorr, bool DoSwapCP) {this->DoPhaseCorr = DoPhaseCorr; this->DoSwapCP = DoSwapCP;};

                demodulator->SetConfig(true,true);

				demodulator->ProcessSubframe(sfBuffer.data(), numChannels);
				auto& ofdmSymbols = demodulator->GetSymbols();
				auto proc = stream->ProcessSubframe(ofdmSymbols.data());
				if(proc == Lte::LteStream::MIBdecoded) {
					std::cout << "MIB decoded" <<std::endl;
					break;
				}
			}
		}

	}
}
