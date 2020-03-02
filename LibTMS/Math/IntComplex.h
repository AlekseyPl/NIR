/*
 * Complex.h
 *
 *  Created on: 30.06.2011
 *      Author: VTV
 */

#ifndef IntMath_Complex_C6416_H_
#define IntMath_Complex_C6416_H_

#include <stdint.h>
#include <ostream>
#include <math.h>
#include <complex>

class Complex16 {
public:
	static const int fract = 15;
	static const int wl = 16;
	static const int shift = fract;
	int16_t re;
	int16_t im;

public:
	Complex16(const int16_t re = 0, const int16_t im = 0);
	Complex16(const int16_t re, const int16_t im, const int scale);
	Complex16(const Complex16 &z) = default;

	Complex16 &operator+=(const Complex16 z);
	Complex16 &operator-=(const Complex16 z);
	Complex16 &operator*=(const Complex16 z);
	Complex16 &operator/=(const Complex16 y);
	Complex16 &operator&=(const Complex16 z);
	Complex16 &operator<<=(const int16_t n);
	Complex16 &operator>>=(const int16_t n);
	const Complex16 operator+(const Complex16 z) const;
	const Complex16 operator-(const Complex16 z) const;
	const Complex16 operator*(const Complex16 z) const;
	const Complex16 operator*(const int16_t y) const;
	const Complex16 operator/(const Complex16 z) const;
	const Complex16 operator/(const int16_t y) const;
	const Complex16 operator&(const Complex16 z) const;
	const Complex16 operator<<(const int16_t n) const;
	const Complex16 operator>>(const int16_t n) const;
	const Complex16 operator-() const;
	const Complex16 operator~() const;
	
	friend std::ostream &operator<<(std::ostream &str, const Complex16 &obj);


	Complex16 operator=(const int16_t &y);
	bool operator==(const Complex16 &y) const;
	bool operator!=(const Complex16 &y) const;

	inline uint8_t width() const { return sizeof(int16_t) * 8; }

	inline friend uint32_t abs(const Complex16 &x);
	inline friend uint32_t abs2(const Complex16 &x);
	inline friend Complex16 conj(const Complex16 &x);
};


struct Complex32 {
public:
	int32_t re;
	int32_t im;
};

//------------------------------------------------------------

inline Complex16::Complex16(const int16_t re, const int16_t im) : re(re), im(im) {}

/**
 *
 * @param re вещественная часть (re = Re(z)*scale)
 * @param im мнимая часть (im = Im(z)*scale)
 * @param scale десятичная точка (scale = 10^N)
 */
inline Complex16::Complex16(const int16_t re, const int16_t im, const int scale) {
	this->re = (int32_t(re) << shift) / scale;
	this->im = (int32_t(im) << shift) / scale;
}

inline Complex16 &Complex16::operator+=(const Complex16 z) {
	re += z.re;
	im += z.im;
	return *this;
}

inline Complex16 &Complex16::operator-=(const Complex16 z) {
	re -= z.re;
	im -= z.im;
	return *this;
}

inline Complex16 &Complex16::operator*=(const Complex16 z) {
	std::complex<int16_t> val(re,im);
	std::complex<int16_t> yVal(z.re,z.im);

	std::complex<int16_t> ret = val*yVal;
	re = ret.real();
	im = ret.imag();
	return *this;
}

inline Complex16 &Complex16::operator/=(const Complex16 y) {

	std::complex<int16_t> val(re,im);
	std::complex<int16_t> yVal(y.re,y.im);

	std::complex<int16_t> ret = val/yVal;
	re = ret.real();
	im = ret.imag();
	return *this;
}


inline Complex16 &Complex16::operator<<=(const int16_t n) {
	re <<= n;
	im <<= n;
	return *this;
}

inline Complex16 &Complex16::operator>>=(const int16_t n) {
	re >>= n;
	im >>= n;
	return *this;
}

inline const Complex16 Complex16::operator+(const Complex16 z) const { return Complex16(*this) += z; }

inline const Complex16 Complex16::operator-(const Complex16 z) const { return Complex16(*this) -= z; }

inline const Complex16 Complex16::operator*(const Complex16 z) const { return Complex16(*this) *= z; }

inline const Complex16 Complex16::operator/(const Complex16 z) const { return Complex16(*this) /= z; }

inline const Complex16 Complex16::operator/(const int16_t y) const { return Complex16(re / y, im / y); }

inline const Complex16 Complex16::operator*(const int16_t y) const { return Complex16(re * y, im * y); }

inline const Complex16 Complex16::operator&(const Complex16 z) const { return Complex16(*this) &= z; }

inline const Complex16 Complex16::operator<<(const int16_t n) const { return Complex16(*this) <<= n; }

inline const Complex16 Complex16::operator>>(const int16_t n) const { return Complex16(*this) >>= n; }

inline const Complex16 Complex16::operator-() const { return Complex16(-re, -im); }

inline const Complex16 Complex16::operator~() const { return Complex16(re, -im); }

inline Complex16 Complex16::operator=(const int16_t &y) {
	re = y;
	im = 0;
	return *this;
}

inline bool Complex16::operator==(const Complex16 &y) const { return ((re == y.re) && (im == y.im)); }

inline bool Complex16::operator!=(const Complex16 &y) const { return ((re != y.re) || (im != y.im)); }

inline uint32_t abs2(const Complex16 &x) {
	std::complex<int16_t> val(x.re,x.im);
	uint32_t ret = std::abs(val);
	ret *= ret;
	return ret;
}

inline uint32_t abs(const Complex16 &x) {
	std::complex<int16_t> val(x.re,x.im);

	return std::abs(val);
}

inline Complex16 conj(const Complex16 &x) { return Complex16(x.re, -x.im); }

inline std::ostream &operator<<(std::ostream &str, const Complex16 &obj) { return str << "(" << obj.re << ", " << obj.im << ")"; }


#endif // IntMath_Complex_H_
