
#include "Support/PnSequence.h"

namespace Lte {

const unsigned X1_R1   = 30;
const unsigned X1_R2   = 27;

const unsigned X2_R1   = 30;
const unsigned X2_R2   = 29;
const unsigned X2_R3   = 28;
const unsigned X2_R4   = 27;

const uint32_t MaskExt = 0x40000000;
const uint32_t MaskGen = 1;

using namespace std;

uint32_t GenerateX1init( uint32_t start )
{
	const uint32_t InitState = 0x40000000;
	uint32_t R = InitState;
	for ( uint32_t i = 0; i < start; ++i ) {
		uint32_t B = ( ( R >> X1_R1 ) ^ ( R >> X1_R2 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}

	return R;
}

uint32_t GenerateX2init( uint32_t start, uint32_t init )
{
	uint32_t R = 0;
	for ( uint32_t i = 0; i <= 30; ++i )
	{
		R <<= 1;
		if ( init & ( 1 << i ) )  R |= 1;
	}

	for ( uint32_t i = 0; i < start; ++i ) {
		uint32_t B = ( ( R >> X2_R1 ) ^ ( R >> X2_R2 ) ^ ( R >> X2_R3 ) ^ ( R >> X2_R4 ) ) & MaskGen;
		R = ( R << 1 ) | B;
	}

	return R;
}

void PnSeqInit( uint32_t cInit, uint32_t* cx1Init, uint32_t* cx2Init )
{
	static const uint32_t Nc = 1600;
	*cx1Init = GenerateX1init( Nc );
	*cx2Init = GenerateX2init( Nc, cInit );
}

void Scrambler( uint32_t cx1Init, uint32_t cx2Init, SoftDecision *sd, uint32_t len, uint32_t skipLen )
{
	uint32_t R1 = cx1Init;
	uint32_t R2 = cx2Init;
	uint32_t x1, x2, B;
	for( uint32_t i = 0; i < ( skipLen + len ); ++i ) {
		x1 = ( R1 & MaskExt ) ? 1 : 0;
		B = ( ( R1 >> X1_R1 ) ^ ( R1 >> X1_R2 ) ) & MaskGen;
		R1 = ( R1 << 1 ) | B;

		x2 = ( R2 & MaskExt ) ? 1 : 0;
		B = ( ( R2 >> X2_R1 ) ^ ( R2 >> X2_R2 ) ^ ( R2 >> X2_R3 ) ^ ( R2 >> X2_R4 ) ) & MaskGen;
		R2 = ( R2 << 1 ) | B;

		if( i >= skipLen ) {
			if ( x1 ^ x2 ) 	*sd = -*sd;
			sd++;
		}
	}
}

} // namespace LTE
