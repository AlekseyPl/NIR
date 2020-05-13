/*
 * RateRecovery.cpp
 *
 *  Created on: Apr 28, 2017
 *      Author: aplotnikov
 */
#include <algorithm>
#include <stdlib.h>
#include <cmath>

#include "Decoder/RateRecovery.h"
#include <Math/Util.h>
#include <Math/ComplexMath.h>
#include <System/DebugInfo.h>

namespace Lte {

template void RateRecovery::CcRateRecovery(SoftDecision* pDk,  uint32_t eSize, int8_t* dk, uint32_t dSize);
template void RateRecovery::CcRateRecovery(SoftDecision* pDk,  uint32_t eSize, SoftDecision* dk, uint32_t dSize);

template void RateRecovery::Normalize(std::vector<float>& in, SoftDecision* out, uint32_t size, float scale);
template void RateRecovery::Normalize(std::vector<float>& in, int8_t* out, uint32_t size, float scale);
template void RateRecovery::Normalize(std::vector<float>& in, float* out, uint32_t size, float scale);

RateRecovery::RateRecovery( ):
	TurboBlockLen( 0 ),
	debug( System::DebugInfo::Locate())
{

}

RateRecovery::~RateRecovery( )
{

}



template<typename T>
void RateRecovery::CcRateRecovery( SoftDecision* ek, uint32_t eSize, T* dk, uint32_t dSize )
{

	SbInfo		info;
	deinterleaver.SubBlockInterleaverInfo( dSize, info );
	uint32_t codeSize = ConvCodeRate * dSize;

	if(floatSd.size() != codeSize ) floatSd.resize(codeSize);
	floatSd.assign(codeSize, 0.0);

	std::vector<T> rr(codeSize);
	if ( codeSize < eSize ) {

		uint32_t shift =  0;
		while( shift < eSize ) {
			uint32_t len = std::min( codeSize, ( eSize - shift ) );
			for( uint32_t i = 0; i < len; ++i) floatSd[i] += static_cast<float>(ek[shift+i]);

			shift += codeSize;
		}
		float scale = sizeof(decltype(dk[0])) == 1 ? ccVcpScale : ccSoftScale;
		Normalize<T>(floatSd, rr.data(), codeSize, scale);
		eSize = codeSize;
	}

	auto in = rr.data();
	uint32_t bits = deinterleaver.CcSubBlockDeinterleaver<T>( in, eSize, dk, 0, info);
	bits += deinterleaver.CcSubBlockDeinterleaver<T>( &in[ bits ], eSize - bits, dk, 1, info);
	bits += deinterleaver.CcSubBlockDeinterleaver<T>( &in[ bits ], eSize - bits, dk, 2, info );
}

void RateRecovery::CtcRateRecovery( SoftDecision* ek, CtcRmParams& params, uint32_t fillerBits, float* dk )
{
	if( params.SbiInfo.Kp != TurboBlockLen ) {

		TurboBlockLen = params.SbiInfo.Kp ;

		ek0.resize( params.SbiInfo.Kp );
		ek1.resize( params.SbiInfo.Kp );
		ek2.resize( params.SbiInfo.Kp );
	}
	ek0.assign( params.SbiInfo.Kp,0.0 );
	ek1.assign( params.SbiInfo.Kp,0.0 );
	ek2.assign( params.SbiInfo.Kp,0.0 );


  	uint32_t     rateLen  = params.SbiInfo.D * TurboCodeRate;

	uint32_t     k  = 0;
	uint32_t     j  = params.k0;
	uint32_t 	 p0 = 0;
	uint32_t	 p1 = 0;
	uint32_t	 p2 = 0;
	std::vector<float> rr(rateLen);
	if ( rateLen < params.E )	{
		debug.SendText("Norm ctcRateRecovery");
		floatSd.resize(rateLen);
		floatSd.assign(rateLen,0.0);

		uint32_t shift = rateLen;
		uint32_t len = std::min( rateLen, ( params.E - shift ) );
		for( int32_t i = 0; i < len; ++i )
			floatSd[ i ] = static_cast<float>(ek[ i ]);

 		while ( shift < params.E ) {
			uint32_t len = std::min( rateLen, ( params.E - shift ) );
			for( int32_t i = 0; i < len; ++i )
				floatSd[ i ] += static_cast<float>(ek[ shift + i ]);

			shift += len;
		}

		Normalize<float>( floatSd, rr.data(), rateLen, ctcScale );
	}
	else {
		rateLen = params.E;
		debug.SendText("No norm ctcRateRecovery rateLen %d", rateLen);
		Complex16* in = reinterpret_cast<Complex16*>(ek);
		ComplexFloat* out = reinterpret_cast<ComplexFloat*>(rr.data());
		Math::ConvertShortToFloat(in,out,rateLen/2);

	}

	if ( j < params.SbiInfo.Kp ) p0 = j;
	else {
		uint32_t i = j - params.SbiInfo.Kp;
		p1 = i / 2 + ( i % 2 );
		p2 = i / 2;
	}

	deinterleaver.InterleaveFillerBits1( ek0.data(), fillerBits, params.SbiInfo ); //-V525
	deinterleaver.InterleaveFillerBits1( ek1.data(), fillerBits, params.SbiInfo );
	deinterleaver.InterleaveFillerBits2( ek2.data(), fillerBits, params.SbiInfo );

	while( k < rateLen ) {
		j %= params.Ncb;
		if ( j < params.SbiInfo.Kp )				Insert( ek0.data(), p0, rr.data(), k, params.SbiInfo.Kp );
		else {
			if ( ( j - params.SbiInfo.Kp ) & 1 )	Insert( ek2.data(), p2, rr.data(), k, params.SbiInfo.Kp );
			else									Insert( ek1.data(), p1, rr.data(), k, params.SbiInfo.Kp );
		}
		j++;
	}

	uint32_t bits = deinterleaver.CtcSubBlockDeinterleaver1( ek0.data(), &dk[0], params.SbiInfo ); //-V525
	deinterleaver.CtcSubBlockDeinterleaver1( ek1.data(), &dk[params.SbiInfo.Kp], params.SbiInfo );
	deinterleaver.CtcSubBlockDeinterleaver2( ek2.data(), &dk[2*params.SbiInfo.Kp], params.SbiInfo );
}

template<typename T>
void	RateRecovery::Normalize( std::vector<float>& in, T* out, uint32_t size, float scale)
{
	float amp = 0.0f;
	for(const auto& i: in)
		if(_fabsf(i)>amp) amp = _fabsf(i);


	for( int32_t i = 0; i < size; ++i ) {
		float res = Math::Div(in[i] , amp) * scale;
		out[ i ] = static_cast< T >(std::round(res));
	}

}


}

