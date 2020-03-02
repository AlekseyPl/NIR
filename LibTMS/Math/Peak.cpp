#include "Peak.h"
#include <algorithm>

namespace Math {

Peak GetPeak(const float *__restrict in, uint32_t length) {
	int64_t pos = 0;
	float max = 0.0;
	float mean = 0.0;
	float quality = 0.0;

	for (unsigned i = 0; i < length; ++i) {
		if (in[i] > max) {
			max = in[i];
			pos = i;
		}
		mean += in[i];
	}
	int64_t leftPos = (pos - 1) % length;
	int64_t rightPos = (pos + 1) % length;

	mean -= max;
	mean -= in[leftPos];
	mean -= in[rightPos];
	mean /= (length - 3);

	quality = (100.0 * (max - mean)) / max;
	if ((quality < 0) || (quality > 100))
		quality = 0; // overflow

	return Peak(max, pos, mean, quality);
}

} // namespace Math
