#ifndef INCLUDE_LTE_SOFTTURBODECODER_H_
#define INCLUDE_LTE_SOFTTURBODECODER_H_

#include "Support/LteConst.h"
#include "Support/LteTypes.h"
#include "Support/LteSupport.h"
#include "Decoder/Crc.h"
#include "Decoder/TurboCodeSupport.h"

#include <vector>
#include <array>
#include <cstring>

namespace System {
    class DebugInfo;
}

namespace Lte {

class SoftTurboDecoder {
public:
    SoftTurboDecoder();
    ~SoftTurboDecoder();

    void    Configure(const CtcInterleaver& interleaver);
    bool    Do( const float* dk0, const float* dk1, const float* dk2, CrcType crcType, uint32_t* res);

private:
    const float max_llr = 1.0e6;

    struct RSCDecoder {
        const float *pLu;
        const float *pLc;
        float *pG11;
        float *pG10;
        float *pA;
        float *pB;
        float *pLambda;
        size_t FrameLen;
    };


    RSCDecoder      rsc;

    std::vector<float> Lu;
    std::vector<float> Lc1;
    std::vector<float> Lc2;
    std::vector<float> Lambda1;
    std::vector<float> Lambda2;
    std::vector<float> g11;
    std::vector<float> g10;
    std::vector<float> alfa;
    std::vector<float> beta;

    std::array<float, TurboCodeTail - 1> Lu1Tail;
    std::array<float, TurboCodeTail - 1> Lu2Tail;

    size_t frameLen;
    size_t len;
    size_t alfaLen; //-V112
    size_t f1;
    size_t f2;
    size_t payloadSize;

    System::DebugInfo& debug;

    void Init(const float* dk0, const float* dk1, const float* dk2);

    void Alfa(const float *pG10, const float *pG11, size_t vFrameLen, float *pA);
    void Beta(const float *pG10, const float *pG11, size_t vFrameLen, float *pB);
    void Gamma(const float *pLu, const float *pLc, size_t vFrameLen, float *pG11, float *pG10);
    void Extrinsic(const float *pA, const float *pB, const float *pG10, const float *pG11, size_t vFrameLen,
                               float *pExtrinsic);
    void RscDecoder(RSCDecoder& RSC);

    void Interleaver(const float *pSrc, float *pDst, size_t vLen, size_t f1, size_t f2);
    void Deinterleaver(const float *pSrc, float *pDst, size_t vLen, size_t f1, size_t f2);

    inline float MaxStar(float v1, float v2);
    inline void HardDecisions(const std::vector<float>& Soft, size_t vLen, uint8_t *pHard);
};

inline float SoftTurboDecoder::MaxStar(float v1, float v2)
{
    return (v1 > v2) ? v1 : v2;
}

inline void SoftTurboDecoder::HardDecisions(const std::vector<float>& Soft, size_t vLen, uint8_t *pHard) {
    std::memset(pHard, 0, (vLen + 7) / 8);
    for (size_t i = 0; i < vLen; ++i)
        if (Soft[i] < 0)    SetBit(pHard,i);
        else                ResetBit(pHard,i);
}

} // namespace Lte

#endif /* INCLUDE_SOFTCONVDECODER_H_ */
