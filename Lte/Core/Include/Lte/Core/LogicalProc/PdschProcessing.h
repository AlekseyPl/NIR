#ifndef LTE_PDSCH_H
#define LTE_PDSCH_H

#include "Support/LteSupport.h"
#include "LogicalProc/TranspBlock.h"
#include <memory>

namespace System {
    class DebugInfo;
}

namespace Lte {

class Decoder;

/**
 *  @class tPDSCH
 *  @brief ���������� Physical Downlink Shared CHannel.
 */
class Pdsch
{
public:
	Pdsch( std::shared_ptr<Decoder>&	decoder );
	~Pdsch( );
	bool ProcessTranspBlock( const TranspBlockParam& param );

	SysInformationBlock1&	GetSib1( )
	{
		 return sib1;
	}

private:

    static  const uint32_t	MaxTrBlocks = 8;

    uint32_t			tbs;
    Modulation		 	mod;
    uint32_t 			frameLen;
    uint32_t	        	mInitX1;
    uint32_t			mInitX2[ LTESubframeInFrame ];

    SysInformationBlock1	sib1;

    bool			isX2Init;
    CodeBlockInfo    		s_info;

    std::vector<uint8_t> 	trb;
    CtcRmParams   		rm[ MaxTrBlocks ];

    std::shared_ptr<Decoder>&	decoder;
    System::DebugInfo&		debug;

    void 		XInit( uint32_t cInit, uint32_t& x2 );
    bool		Mcs( uint32_t mcs, uint32_t rb, uint32_t& tbs, Modulation& m );
    void		CtcRmInfo( const CodeBlockInfo& codeBlockInfo, const CtcRmConsts& consts, CtcRmParams* params );
    void 		ComputeCodeBlockInfo( uint32_t codeBlockSize, CodeBlockInfo& info );
    uint32_t            GetCrcRv( uint8_t* bits, uint32_t size, uint32_t crcLen );
};

} // namespace LTE


#endif
