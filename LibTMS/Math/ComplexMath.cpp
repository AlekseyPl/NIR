#include "ComplexMath.h"

namespace Math {

void ConvertShortToFloat(Complex16 *__restrict in, ComplexFloat *__restrict out, uint32_t length, uint32_t inStride, uint32_t outStride) {
	for (uint32_t i = 0; i < length; ++i) {
		out->Real(static_cast<float>(in->re));
		out->Imag(static_cast<float>(in->im));
		out += outStride;
		in += inStride;
	}
}

void ConvertFloatToShort(ComplexFloat *__restrict in, Complex16 *__restrict out, uint32_t length, uint32_t inStride, uint32_t outStride) {

	for (uint32_t i = 0; i < length; ++i) {
		out->re = static_cast<int16_t>(in->Real());
		out->im = static_cast<int16_t>(in->Imag());
		out += outStride;
		in += inStride;
	}
}



} // namespace Math
