#include "LogicalProc/PdschProcessing.h"
#include "Support/TrBlockSizeTable.h"
#include "Support/PnSequence.h"
#include "Decoder/Crc.h"
#include "Decoder/TurboCodeSupport.h"
#include "Decoder/Deinterleaver.h"
#include "Decoder/Decoder.h"

#include <System/DebugInfo.h>

#include <string>

namespace Lte {
/************************************************************************/
/*                             tPDSCH                                   */
/************************************************************************/
Pdsch::Pdsch( std::shared_ptr<Decoder>&	decoder_ ):
	decoder( decoder_ ), mInitX1( 0 ), tbs( 0 ),	isX2Init( false ), frameLen( 0 ), mod( lteQPSK ),
	debug( System::DebugInfo::Locate( ) )
{
	sib1.reserve(100);
}

Pdsch::~Pdsch( )
{

}

bool Pdsch::ProcessTranspBlock( const TranspBlockParam& param )
{
	bool crc = false;
	sib1.clear();
	uint32_t x2;
	XInit( param.mCinit, x2 );
	SoftDecision *p = const_cast<SoftDecision*>(param.mSd.data( ));
	Scrambler( mInitX1, x2, p, param.mSize );
	bool ok = false;
	if ( param.mFormat1C ) 	{
		if ( param.mTbsIndex < Max_TBS_Index_1C ) {
			tbs = TrBlockSize1C[ param.mTbsIndex ];
			mod = lteQPSK;
			ok = true;
		}
	}
	else 	ok = Mcs( param.mMcs, param.mRb, tbs, mod );

	if( ok ) {
 		ComputeCodeBlockInfo( tbs + TurboCRC_Size, s_info );
		CtcRmConsts rm_consts( param.mSize, param.mRv, mod );

		CtcRmInfo( s_info, rm_consts, &rm[ 0 ] );

		trb.resize( tbs / BitsInUInt8 );

		frameLen = tbs + TurboCRC_Size;
		debug.SendText("TrBlock size %d and with CRC %d", tbs, frameLen );


		uint32_t e = 0;
		uint32_t progress = 0;
		CrcType crcType = ( s_info.C == 1 ) ? lteCRC24A : lteCRC24B;
		for ( uint32_t r = 0; r < s_info.C; ++r ) {
			crc = decoder->TurboDecodeSCH( &p[ e ], rm[ r ], r ? 0 : s_info.F , crcType );
		 	if( crc ) {
				uint32_t	dSize = tbs / BitsInUInt8;
				decoder->GetBits( trb.data()+ progress, dSize );
				progress += dSize;
				e += rm[ r ].E;

			}
		 	else 	break;
		}



		if( s_info.C > 1 ) {

			trb.assign(s_info.F / BitsInUInt8,0 );

			uint32_t crc_rv = GetCrcRv( trb.data( ), trb.size( ), TurboCRC_Size );
			crc = CheckCrc( trb.data( ), lteCRC24A, crc_rv, frameLen );
			std::memmove( trb.data( ), trb.data( ) + s_info.F, s_info.F * sizeof( uint8_t ) );
		}

		for(const auto& s: trb)
			sib1.push_back(s);

 	}
	return crc;

}

bool Pdsch::Mcs( uint32_t mcs, uint32_t rb, uint32_t& tbs, Modulation& m )
{
	bool r = ( mcs <= Max_MCS_Index );
	if ( r ) {
		m = McsMod[ mcs ];
		uint32_t Itbs = TbsIndex[ mcs ];
		tbs = TrBlockSizeTable[ Itbs ][ rb - 1 ];
	}
	return r;
}

inline void Pdsch::XInit( uint32_t cInit, uint32_t& x2 )
{
	uint32_t subframe = ( cInit >> 9 ) & 0xF;
	if ( !isX2Init ) {
		uint32_t c = cInit - ( subframe << 9 );
		for ( uint32_t i = 0; i < LTESubframeInFrame; ++i )
			PnSeqInit( c + ( i << 9 ), &mInitX1, &mInitX2[ i ]);
		isX2Init = true;
	}
	x2 = mInitX2[ subframe ];
}

void Pdsch::CtcRmInfo( const CodeBlockInfo& codeBlockInfo, const CtcRmConsts& consts, CtcRmParams* params )
{
	uint32_t Kmimo = ( ( consts.TM == lteTM3 ) || ( consts.TM == lteTM4 ) ) ? 2 : 1;
	uint32_t Nir   = ( consts.Nsoft / (Kmimo * std::min(consts.MaxDlHarqProc, M_DL_HARQ_Limit)) );
	for ( uint32_t r = 0; r < codeBlockInfo.C; ++r )	{
		uint32_t D    = ( ( r < codeBlockInfo.C_plus ) ? codeBlockInfo.K_plus : codeBlockInfo.K_minus ) + TurboCodeTail;
		Deinterleaver::SubBlockInterleaverInfo( D, params[ r ].SbiInfo );
		uint32_t Kp   = params[ r ].SbiInfo.Kp;
		uint32_t Ncb  = consts.Uplink ? ( Kp * TurboCodeRate ) : std::min( Nir / codeBlockInfo.C, Kp * TurboCodeRate );
		uint32_t Nl   = ( consts.Nl == 1 ) ? 1 : 2;
		uint32_t Qmod = Qm( consts.Mod );
		uint32_t Gs   = consts.G / ( Nl * Qmod );
		uint32_t y    = Gs % codeBlockInfo.C;
		uint32_t k0   = params[ r ].SbiInfo.R_tc * ( 2 * DivideToPlusInf( Ncb, ( 8 * params[ r ].SbiInfo.R_tc ) ) * consts.rv + 2 );
		params[ r ].Ncb = Ncb;
		params[ r ].E   = Nl * Qmod * ( (r <= ( codeBlockInfo.C - y - 1) ) ? ( Gs / codeBlockInfo.C ) : DivideToPlusInf( Gs, codeBlockInfo.C ) );
		params[ r ].k0  = k0;

	}
}

void Pdsch::ComputeCodeBlockInfo( uint32_t codeBlockSize, CodeBlockInfo& info )
{
	memset( &info, 0, sizeof( info ) );
	info.B   = codeBlockSize;
	uint32_t Z = MaxCTCBlockSize;
	if ( info.B <= Z )	{
		info.L  = 0;
		info.C  = 1;
		info.Bs = info.B;
		if ( info.B < MinCTCBlockSize )
			info.F = MinCTCBlockSize - info.B;
	}
	else	{
		info.L = 24;
		info.C = info.B / ( Z - info.L );
		if ( ( info.C * ( Z - info.L ) ) != info.B )
			info.C++;
		info.Bs = info.B + info.C * info.L;
	}
	for ( uint32_t i = 0; i < CtcInterleaverParamsCount; ++i )
		if ( ( info.C * CtcInterleaverParams[ i ].Ki ) >= info.Bs )		{
			info.K_plus  = CtcInterleaverParams[ i ].Ki;
			info.K_minus = ( i && ( info.C > 1 ) ) ? CtcInterleaverParams[ i - 1 ].Ki : 0;
			break;
		}

	if ( info.C == 1 )	{
		info.C_plus  = 1;
		info.C_minus = 0;
		info.K_minus = 0;
	}
	else	{
		uint32_t deltaK = info.K_plus - info.K_minus;
		info.C_minus  = ( info.C * info.K_plus - info.Bs ) / deltaK;
		info.C_plus   = info.C - info.C_minus;
	}
	info.F = info.C_plus * info.K_plus + info.C_minus * info.K_minus - info.Bs;
}

uint32_t 	Pdsch::GetCrcRv( uint8_t* bits, uint32_t size, uint32_t crcLen )
{
	uint32_t rv = 0;
	uint32_t elemCount = crcLen / BitsInUInt8;

	for( uint32_t i = 0; i < elemCount; ++i ) {

		uint8_t value = bits[ size - elemCount + i ];
		for( uint32_t j = 0; j < BitsInUInt8; ++j ) {
			if( value & 1 )	rv |= (Mask1In32 << ( i * BitsInUInt8 + j  ) );
			else			rv &= ~(Mask1In32 << ( i * BitsInUInt8 + j ) );
			value >>=1;
		}
	}
	return rv;
}
} // namespace LTE

