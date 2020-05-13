#ifndef LTE_CORRELATION_CORRELATION_V2_H_
#define LTE_CORRELATION_CORRELATION_V2_H_

#include <Common/Buffer.h>
#include <Math/Complex.h>
#include <System/File.h>

#include <memory>

namespace Math {

class FftSP;

} // namespace IntMath

namespace Lte {

/**
 * @brief The Correlation class
 *	Расчет ВКФ одного подкадра между каналами(каждый с каждым).
 */
class CorrelationV2 {
public:
    CorrelationV2(uint32_t numChannels, uint32_t gapCorr, Common::Allocator &allocator);
    virtual ~CorrelationV2();

    struct Result {
            Result() : pos(0.0), quality(0) { }
        double pos;
        int32_t quality;
    };

    void Do( Complex16* in, uint32_t stride, uint32_t startSC, uint32_t countSC );

    inline uint32_t SizeCorr() const { return corr[0]->Size(); }

    inline const float * Corr(uint32_t ch0, uint32_t ch1) const {
            int32_t idx = GetIdx(ch0, ch1);
        return (idx == -1) ?
                        static_cast<const float *>(nullptr) : static_cast<const float *>(corr[idx]->Ptr());
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

    Common::Buffer<ComplexFloat>                    tmp0;
    Common::Buffer<ComplexFloat>                    tmp1;
    Common::Buffer<ComplexFloat>                    tmp2;
    Common::Buffer<Common::Buffer<ComplexFloat> *>  fftOut;
    Common::Buffer<Common::Buffer<float> *>         corr;
    Common::Buffer<Result>                          result;
    Common::Buffer<int32_t>                         freqOffset;
    std::unique_ptr<Math::FftSP>                   fftSp;

    System::File                file;
};

inline int32_t CorrelationV2::GetIdx(uint32_t ch0, uint32_t ch1) const {
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
