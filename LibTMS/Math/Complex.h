/*
 * Complex.h
 *
 *  Created on: 28.06.2019
 *      Author:
 */

#ifndef Math_Complex_H_
#define Math_Complex_H_

#include "C6xSim/C6xSimulator.h"
#include "IntComplex.h"
#include "Util.h"

#include <cmath>
#include <stdint.h>

class ComplexFloat {
private:
	union {
		__float2_t dReg;
		struct {
			float re;
			float im;
		};
	};

public:
	explicit ComplexFloat(const __float2_t &);
	explicit ComplexFloat(const float &reVal = 0.0, const float &imVal = 0.0);
	explicit ComplexFloat(const double &re, const double &im);
	explicit ComplexFloat(const int16_t &re, const int16_t &im);
	explicit ComplexFloat(const int32_t &re, const int32_t &im);

	ComplexFloat(const ComplexFloat &z) = default;
	explicit ComplexFloat(const Complex16 &z);

	float Real() const;
	float Imag() const;
	void Real(const float &val);
	void Imag(const float &val);

	ComplexFloat &operator+=(const ComplexFloat &z);
	ComplexFloat &operator-=(const ComplexFloat &z);
	ComplexFloat &operator*=(const ComplexFloat &z);
	ComplexFloat &operator*=(const float &y);
	ComplexFloat &operator/=(const ComplexFloat &y);
	ComplexFloat &operator/=(const float &y);
	ComplexFloat &operator&=(const ComplexFloat &z) = delete;
	ComplexFloat &operator<<=(const int16_t &n) = delete;
	ComplexFloat &operator>>=(const int16_t &n) = delete;
	const ComplexFloat operator+(const ComplexFloat &z) const;
	const ComplexFloat operator-(const ComplexFloat &z) const;
	const ComplexFloat operator*(const ComplexFloat &z) const;
	const ComplexFloat operator*(const float &y) const;
	const ComplexFloat operator/(const ComplexFloat &z) const;
	const ComplexFloat operator/(const float &y) const;
	const ComplexFloat operator&(const ComplexFloat &z) const;
	//	const ComplexFloat operator&(const Complex32 &z) const;
	const ComplexFloat operator<<(const int16_t &n) const;
	const ComplexFloat operator>>(const int16_t &n) const;
	const ComplexFloat operator-() const;
	const ComplexFloat operator~() const;

	// ComplexFloat
	// operator=(const ComplexFloat &y) = default;
	ComplexFloat operator=(const float &y);
	//	ComplexFloat operator=(const Complex32 &y);
	bool operator==(const ComplexFloat &y) const;
	bool operator!=(const ComplexFloat &y) const;

	inline uint8_t width() const { return sizeof(float) * 8; }

	inline friend float abs(const ComplexFloat &x);
	inline friend float abs2(const ComplexFloat &x);
	inline friend ComplexFloat conj(const ComplexFloat &x);
	inline friend ComplexFloat conj_mpy(const ComplexFloat &x, const ComplexFloat &y);
	inline friend float Abs(const ComplexFloat &x) { return abs(x); }
	inline friend float Abs2(const ComplexFloat &x) { return abs2(x); }
	inline friend ComplexFloat Conj(const ComplexFloat &x) { return conj(x); }
	inline friend ComplexFloat ConjMpy(const ComplexFloat &x, const ComplexFloat &y) { return conj_mpy(x, y); }
};

//------------------------------------------------------------

inline ComplexFloat::ComplexFloat(const __float2_t &val) : dReg(val) {}
inline ComplexFloat::ComplexFloat(const float &re, const float &im) : re(re), im(im) {}
inline ComplexFloat::ComplexFloat(const double &re, const double &im)
	: re(static_cast<float>(re)), im(static_cast<float>(im)) {}

inline ComplexFloat::ComplexFloat(const int16_t &re, const int16_t &im) : ComplexFloat(Complex16(re, im)) {}

inline ComplexFloat::ComplexFloat(const int32_t &re, const int32_t &im) : ComplexFloat(ComplexFloat(static_cast<float>(re),
								static_cast<float>(im))) {}


inline ComplexFloat::ComplexFloat(const Complex16 &z) : re(static_cast<float>(z.re)), im(static_cast<float>(z.im)) {}


inline float ComplexFloat::Real() const { return re; }

inline float ComplexFloat::Imag() const { return im; }

inline void ComplexFloat::Real(const float &val) { this->re = val; }

inline void ComplexFloat::Imag(const float &val) { this->im = val; }

inline ComplexFloat &ComplexFloat::operator+=(const ComplexFloat &z) {
	re += z.re;
	im += z.im;
	return *this;
}

inline ComplexFloat &ComplexFloat::operator-=(const ComplexFloat &z) {
	re -= z.re;
	im -= z.im;

	return *this;
}

inline ComplexFloat &ComplexFloat::operator*=(const ComplexFloat &z) {

	float t = re * z.re - im * z.im;
	im = re * z.im + im * z.re;
	re = t;

	return *this;
}

inline ComplexFloat &ComplexFloat::operator/=(const ComplexFloat &y) {

	float nomRe = re * y.re + im * y.im;
	float nomIm = -re * y.im + im * y.re;
	float den = abs2(y);
	re = nomRe / den;
	im = nomIm / den;

	return (*this);
}

inline ComplexFloat &ComplexFloat::operator/=(const float &y) {

	re /= y;
	im /= y;
	return (*this);
}

inline ComplexFloat &ComplexFloat::operator*=(const float &y) {

	re *= y;
	im *= y;
	return (*this);
}


inline const ComplexFloat ComplexFloat::operator+(const ComplexFloat &z) const { return ComplexFloat(*this) += z; }

inline const ComplexFloat ComplexFloat::operator-(const ComplexFloat &z) const { return ComplexFloat(*this) -= z; }

inline const ComplexFloat ComplexFloat::operator*(const ComplexFloat &z) const { return ComplexFloat(*this) *= z; }

inline const ComplexFloat ComplexFloat::operator/(const ComplexFloat &z) const { return ComplexFloat(*this) /= z; }

inline const ComplexFloat ComplexFloat::operator*(const float &z) const { return ComplexFloat(*this) *= z; }

inline const ComplexFloat ComplexFloat::operator/(const float &z) const { return ComplexFloat(*this) /= z; }

inline const ComplexFloat ComplexFloat::operator-() const { return ComplexFloat(-re, -im); }

inline const ComplexFloat ComplexFloat::operator~() const { return ComplexFloat(re, -im); }

inline ComplexFloat ComplexFloat::operator=(const float &y) {
	re = y;
	im = 0.0;
	return *this;
}

inline bool ComplexFloat::operator==(const ComplexFloat &y) const { return ((re == y.re) && (im == y.im)); }

inline bool ComplexFloat::operator!=(const ComplexFloat &y) const { return ((re != y.re) || (im != y.im)); }

inline float abs2(const ComplexFloat &x) {
	float ret = x.re * x.re + x.im * x.im;
	return ret;
}

inline float abs(const ComplexFloat &x) { return Math::Sqrt(abs2(x)); }

inline ComplexFloat conj(const ComplexFloat &x) { return ~x; }

inline ComplexFloat conj_mpy(const ComplexFloat &x, const ComplexFloat &y) {
	float re = x.re * y.re + x.im * y.im;
	float im = y.re * x.im - x.re * y.im;

	return ComplexFloat(re, im);
}

#endif // Math_Complex_H_
