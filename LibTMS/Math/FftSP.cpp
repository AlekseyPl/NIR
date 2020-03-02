/*
 * FftSP.cpp
 *
 *  Created on: 02.07.2019
 *      Author: Rachicky
 */

#include "FftSP.h"
#include "DSPF_sp_fftSPxSP_cn.h"
#include "DSPF_sp_ifftSPxSP_cn.h"

#include <cstring>

namespace {

void GenIfftTw(float *w, int n);
void GenFftTw(float *w, int n);

uint32_t GenRad(uint32_t length);


} // namespace

namespace Math {

FftSP::FftSP(uint32_t length)
    : length(length), twiddleFft(length * 2), twiddleIfft(length * 2),
      rad(GenRad(length))
{
	GenFftTw(twiddleFft.data(), length);
	GenIfftTw(twiddleIfft.data(), length);
}

FftSP::FftSP() : length(0), rad(2)
{

}

FftSP::~FftSP() {

}

void FftSP::Init(uint32_t length) {
	this->length = length;
	twiddleIfft.resize(length * 2);
	twiddleFft.resize(length * 2);
	GenFftTw(twiddleFft.data(), length);
	GenIfftTw(twiddleIfft.data(), length);

	rad = GenRad(length);

}

void FftSP::DoIt(ComplexFloat *__restrict x, ComplexFloat *__restrict y) {
	DSPF_sp_fftSPxSP_cn(length, reinterpret_cast<float *>(x), twiddleFft.data(), reinterpret_cast<float *>(y), nullptr,
	                    rad, 0, length);
}

void FftSP::Undo(ComplexFloat *__restrict x, ComplexFloat *__restrict y) {
	DSPF_sp_ifftSPxSP_cn(length, reinterpret_cast<float *>(x), twiddleIfft.data(), reinterpret_cast<float *>(y), nullptr,
	                     rad, 0, length);
}


void FftSP::Shift(ComplexFloat *__restrict x, ComplexFloat *__restrict y) {
	uint32_t lengthDiv2 = length >> 1;
	std::memcpy(&y[lengthDiv2], x, lengthDiv2 * sizeof(ComplexFloat));
	std::memcpy(y, &x[lengthDiv2], lengthDiv2 * sizeof(ComplexFloat));
}

} // namespace Math

namespace {

void GenIfftTw(float *w, int n) {
	int i, j, k;
	const double PI = 3.141592654;

	for (j = 1, k = 0; j <= n >> 2; j = j << 2) {
		for (i = 0; i < (n >> 2); i += j) {
			w[k] = (float)-sin(2 * PI * i / n);
			w[k + 1] = (float)cos(2 * PI * i / n);
			w[k + 2] = (float)-sin(4 * PI * i / n);
			w[k + 3] = (float)cos(4 * PI * i / n);
			w[k + 4] = (float)-sin(6 * PI * i / n);
			w[k + 5] = (float)cos(6 * PI * i / n);
			k += 6;
		}
	}
}

void GenFftTw(float *w, int n) {
	int i, j, k;
	const double PI = 3.141592654;

	for (j = 1, k = 0; j <= n >> 2; j = j << 2) {
		for (i = 0; i < (n >> 2); i += j) {
			w[k] = (float)sin(2 * PI * i / n);
			w[k + 1] = (float)cos(2 * PI * i / n);
			w[k + 2] = (float)sin(4 * PI * i / n);
			w[k + 3] = (float)cos(4 * PI * i / n);
			w[k + 4] = (float)sin(6 * PI * i / n);
			w[k + 5] = (float)cos(6 * PI * i / n);
			k += 6;
		}
	}
}

uint32_t GenRad(uint32_t length) {
	uint32_t j = 0;
	for (uint32_t i = 0; i <= 31; i++)
		if ((length & (static_cast<uint32_t>(1) << i)) == 0)
			j++;
		else
			break;

	if (j % 2 == 0)
		return 4;
	else
		return 2;
}
} // namespace
