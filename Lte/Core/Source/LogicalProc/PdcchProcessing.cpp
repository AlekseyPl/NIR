#include "LogicalProc/PdcchProcessing.h"
#include "Support/CellInfo.h"
#include "Support/PnSequence.h"
#include "Decoder/Decoder.h"
#include <algorithm>
#include <System/DebugInfo.h>

using namespace std;

namespace Lte {

/************************************************************************/
/*                      ���������� ������ tPDCCH                        */
/************************************************************************/

Pdcch::Pdcch(std::shared_ptr<Decoder>& decoder ): decoder(decoder)
{
	m_Quad.reserve(PDCCH_MaxREGs);
	ChangeState( stIdle );
	dciBits.reserve(DCCH_Length8);
}

void Pdcch::SetCellInfo(const CellInfo& cell)
{
	m_nCellId             = cell.nCellId;
	m_Environment.mDuplex = cell.duplex;
	ChangeState( stIdle );

	for ( uint32_t i = 0; i < LTESubframeInFrame; ++i ){
		uint32_t	 c_init = i * ( 1 << 9 ) + m_nCellId;
		PnSeqInit( c_init, &m_CX1init, &m_CX2init[ i ] );
	}
	ChangeState( stRcvCellInfo );
}

void Pdcch::ProcessQuadruplets( const Quadruplet* q, uint32_t len, uint32_t subframe, DciIndQueue &queue )
{
	if ( len > m_Quad.size( ) )   m_Quad.resize( len );
	deinterleaver.PdcchQuadDeinterleaver( q, len, m_nCellId, m_Quad.data() );

	auto sd = reinterpret_cast<SoftDecision*>( m_Quad.data() );
	uint32_t length = len * QuadrupletSize *  BitPerSymbol_QPSK;
	// TS 6.7.1 Scrambling
	Scrambler( m_CX1init, m_CX2init[ subframe ], sd, length );
	//Decoding
	CommonSearchSpaceBlindDecoder( sd, length, queue );
}

void Pdcch::SetPBCHInfo( uint32_t nDlRb, TxAntPorts antPorts )
{
	m_Environment.mAntPorts     = antPorts;
	m_Environment.mDL_BandWidth = nDlRb;
	m_Environment.mUL_BandWidth = nDlRb;

	DefineDCIFormatLen( m_Environment, m_FormatLen );
	ChangeState( stRcvPBCHInfo );
}

void Pdcch::ChangeState( eState state )
{
	switch ( state ) {
		case stIdle:
			m_MeanCntr = 0;
			m_State = stIdle;
		break;

		case stRcvCellInfo:
			m_State = ( m_State == stRcvPBCHInfo ) ? stReady : stRcvCellInfo;
		break;

		case stRcvPBCHInfo:
			m_State = ( m_State == stRcvCellInfo ) ? stReady : stRcvPBCHInfo;
		break;

		case stReady:
			m_State = stReady;
		break;
		}
}

bool Pdcch::DecodeFormat1A( SoftDecision* sd, uint32_t len, DciIndQueue& queue )
{
	bool ok = decoder->ViterbiDecodeCCH( sd, len, m_FormatLen.mFormat1ALen );
	if ( ok ) {
		dciBits.resize((( m_FormatLen.mFormat1ALen + 7 ) / BitsInUInt8 ));
		decoder->GetBits( dciBits.data(), dciBits.size() );
		Context cntx( dciBits.data(), m_FormatLen.mFormat1ALen );
		MsgDciInd msg( SI_RNTI, dciFormat1A );
		if ( ParseDCIFormat1A( m_Environment, cntx, msg.VF.mFormat1A ) )  queue.push( msg );
	}
	return ok;
}

bool Pdcch::DecodeFormat1C( SoftDecision* sd, uint32_t len, DciIndQueue& queue )
{
 	bool ok = decoder->ViterbiDecodeCCH( sd, len, m_FormatLen.mFormat1CLen );
	if ( ok ) {
		dciBits.resize((( m_FormatLen.mFormat1CLen + 7 ) / BitsInUInt8 ));
		decoder->GetBits( dciBits.data(), dciBits.size() );
		Context cntx( dciBits.data(), m_FormatLen.mFormat1CLen );
		MsgDciInd msg( SI_RNTI, dciFormat1C );
		if ( ParseDCIFormat1C( m_Environment, cntx, msg.VF.mFormat1C ) )  queue.push( msg );
	}
	return ok;
}

void Pdcch::CommonSearchSpaceBlindDecoder( SoftDecision* sd, uint32_t len, DciIndQueue &queue )
{
	bool used[ AggLev8Count ] = { false, false };
	System::DebugInfo::Locate().SendText("Start CommonSearchSpaceBlindDecoder");
	for ( uint32_t i = 0, j = 0; ( i < AggLev4Count ) && ( ( j + AggLev4Len ) <= len ); ++i, j += AggLev4Len ) {
		if ( DecodeFormat1C( &sd[ j ], AggLev4Len, queue ) ||
			DecodeFormat1A( &sd[ j ], AggLev4Len, queue ) )
		{
			used[ i / AggLev8Count ] = true;
		}
	}
	for ( uint32_t i = 0, j = 0; ( i < AggLev8Count ) && ( ( j + AggLev8Len ) <= len ); ++i, j += AggLev8Len ) {
		if ( used[ i ] ) {
			continue;
		}
		if ( DecodeFormat1C( &sd[ j ], AggLev8Len, queue ) ) {
			continue;
		}
		DecodeFormat1A( &sd[ j ], AggLev8Len, queue );
	}
}



} // namespace LTE


