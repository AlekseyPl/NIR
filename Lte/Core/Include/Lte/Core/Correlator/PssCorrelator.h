#ifndef Lte_PssCorrelator_H_
#define Lte_PssCorrelator_H_

#include "Correlator/SyncCode.h"
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

	struct PssRes {
		uint32_t pos;
		float val;
	};

	explicit PssCorrelator(uint32_t corrSize);
	void ReInit(uint32_t corrSize);

	void Correlate(const ComplexFloat* signal, uint32_t nid2 = SyncCode::PSS_COUNT );
	void CorrelateThin(const ComplexFloat* signal, float* absCorr, uint32_t nid2, uint32_t gap );

	const PssRes GetCorrMaxPos(uint32_t nid2);

private:
	const ComplexFloat zero;
	std::shared_ptr<Math::FftSP> fft;
	PrimarySyncCode     pss;

	uint32_t sliceCount;
	uint32_t winSize;
	uint32_t corrSize;
	uint32_t fftSize;
	uint32_t blockSize;

	std::array<std::vector<ComplexFloat>, SyncCode::PSS_COUNT> winFftSlice;

	std::vector<ComplexFloat> sigFftSlice;
	std::vector<ComplexFloat> sigSlice;

	std::vector<ComplexFloat> sliceResT;
	std::vector<ComplexFloat> sliceResF;
	std::array<std::vector<ComplexFloat>, SyncCode::PSS_COUNT> corr;
	std::array<std::vector<float>, SyncCode::PSS_COUNT> absCorr;
	std::vector<float> sigAmps;

	void PrepareWin();
	void CorrSpecifyNid(uint32_t sliceCntr, uint32_t nid2, uint32_t progress);
	void CalcPowers(const ComplexFloat *sig);

	inline void PrepareSigHalfZero(const ComplexFloat* sig, uint32_t count);
	inline void ClearSlice(std::vector<ComplexFloat>& slice);

};

}

#endif
