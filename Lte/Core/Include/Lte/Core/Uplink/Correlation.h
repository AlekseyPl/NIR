/*
 * Correlation.h
 *
 *  Created on: Jul 9, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_CORRELATION_CORRELATION_H_
#define LTE_CORRELATION_CORRELATION_H_

#include <Common/Buffer.h>
#include <IntMath/Complex.h>
#include <System/File.h>

namespace IntMath {

class Fft32;

} // namespace IntMath

namespace Lte {

/**
 * @brief The Correlation class
 *	Расчет ВКФ одного подкадра между каналами(каждый с каждым).
 */
class Correlation {
public:
        Correlation(uint32_t numChannels, uint32_t gapCorr, Common::Allocator &allocator);
	virtual ~Correlation();

	struct Result {
	        Result() : pos(0.0), quality(0) { }
		double pos;
		int32_t quality;
	};

	void Do( Complex16* in, uint32_t stride, uint32_t startSC, uint32_t countSC );

	inline uint32_t SizeCorr() const { return corr[0]->Size(); }

	inline const uint32_t * Corr(uint32_t ch0, uint32_t ch1) const {
	        int32_t idx = GetIdx(ch0, ch1);
		return (idx == -1) ?
		                static_cast<const uint32_t *>(nullptr) : static_cast<const uint32_t *>(corr[idx]->Ptr());
	}

	inline const Result &Delay(uint32_t ch0, uint32_t ch1) const {
	        int32_t idx = GetIdx(ch0, ch1);
		return result[idx];
	}

	inline int32_t FreqOffset(uint32_t ch0, uint32_t ch1) const {
	        int32_t idx = GetIdx(ch0, ch1);
		return (idx == -1) ? 0 : freqOffset[idx];
	}

private:
	int32_t GetIdx(uint32_t ch0, uint32_t ch1) const;

	const uint32_t numCorrelations;
	const uint32_t numChannels;
	const uint32_t lenData;
	const uint32_t logLenData;
	const uint32_t logLenCorr;
	const uint32_t lenCorr;
	const uint32_t gapCorr;

	Common::Buffer<Complex32>                   tmp0;
	Common::Buffer<Complex32>                   tmp1;
	Common::Buffer<Complex32>                   tmp2;
	Common::Buffer<Common::Buffer<Complex32> *> fftOut;
	Common::Buffer<Common::Buffer<uint32_t> *>  corr;
	Common::Buffer<Result>			    result;
	Common::Buffer<int32_t>			    freqOffset;
	IntMath::Fft32*                             fft;
};

inline int32_t Correlation::GetIdx(uint32_t ch0, uint32_t ch1) const {
        int32_t idx = 0;
	for (uint32_t chI = 0; chI < numChannels - 1; ++chI) {
	        for (uint32_t chJ = chI + 1; chJ < numChannels; ++chJ) {
		        if((ch0 == chI) && (ch1 == chJ))
			        return idx;
			++idx;
		}
	}
	return -1;
}

}   /* namespace Lte */

#endif /* LTE_CORRELATION_CORRELATION_H_ */
