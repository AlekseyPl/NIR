#include <stdint.h>

#include <Math/IntComplex.h>
#include <Lte/Core/LteCorrelator.h>
#include <Lte/Core/Common/LteConst.h>
#include <Lte/Core/Common/LteDemConst.h>


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


	const float corrThreshold = 0.5;
	const uint32_t searchDepth = 5;
	const uint32_t decimFactor = 16;
	const uint32_t numChannels = 1;


	correlator->SetThreshold( corrThreshold );
	correlator->ConfigureSearchParams( searchDepth, decimFactor );

	std::cout << "Start sync " << std::endl;
	bool res = correlator->Process( corrData.data(), numChannels);


}
