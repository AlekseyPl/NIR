/* ======================================================================== *
 * MATHLIB -- TI Floating-Point Math Function Library                       *
 * ======================================================================== */

#ifndef UTIL_MATH_H
#define UTIL_MATH_H

#include <cmath>
#include <stdint.h>

namespace Math {

/* ======================================================================= */
/* single precision floating point division								   */
/*              optimized inlined C implementation (w/ intrinsics)         */
/* ======================================================================= */
inline float Div(float a, float b);

/* ======================================================================= */
/* double precision floating point division								   */
/*              optimized inlined C implementation (w/ intrinsics)         */
/* ======================================================================= */
inline double Div(double a, double b);

/* ======================================================================= */
/* single precision floating point log base 10							   */
/*              optimized inlined C implementation (w/ intrinsics)         */
/* ======================================================================= */
inline float Log10(float a);

/* ======================================================================= */
/* single precision floating point sqrt				                       */
/*              optimized inlined C implementation (w/ intrinsics)         */
/* ======================================================================= */
inline float Sqrt(float a);

extern double ti_math_logtable[];
extern double ti_math_kTable[];
extern double ti_math_jTable[];
extern double ti_math_vTable[];

inline float Div(float a, float b) {
	return a / b;
}

inline double Div(double a, double b) {
	return a / b;
}

inline float Log10(float a) {
	return std::log10(a);
}

inline float Sqrt(float a) {
	return std::sqrt(a);
}

} // namespace Math

#endif // UTIL_MATH_H

/* ======================================================================== */
/*  End of file: Util.h                                                     */
/* ======================================================================== */
