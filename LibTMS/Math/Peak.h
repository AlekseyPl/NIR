#ifndef Math_Peak_H_
#define Math_Peak_H_

#include <cmath>
#include <stdint.h>

namespace Math {

struct Peak {
    float max;
    int32_t pos;
    float mean;
    float quality;

    Peak(float max = 0, int32_t pos = 0, float mean = 0, float quality = 0):
        max(max), pos(pos), mean(mean), quality(quality) {}
};

Peak GetPeak(const float* __restrict data, uint32_t length);

inline float EstimateMaxPos(float corr0, float corr1, float corr2) {
    return (corr2 - corr0) / (4 * corr1 - 2 * (corr2 + corr0));
}


}

#endif
