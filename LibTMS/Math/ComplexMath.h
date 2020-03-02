#ifndef Math_ComplexMath_H_
#define Math_ComplexMath_H_

#include "IntComplex.h"
#include "Complex.h"

namespace Math {


void    ConvertShortToFloat(Complex16 *__restrict in, ComplexFloat *__restrict out, uint32_t length, uint32_t inStride = 1, uint32_t outStride = 1);


void    ConvertFloatToShort(ComplexFloat *__restrict in, Complex16 *__restrict out, uint32_t length, uint32_t inStride = 1, uint32_t outStride = 1);

}

#endif
