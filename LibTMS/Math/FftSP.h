/*
 * FftSP.h
 *
 *  Created on: 02.07.2019
 *      Author: Rachicky
 */

#ifndef MATH_FFTSP_H_
#define MATH_FFTSP_H_

#include "Complex.h"
#include <vector>

namespace Math {

class FftSP {
public:
	FftSP();
	FftSP(uint32_t length );
	virtual ~FftSP();

	void Init(uint32_t length);
	void DoIt(ComplexFloat * __restrict x, ComplexFloat * __restrict y);
	void Undo(ComplexFloat * __restrict x, ComplexFloat * __restrict y);
	void Shift(ComplexFloat * __restrict x);
	void Shift(ComplexFloat * __restrict x, ComplexFloat * __restrict y);

protected:
	uint32_t length;
	std::vector<float> twiddleFft;
	std::vector<float> twiddleIfft;

	uint32_t rad;

};

} // namespace Math

#endif // MATH_FFTSP_H_
