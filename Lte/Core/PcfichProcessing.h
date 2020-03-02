
#ifndef LTE_PCFICH_H
#define LTE_PCFICH_H

#include "Lte/Core/Common/LteDemConst.h"
#include <array>

namespace Lte {
/**
 *  @class Pcfich
 *  @brief Physical Control Format Indication Channel.
 */
class Pcfich {
public:
	Pcfich( );
	void Init( uint32_t nCellId );
	Cfi OnReceiveData( const SoftDecision* sd, uint32_t len, uint32_t slot );
	uint32_t	Cfi2Index( Cfi cfi);

private:

	static const int32_t CfiCodeWordsCount      = 4; //-V112
	static const int32_t CfiCodeWordSize        = 32; //-V112

    bool         	m_Init;
    uint32_t     	m_CX1init;
    uint32_t	    m_CX2init[ LTESubframeInFrame ];
    SoftDecision 	m_Bits[ CFI_CodeWordsSize ];
	std::array< std::array<SoftDecision, CfiCodeWordSize >, CfiCodeWordsCount> CfiSoftCodeWords;
	Cfi				CfiSoftDecoder(const SoftDecision *codedVector);
    Cfi				Index2CFI( int32_t index );
};

} // namespace LTE


#endif
