#ifndef Lte_PssCorrelator_H_
#define Lte_PssCorrelator_H_

#include "Common/SyncCode.h"
#include <Math/Complex.h>

#include <stdint.h>
#include <memory>
#include <vector>

namespace Math {
class FftSP;
}

namespace Lte {

class PssCorrelator {
public:
	explicit PssCorrelator(uint32_t corrSize);

	void Configure(uint32_t nid2);
	void Correlate(const ComplexFloat* signal, float* corr);

private:
	const ComplexFloat zero;
	std::shared_ptr<Math::FftSP> fft;
	PrimarySyncCode     pss;

	uint32_t sliceCount;

	uint32_t winSize;
	uint32_t corrSize;
	uint32_t fftSize;
	uint32_t nid2;
	void Configure( );
	std::vector<ComplexFloat> winSlice;
	std::vector<ComplexFloat> winFftSlice;

	std::vector<ComplexFloat> sigFftSlice;
	std::vector<ComplexFloat> sigSlice;

	std::vector<ComplexFloat> sliceResT;
	std::vector<ComplexFloat> sliceResF;
	std::vector<ComplexFloat> result;

	inline void PrepareWin(std::vector<ComplexFloat>& win);
	inline void PrepareSig(const ComplexFloat* sig);
	inline void PrepareSigHalfZero(const ComplexFloat* sig);
	inline void ClearSlice(std::vector<ComplexFloat>& slice);

};

}

#endif
