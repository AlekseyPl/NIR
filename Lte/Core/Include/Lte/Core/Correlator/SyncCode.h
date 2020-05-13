/*
 * sync_code.h
 *
 *  Created on: 19.02.2015
 *      Author: dblagov
 */

#ifndef SYNC_CODE_H_
#define SYNC_CODE_H_

#include <stdint.h>
#include <vector>
#include <complex>
#include <array>
#include <memory>

#include "Lte/Core/Support/LteDemConst.h"
#include <Math/FftSP.h>
#include <Math/Fftwf.h>

namespace Lte {

class SyncCode {
public:
	static const int32_t FFTLEN = LTEFFTLenSync;

	static const int32_t PSS_COUNT = LTENid2Max + 1;
	static const int32_t SSS_COUNT = LTENid1Max + 1;
	SyncCode( );
	virtual ~SyncCode( ) {}

	float	GetScale( ){ return scale; }

protected:
	const float scale;

	void	CalcIfft( const ComplexFloat* freqSyncCode, ComplexFloat* timeSyncCode );

private:
	std::shared_ptr<Math::FftSP> 	fft;
	std::vector<ComplexFloat> inIfftData;
	std::vector<ComplexFloat> outIfftData;

	void	FftShift( ComplexFloat* in, ComplexFloat* out );
};

class PrimarySyncCode : public SyncCode {
public:
	PrimarySyncCode( );
	virtual ~PrimarySyncCode( ){ }

	std::vector< ComplexFloat >& 	GetCode( uint32_t nid2 );
	float						 	GetAmp( uint32_t nid2 );

private:
	std::array<std::vector< ComplexFloat >, PSS_COUNT> 	pss;
	std::array<float, PSS_COUNT>						amp;

	void	Generate( );
};


class SecondarySyncCode : public SyncCode {
public:
	SecondarySyncCode( bool isFreqSearch = true );
	virtual ~SecondarySyncCode( ){ }
    enum SpecCode {
        sss0Even = 0,
        sss0Odd  = 1,
        sss5Even = 2,
        sss5Odd  = 3,
    };

	void DemodCt(ComplexFloat* signal, uint32_t nid2);
	void DemodCtZt(ComplexFloat* signal, uint32_t nid2, int32_t m);

	std::vector<ComplexFloat>& 	GetCode( uint32_t nid1, uint32_t nid2, uint32_t num );
	std::vector<ComplexFloat>&	GetSpecCode(SecondarySyncCode::SpecCode code);
	float GetAmp( uint32_t nid1, uint32_t nid2, uint32_t num );

private:

	using SssCodesArray = std::array< std::vector< ComplexFloat >, SSS_COUNT >;
	std::array<SssCodesArray, PSS_COUNT> sss0;
	std::array<SssCodesArray, PSS_COUNT> sss1;

	std::array<std::array<float, SSS_COUNT>, PSS_COUNT>	amp0;
	std::array<std::array<float, SSS_COUNT>, PSS_COUNT> amp1;

	std::vector< uint32_t >		m0Val;
	std::vector< uint32_t >		m1Val;
	std::vector< int32_t >		st;
	std::vector< int32_t >		ct;
	std::vector< int32_t >		zt;


    std::vector<ComplexFloat> sssSpectrsS0mX;
    std::vector<ComplexFloat> sssSpectrsS1mX;

	void	GenerateTime( );
	void	GenerateFreq( );

	void	GenerateM0M1( );
	void	GenerateSt( );
	void	GenerateCt( );
	void	GenerateZt( );
};

}



#endif /* SYNC_CODE_H_ */
