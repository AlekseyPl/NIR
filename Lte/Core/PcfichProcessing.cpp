#include "PcfichProcessing.h"
#include "Common/PnSequence.h"
#include <cstring>

using namespace std;

namespace Lte {

/************************************************************************/
/*                      Implementation  tPCFICH                       */
/************************************************************************/


Pcfich::Pcfich():
	CfiSoftCodeWords(
	{ +1,-1,-1,+1,-1,-1,+1,-1, -1,+1,-1,-1,+1,-1,-1,+1, -1,-1,+1,-1,-1,+1,-1,-1, +1,-1,-1,+1,-1,-1,+1,-1,
	  -1,+1,-1,-1,+1,-1,-1,+1, -1,-1,+1,-1,-1,+1,-1,-1, +1,-1,-1,+1,-1,-1,+1,-1, -1,+1,-1,-1,+1,-1,-1,+1,
	  -1,-1,+1,-1,-1,+1,-1,-1, +1,-1,-1,+1,-1,-1,+1,-1, -1,+1,-1,-1,+1,-1,-1,+1, -1,-1,+1,-1,-1,+1,-1,-1,
	  +1,+1,+1,+1,+1,+1,+1,+1, +1,+1,+1,+1,+1,+1,+1,+1, +1,+1,+1,+1,+1,+1,+1,+1, +1,+1,+1,+1,+1,+1,+1,+1 }
	)
{
	m_Init = false;
}

Cfi Pcfich::OnReceiveData( const SoftDecision* sd, uint32_t len, uint32_t slot )
{
	// TS 36.211 6.7.4 Mapping to REs
	std::memcpy( m_Bits, sd, len * sizeof( SoftDecision ) );
	// TS 6.7.1 Scrambling
	Scrambler( m_CX1init, m_CX2init[ slot / LTESlotsInSubframe ], m_Bits, CFI_CodeWordsSize );
	// 	Sending a transport layer block
	return CfiSoftDecoder( m_Bits );
}

void Pcfich::Init( uint32_t nCellId )
{
	// Scrambler initialization for each subframe
	for ( uint32_t i = 0; i < LTESubframeInFrame; ++i ) {
		uint32_t c_init = ( i + 1 ) * ( 2 * nCellId + 1 ) * ( 1 << 9 ) + nCellId;
		PnSeqInit( c_init, &m_CX1init, &m_CX2init[ i ] );
	}
	m_Init = true;
}


Cfi Pcfich::CfiSoftDecoder(const SoftDecision *input)
{
	int32_t max_corr = 0;
	int32_t max_pos  = 0;
	for ( int32_t i = 0; i < CfiCodeWordsCount; ++i ) {
		int32_t c = 0;
		for ( int32_t j = 0; j < CfiCodeWordSize; ++j )
			c += input[ j ] * CfiSoftCodeWords[ i ][ j ];

		if (c > max_corr )    {
			max_corr = c;
			max_pos  = i;
		}
	}

	return Index2CFI( max_pos );
}

Cfi Pcfich::Index2CFI(int index)
{
	Cfi cfi;
	switch (index) {
		case 0: cfi = lteCFI1; break; /*! CFI 1 */
		case 1: cfi = lteCFI2; break; /*! CFI 2 */
		case 2: cfi = lteCFI3; break; /*! CFI 3 */
		case 3: cfi = lteCFI4; break; /*! CFI 4 (Reserved) */
	}

	return cfi;

}


uint32_t Pcfich::Cfi2Index(Cfi cfi)
{
	uint32_t cfiIndex;
	switch ( cfi ) {
	case lteCFI1:		cfiIndex = 0;		break; //-V525
	case lteCFI2:		cfiIndex = 1;		break;
	case lteCFI3:		cfiIndex = 2;		break;
	case lteCFI4:		cfiIndex = 0;		break; // HACK: CFI 4 reserved!
	}
	return cfiIndex;
}
}// namespace LTE
