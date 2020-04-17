/*
 * sync_code.cpp
 *
 *  Created on: 19.02.2015
 *      Author: dblagov
 */

#include "Common/SyncCode.h"
#include <System/DebugInfo.h>
#include <cassert>
#include <algorithm>
#include <cmath>
#include <fstream>

using namespace std;

namespace Lte {

SyncCode::SyncCode( ) :
	  scale( 256.0 ), fft(new Math::FftSP(FFTLEN))
{
	inIfftData.resize(FFTLEN);
	outIfftData.resize(FFTLEN);
}

void SyncCode::CalcIfft( const ComplexFloat* freqDataIn, ComplexFloat* timeDataOut )
{
	const int32_t startPos0 = FFTLEN / 2 - LTESyncCodeHalfLen;
	const int32_t startPos1 = FFTLEN / 2 + 1;

	outIfftData.assign(outIfftData.size(), ComplexFloat{0.0, 0.0});
	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
		outIfftData[ i + startPos0 ] = freqDataIn[ i ];
		outIfftData[ i + startPos1 ] = freqDataIn[ i + LTESyncCodeHalfLen ];
	}

	fft->Shift(outIfftData.data(), inIfftData.data());
	fft->Undo(inIfftData.data(),outIfftData.data());
	//  calc max amp
	float maxamp = 0.0;
	for(const auto& el : outIfftData) {
		float curramp = abs(el);
		if( curramp > maxamp ) maxamp = curramp;
	}

	std::transform(outIfftData.begin(),outIfftData.end(), timeDataOut,
				   [maxamp,this](ComplexFloat val) { return val/maxamp*this->scale;});
}

PrimarySyncCode::PrimarySyncCode( )
{
	for( auto& p : pss)		p.resize(FFTLEN);
	Generate( );
}

void PrimarySyncCode::Generate( )
{
	ComplexFloat pssTmp[ LTESyncCodeLen ];
	const int32_t u[ PSS_COUNT ] = { 25, 29, 34 };

	for( uint32_t pssCntr = 0; pssCntr < PSS_COUNT; ++pssCntr ) {

		for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
			float re = cos( M_PI * u[ pssCntr ] * i * ( i + 1 ) / 63 );
			float im = -sin( M_PI * u[ pssCntr ] * i * ( i + 1 ) / 63 );

			pssTmp[ i ] = ComplexFloat( re, im );
		}

		for( uint32_t i = LTESyncCodeHalfLen; i < LTESyncCodeLen; ++i ) {
			float re = cos( M_PI * u[ pssCntr ] * ( i + 1 ) * ( i + 2 ) / 63 );
			float im = -sin( M_PI * u[ pssCntr ] * ( i + 1 )  * ( i + 2 ) / 63 );

			pssTmp[ i ] = ComplexFloat( re, im );
		}

		CalcIfft( pssTmp, pss.at(pssCntr).data() );

		amp[ pssCntr ] = 0.0;
		for( auto& p : pss[pssCntr] )
			amp[ pssCntr ] += abs2( p );
		amp[pssCntr] = Math::Sqrt(amp[pssCntr]);
	}
}

vector< ComplexFloat >& PrimarySyncCode::GetCode( uint32_t nid2 )
{
	return pss.at(nid2);
}

float PrimarySyncCode::GetAmp( uint32_t nid2 )
{
	return amp.at(nid2);
}

SecondarySyncCode::SecondarySyncCode( bool isFreqSearch ):
	fft32(LTESyncCodeHalfLen+1)
{
    if( isFreqSearch ) {
        for (uint32_t i = 0; i < 3; ++i) {
            //sssSpectrs[i].resize(LTESyncCodeHalfLen+1);
            sssSpectrsS0mX[i].resize(LTESyncCodeHalfLen+1);
            sssSpectrsS1mX[i].resize(LTESyncCodeHalfLen+1);
        }



		GenerateV2( );
	}
    else {

        for( int32_t nid2 = 0; nid2 < PSS_COUNT; ++nid2 ) {
			for(auto& s0 : sss0.at(nid2))	s0.resize(FFTLEN);
			for(auto& s1 : sss1.at(nid2))	s1.resize(FFTLEN);
		}
        Generate( );
	}
}

