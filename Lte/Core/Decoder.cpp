/*
 * TurboDecoder.cpp
 *
 *  Created on: Sep 13, 2016
 *      Author: aplotnikov
 */

#include "Decoder.h"
#include "SoftConvDecoder.h"
#include "SoftTurboDecoder.h"
#include "Common/RateRecovery.h"
#include "Common/TurboCodeSupport.h"
#include <System/DebugInfo.h>
//#include <Chip/Core.h>


#define VCP_SOFT
#define TCP_SOFT

namespace {
#pragma DATA_ALIGN(128)
static int8_t branchMetrics[1024];
#pragma DATA_ALIGN(128)
static uint32_t decis[32];
}



namespace Lte {

Decoder::Decoder(): /*tcp( NULL ), */vcp( nullptr ), frameLength( 0 ),
	  convDec( new SoftConvDecoder()), 	turboDec( new SoftTurboDecoder()),
	  rateRecovery( new RateRecovery( )),debug( System::DebugInfo::Locate() )
{
#ifndef VCP_SOFT
	vcp = Chip::VcpFactory::Create( Constraint, Chip::Vcp::rate3, 0133, 0171, 0165, 0 );
#endif
#ifndef TCP_SOFT
	tcp = Chip::TcpFactory::Create( Chip::Tcp::_3GPP, Chip::Tcp::rate1_3 );
	MakeTurboFakeRound( );
#endif
}

Decoder::~Decoder( )
{
#ifndef TCP_SOFT
	Chip::TcpFactory::Release( tcp );
#endif
#ifndef VCP_SOFT
	Chip::VcpFactory::Release( vcp );
#endif
}

bool 	Decoder::TurboDecodeSCH( SoftDecision *data, CtcRmParams& rm, uint32_t fillerBits, CrcType crcType )
{
	if( frameLength != rm.SbiInfo.D - TurboCodeTail ) {
		frameLength  = rm.SbiInfo.D - TurboCodeTail;
		encodedf.resize( rm.SbiInfo.Kp  * TurboCodeRate );
		hd.resize( ( frameLength + 31 ) / BitsInUInt32 );
		bits.resize( ( frameLength - TurboCRC_Size + 7 ) / BitsInUInt8 );
	}

	uint32_t msgLength = frameLength - TurboCRC_Size;
	debug.SendText("Start CTC recovery" );

	rateRecovery->CtcRateRecovery( data, rm, fillerBits, encodedf.data( ) );

	size_t  		 index      = ComputeCtcInterleaverParams( frameLength );
	CtcInterleaver   il         = CtcInterleaverParams[ index ];
	debug.SendText("IL index%d Params: i = %d Ki = %d f1 = %d f2 = %d", index, il.i, il.Ki, il.f1, il.f2);
	auto pData = encodedf.data();
	auto dk0 = &pData[0];
	auto dk1 = &pData[rm.SbiInfo.Kp];
	auto dk2 = &pData[2 * rm.SbiInfo.Kp];

	turboDec->Configure(il);
	bool res =turboDec->Do( dk0, dk1, dk2, crcType, hd.data());
	ExtractMsgBits(hd.data(),msgLength, bits.data());

	if (res) debug.SendText("SharedCH crc check success");
	else	 debug.SendText("SharedCH crc check failed" );

	return res;
}


bool 	Decoder::ViterbiDecodeBCH( SoftDecision* data, uint32_t srcLen, uint32_t dstLen, TxAntPorts& exPorts )
{
	bool crc = false;
	if( frameLength != dstLen + BCH_CRC_Size ) {
		frameLength   = dstLen + BCH_CRC_Size;
#ifndef VCP_SOFT
		vcp->Init( frameLength );
		encoded.resize( frameLength * ConvCodeRate );
#else
		encoded16.resize( frameLength * ConvCodeRate );
#endif
		bm.resize( ( frameLength +  Constraint - 1 ) * ( ConvCodeRate + 1 ) );
		hd.resize( ( frameLength + 31 ) / BitsInUInt32 );
		bits.resize( ( dstLen + 7 ) / BitsInUInt8 );
	}

#ifndef VCP_SOFT

	rateRecovery->CcRateRecovery<int8_t>( data, srcLen, encoded.data(), frameLength );
	vcp->GenerateBranchMetricsTailBiting( encoded.data(), branchMetrics );
	vcp->Decode( branchMetrics, decis );
#else
	rateRecovery->CcRateRecovery<int16_t>( data, srcLen, encoded16.data(), frameLength );
	convDec->Process( encoded16.data(), decis, frameLength );
#endif

	uint32_t rv = ExtractRvCrc(decis, dstLen, BCH_CRC_Size);
	ExtractMsgBits(decis, dstLen, bits.data());
	uint32_t cv = BchCheckCrc( bits.data() );

	if ( cv == rv ) {
		crc      = true;
		exPorts  = lteTxAntPorts1;
	}
	else
		if ( cv == ( rv ^ 0xFFFF ) ) {
			crc      = true;
			exPorts  = lteTxAntPorts2;
		}
		else
			if ( cv == ( rv ^ 0x5555 ) ) {
				crc      = true;
				exPorts  = lteTxAntPorts4;
			}
	return crc;
}

bool Decoder::ViterbiDecodeCCH( SoftDecision* data, uint32_t srcLen, uint32_t dstLen )
{
	if( frameLength != dstLen + DCI_CRC_Size ) {
		frameLength  = dstLen + DCI_CRC_Size;
#ifndef VCP_SOFT
		vcp->Init( frameLength );
		encoded.resize( frameLength * ConvCodeRate );
#else
		encoded16.resize( frameLength * ConvCodeRate );
#endif
		bm.resize( ( frameLength + Constraint - 1 ) * ( ConvCodeRate + 1 ) );
		hd.resize( ( frameLength + 31 ) / BitsInUInt32 );

		bits.resize( ( dstLen + 7 ) / BitsInUInt8 );
	}
#ifndef VCP_SOFT

	rateRecovery->CcRateRecovery<int8_t>( data, srcLen, encoded.data(), frameLength );
	vcp->GenerateBranchMetricsTailBiting( encoded.data(), branchMetrics );
	vcp->Decode( branchMetrics, decis);
#else

	rateRecovery->CcRateRecovery<int16_t>( data, srcLen, encoded16.data(), frameLength );
	convDec->Process( encoded16.data(), hd.data(), frameLength );
#endif

	uint16_t rv = ExtractRvCrc(decis, dstLen, DCI_CRC_Size);
	ExtractMsgBits(decis, dstLen, bits.data());
	rv ^= SI_RNTI;
	TopsyBits( &rv, DCI_CRC_Size );

	return rv == crc16( bits.data(), dstLen  );
}
void 	Decoder::GenInterleaverTable( )
{
#ifndef TCP_SOFT

	rateRecovery->CtcRateRecovery<int8_t>( data, rm, fillerBits, encoded.data() );
	int res = tcp->Decode( encoded.data(), hd.data() );
	uint32_t crc_rv = ExtractCrc( msgLength, TurboCRC_Size );
	return CheckCrc( bits.data(), crcType, crc_rv, frameLength );


	size_t  		 index      = ComputeCtcInterleaverParams( frameLength );
	CtcInterleaver   il         = CtcInterleaverParams[ index ];
	ilTable.resize( frameLength );
	debug.SendText("Il.ki %d , frLength %d idex %d", frameLength, il.Ki, index );
	if ( frameLength == il.Ki ) {
		for ( uint32_t j = 0; j < frameLength; ++j )
			ilTable[ j ] = ( il.f1 * j + il.f2 * j * j ) % il.Ki;
	}
	else
		debug.SendText( "Interleaver Table generation error, frame length = %d", frameLength );
#endif
}




}


