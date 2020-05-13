#ifndef Lte_FilterAfr_H_
#define Lte_FilterAfr_H_

#include <stdint.h>
#include <array>

namespace Lte {

struct FilterAfr {
	FilterAfr();
	static const uint32_t mFilterSize = 174;
	std::array<float, mFilterSize> filterCoef;
};

}
#endif