void SecondarySyncCode::GenerateV2()
{
	GenerateM0M1( );
	GenerateSt( );
	GenerateCt( );
	GenerateZt( );

	int32_t nid1 = 0;
    ComplexFloat s0m0 [ LTESyncCodeHalfLen+1 ];
    ComplexFloat s0m8 [ LTESyncCodeHalfLen+1 ];
    ComplexFloat s0m16[ LTESyncCodeHalfLen+1 ];



	ComplexFloat s1m1[ LTESyncCodeHalfLen+1 ];
    ComplexFloat s1m9[ LTESyncCodeHalfLen+1 ];
    ComplexFloat s1m17[ LTESyncCodeHalfLen+1 ];


	int32_t m0 = m0Val[ nid1 ];
	int32_t m1 = m1Val[ nid1 ];
	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
        s0m0    [ i ] = ComplexFloat(st[ (i + m0)      % LTESyncCodeHalfLen ], 0);
        s0m8    [ i ] = ComplexFloat(st[ (i + m0 + 8)  % LTESyncCodeHalfLen ], 0);
        s0m16   [ i ] = ComplexFloat(st[ (i + m0 + 16) % LTESyncCodeHalfLen ], 0);


        s1m1[ i ] =  ComplexFloat(st[ ( i + m1)       % LTESyncCodeHalfLen ], 0);
        s1m9[ i ] =  ComplexFloat(st[ ( i + m1 + 8)   % LTESyncCodeHalfLen ], 0);
        s1m17[ i ]=  ComplexFloat(st[ ( i + m1 + 16)  % LTESyncCodeHalfLen ], 0);
	}
	s0m0[LTESyncCodeHalfLen]=ComplexFloat(0,0);
    s0m8[LTESyncCodeHalfLen]=ComplexFloat(0,0);
    s0m16[LTESyncCodeHalfLen]=ComplexFloat(0,0);

	s1m1[LTESyncCodeHalfLen]=ComplexFloat(0,0);
    s1m9[LTESyncCodeHalfLen]=ComplexFloat(0,0);
    s1m17[LTESyncCodeHalfLen]=ComplexFloat(0,0);

    std::vector <ComplexFloat> s0m01;

    s0m01.assign(s0m0, s0m0 + LTESyncCodeHalfLen);

    std::ofstream output("/home/stepan/matlab_scripts/s0m0.dat", std::ios::binary);
    output.write(reinterpret_cast<char*>(s0m01.data()), s0m01.size() * sizeof(s0m01[0]));
    output.close();


    fft32.DoIt(s0m0,  sssSpectrsS0mX[0].data());
    fft32.DoIt(s0m8,  sssSpectrsS0mX[1].data());
    fft32.DoIt(s0m16, sssSpectrsS0mX[2].data());

    fft32.DoIt(s1m1,  sssSpectrsS1mX[0].data());
    fft32.DoIt(s1m9,  sssSpectrsS1mX[1].data());
    fft32.DoIt(s1m17, sssSpectrsS1mX[2].data());

}

