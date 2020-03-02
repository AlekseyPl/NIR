#ifndef LTE_PDCCH_H
#define LTE_PDCCH_H

#include <queue>
#include <string.h>
#include <memory>
#include "Lte/Core/Common/Quadruplet.h"
#include "Lte/Core/Common/DciSupport.h"
#include "Lte/Core/Common/Deinterleaver.h"

namespace Lte {

struct MsgDciInd
{
	Rnti      mRNTI;
	DciFormat mFormat;
	union {
		DciFormat0  mFormat0;
		DciFormat1  mFormat1;
		DciFormat1A mFormat1A;
		DciFormat1B mFormat1B;
		DciFormat1C mFormat1C;
		DciFormat1D mFormat1D;
		DciFormat2  mFormat2;
		DciFormat2A mFormat2A;
		DciFormat3  mFormat3;
		DciFormat3A mFormat3A;
	} VF;

	inline MsgDciInd( )
	{
		memset( this, 0, sizeof( *this ) );
	}

	inline MsgDciInd( Rnti rnti, DciFormat format ) :
			mRNTI( rnti ), mFormat( format )
	{
		memset( &this->VF, 0, sizeof( this->VF ) );
	}
};

class Decoder;
struct CellInfo;
using DciIndQueue = std::queue< MsgDciInd >;

/**
 *  @class Pdcch
 *  @brief Physical Downlink Control Channel.
 */
class Pdcch 	{
public:
	Pdcch(  );

	void SetDecoder( std::shared_ptr<Decoder> decoder_ );
	void SetPBCHInfo( uint32_t nDlRb, TxAntPorts antPorts );
	void SetCellInfo( const CellInfo& cell );
	void ProcessQuadruplets( const Quadruplet* q, uint32_t len, uint32_t slot, DciIndQueue &queue );

private:
	static const uint32_t UE_TX_AntSelMaskPort0 = 0x0000;
	static const uint32_t UE_TX_AntSelMaskPort1 = 0x0001;

	enum eState {
		stIdle,
		stRcvCellInfo,
		stRcvPBCHInfo,
		stReady
	};
	eState     		    m_State;
	uint32_t                    m_CX1init;
	uint32_t                    m_CX2init[ LTESubframeInFrame ];
	Environment                 m_Environment;
	FormatLen                   m_FormatLen;

	TxAntPorts                  m_AntPorts;
	uint32_t                    m_nCellId;

	Quadruplet                  m_Quad[ PDCCH_MaxREGs ];
	uint32_t                    m_QuadSize;
	int32_t                     m_MeanCntr;

	Deinterleaver               deinterleaver;
	std::shared_ptr<Decoder>    decoder;
	std::vector<uint8_t>        dciBits;

	void ChangeState( eState state );
	bool DecodeFormat1A( SoftDecision* sd, uint32_t len, DciIndQueue& queue );
	bool DecodeFormat1C( SoftDecision* sd, uint32_t len, DciIndQueue& queue );
	void CommonSearchSpaceBlindDecoder( SoftDecision* sd, uint32_t len, DciIndQueue &queue );
};

} // namespace LTE


#endif
