/*
 * filter.h
 *
 *  Created on: 25.02.2015
 *      Author: dblagov
 */

#ifndef FILTER_FFT_H_
#define FILTER_FFT_H_

#include "Lte/Core/Support/LteDemConst.h"
#include "FilterAfr.h"
#include <Math/FftSP.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <memory>


namespace Lte {

class Filter {
public:
	Filter( uint32_t nFFT = 2048);
	virtual ~Filter( ) { }

    uint32_t GetFilterSize( ) const;
    void    SetDecimFactor( uint32_t decimFactor );

    void    Process( const Complex16* in, uint32_t step );
    void    GetFilteredData( ComplexFloat* out, uint32_t startPos, uint32_t stopPos, uint32_t decFactor = LTEDownFactorSync);
private:
	FilterAfr afr;
	uint32_t             decimFactor;

	uint32_t sliceCount;
	uint32_t winSize;
	uint32_t fullSize;
	uint32_t fftSize;
	uint32_t blockSize;

	std::vector<ComplexFloat> winFft;
	std::vector<ComplexFloat> sigFftSlice;
	std::vector<ComplexFloat> sigSlice;

	std::vector<ComplexFloat> sliceResT;
	std::vector<ComplexFloat> sliceResF;

	std::vector<ComplexFloat> filteredSig;
	std::vector< ComplexFloat >	iq;
	std::shared_ptr<Math::FftSP> fft;

	void PrepareWin( );

	inline void PrepareSigHalfZero(const ComplexFloat* sig, uint32_t count);
	inline void ClearSlice(std::vector<ComplexFloat>& slice);

};

}



#endif /* FILTER_H_ */