void SecondarySyncCode::Generate( )
{
	GenerateM0M1( );
	GenerateSt( );
	GenerateCt( );
	GenerateZt( );

	int32_t prevM0 = -1;
	int32_t prevM1 = -1;

	int32_t m0 = 0, m1 = 0;
	int32_t s0m0[ LTESyncCodeHalfLen ];
	int32_t s1m1[ LTESyncCodeHalfLen ];
	int32_t c0[ LTESyncCodeHalfLen ];
	int32_t c1[ LTESyncCodeHalfLen ];
	int32_t z1m0[ LTESyncCodeHalfLen ];
	int32_t z1m1[ LTESyncCodeHalfLen ];

	ComplexFloat	sssTmp0[ LTESyncCodeLen ];
	ComplexFloat	sssTmp1[ LTESyncCodeLen ];

	for( int32_t nid1 = 0; nid1 < SSS_COUNT; ++nid1 ) {
		for( int32_t nid2 = 0; nid2 < PSS_COUNT; ++nid2 ) {
			m0 = m0Val[ nid1 ];
			m1 = m1Val[ nid1 ];

			if( m0 != prevM0 ) {
				for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
					s0m0[ i ] = st[ ( i + m0 ) % LTESyncCodeHalfLen ];
				}
			}

			if( m1 != prevM1 ) {
				for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ){
					s1m1[ i ] = st[ ( i + m1 ) % LTESyncCodeHalfLen ];
				}
			}

			for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
				c0[ i ] = ct[ ( i + nid2 ) % LTESyncCodeHalfLen ];
			}
			for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
				c1[ i ] = ct[ ( i + nid2 + 3 ) % LTESyncCodeHalfLen ];
			}

			if( m0 != prevM0 ) {
				for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
					z1m0[ i ] = zt[ ( i + ( m0 & 7 ) ) % LTESyncCodeHalfLen ];
				}
			}
			if( m1 != prevM1 ) {
				for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
					z1m1[ i ] = zt[ ( i + ( m1 & 7 ) ) % LTESyncCodeHalfLen ];
				}
			}

			prevM0 = m0;
			prevM1 = m1;

			for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) {
				sssTmp0[ 2 * i ] = ComplexFloat( s0m0[ i ] * c0[ i ], 0 );
				sssTmp1[ 2 * i ] = ComplexFloat( s1m1[ i ] * c0[ i ], 0 );

				sssTmp0[ 2 * i + 1 ] = ComplexFloat( s1m1[ i ] * c1[ i ] * z1m0[ i ], 0 );
				sssTmp1[ 2 * i + 1 ] = ComplexFloat( s0m0[ i ] * c1[ i ] * z1m1[ i ], 0 );
			}

			CalcIfft( sssTmp0, sss0[ nid2 ][ nid1 ].data() );
			CalcIfft( sssTmp1, sss1[ nid2 ][ nid1 ].data() );

			amp0[ nid2 ][ nid1 ] = 0.0;
			amp1[ nid2 ][ nid1 ] = 0.0;

			for( int32_t i = 0; i < FFTLEN; ++i ) {
				amp0[ nid2 ][ nid1 ] += abs2( sss0[ nid2 ][ nid1 ][ i ] );
				amp1[ nid2 ][ nid1 ] += abs2( sss1[ nid2 ][ nid1 ][ i ] );
			}

			amp0[nid2][nid1] = Math::Sqrt(amp0[nid2][nid1]);
			amp1[nid2][nid1] = Math::Sqrt(amp1[nid2][nid1]);
		}
	}
}

void SecondarySyncCode::DemodCt(ComplexFloat* signal, int32_t nid2)
{
	std::vector<float> c0;
	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i )
		c0.push_back( ct[ ( i + nid2 ) % LTESyncCodeHalfLen ]);

	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i )
		signal[ i ] *= c0[ i ];
}

void SecondarySyncCode::DemodCtZt(ComplexFloat* signal, int32_t nid2, int32_t m)
{
	std::vector<float> c1;
	std::vector<float> z1;
	for( int32_t i = 0; i < LTESyncCodeHalfLen; ++i )
        c1.push_back( ct[ ( i + nid2 + 3 ) % LTESyncCodeHalfLen ]);


	for( int32_t i = 0; i < LTESyncCodeHalfLen; ++i )
        z1.push_back( zt[ ( i + ( m & 7 ) ) % LTESyncCodeHalfLen ] );



//    std::ofstream output("/home/stepan/matlab_scripts/s0m0.dat", std::ios::binary);
//    output.write(reinterpret_cast<char*>(s0m01.data()), s0m01.size() * sizeof(s0m01[0]));
//    output.close();

	for( int32_t i = 0; i < LTESyncCodeHalfLen; ++i )
        signal[ i ] *= c1[ i ] * z1[ i ];


}

void SecondarySyncCode::GenerateM0M1( )
{
	m0Val.clear( );
	m1Val.clear( );

	m0Val.resize( LTENid1Max + 1 );
	m1Val.resize( LTENid1Max + 1 );

	for( int32_t nid1 = 0; nid1 <= LTENid1Max; ++nid1 ) {
		int32_t	qs = nid1 / 30;
		int32_t q  = ( ( nid1 + qs *( qs + 1 ) /2 ) / 30 );
		int32_t ms = nid1 + q * ( q + 1 ) / 2;

        int32_t m0 = ms % LTESyncCodeHalfLen;
        int32_t m1 = ( m0 + ( ms / LTESyncCodeHalfLen ) + 1 ) % LTESyncCodeHalfLen;

        m0Val[ nid1 ] = m0;
        m1Val[ nid1 ] = m1;
	}
}

void SecondarySyncCode::GenerateSt( )
{
	st.clear( );
	st.resize( LTESyncCodeHalfLen );

	int32_t x[ LTESyncCodeHalfLen ];
	x[ 0 ] = 0;
	x[ 1 ] = 0;
	x[ 2 ] = 0;
	x[ 3 ] = 0;
	x[ 4 ] = 1;
    for( uint32_t i = 0; i < ( LTESyncCodeHalfLen - 5 ); ++i )
        x[ i + 5 ] = ( x[ i + 2 ] + x[ i ] ) & 0x01;
    for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i )
        st[ i ] = 1 - 2 * x[ i ];
}

void SecondarySyncCode::GenerateCt( )
{
	ct.clear( );
	ct.resize( LTESyncCodeHalfLen );

	int32_t x[ LTESyncCodeHalfLen ];
	x[ 0 ] = 0;
	x[ 1 ] = 0;
	x[ 2 ] = 0;
	x[ 3 ] = 0;
	x[ 4 ] = 1;
	for( uint32_t i = 0; i < ( LTESyncCodeHalfLen - 5 ); ++i ) x[ i + 5 ] = ( x[ i + 3 ] + x[ i ] ) & 0x01;
	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) ct[ i ] = 1 - 2 * x[ i ];
}

void SecondarySyncCode::GenerateZt( )
{
	zt.clear( );
	zt.resize( LTESyncCodeHalfLen );

	int32_t x[ LTESyncCodeHalfLen ];
	x[ 0 ] = 0;
	x[ 1 ] = 0;
	x[ 2 ] = 0;
	x[ 3 ] = 0;
	x[ 4 ] = 1;
	for( uint32_t i = 0; i < ( LTESyncCodeHalfLen - 5 ); ++i ) x[ i + 5 ] = ( x[ i + 4 ] + x[ i + 2 ] + x[ i + 1 ] + x[ i ] ) & 0x01;
	for( uint32_t i = 0; i < LTESyncCodeHalfLen; ++i ) zt[ i ] = 1 - 2 * x[ i ];
}

vector< ComplexFloat >& SecondarySyncCode::GetCode( int32_t nid1, int32_t nid2, int32_t num )
{
	return ( num == 0 ) ? sss0[ nid2 ][ nid1 ] : sss1[ nid2 ][ nid1 ];
}

float SecondarySyncCode::GetAmp( int32_t nid1, int32_t nid2, int32_t num )
{
	return ( num == 0 ) ? amp0[ nid2 ][ nid1 ] : amp1[ nid2 ][ nid1 ];
}

std::array<std::vector< ComplexFloat >, 3> & SecondarySyncCode::GetSpecCode(SecondarySyncCode::SpecCode code)
{
    switch(code) {
    case SecondarySyncCode::sss0Even:
    case SecondarySyncCode::sss5Odd:
        return sssSpectrsS0mX;
        break;
    case SecondarySyncCode::sss0Odd:
    case SecondarySyncCode::sss5Even:
        return sssSpectrsS1mX;
        break;
    }
//    switch(code) {
//    case SecondarySyncCode::sssS0m0:
//        return sssSpectrs[0];
//        break;
//    case SecondarySyncCode::sssS0m8:
//        return sssSpectrs[1];
//        break;
//    case SecondarySyncCode::sssS0m16:
//        return sssSpectrs[2];
//        break;
//    case SecondarySyncCode::sssS1m1:
//        return sssSpectrs[3];
//        break;
//    case SecondarySyncCode::sssS1m9:
//        return sssSpectrs[4];
//        break;
//    case SecondarySyncCode::sssS1m17:
//        return sssSpectrs[5];
//        break;
//	}


}

}
