#include "Lte/Core/SoftTurboDecoder.h"

#include <Math/ComplexMath.h>
#include <System/DebugInfo.h>

namespace Lte {

SoftTurboDecoder::SoftTurboDecoder()
	: debug(System::DebugInfo::Locate())
{

}

SoftTurboDecoder::~SoftTurboDecoder() {

}

void SoftTurboDecoder::Configure(const CtcInterleaver &interleaver) {

	frameLen = interleaver.Ki;
	len = frameLen + TurboCodeTail - 1;
	payloadSize = frameLen - TurboCRC_Size;
	alfaLen = (frameLen + TurboCodeTail) << TurboCodeRate;  //-V112

	f1 = interleaver.f1;
	f2 = interleaver.f2;

	Lu.resize(len);
	Lc1.resize(len);
	Lc2.resize(len);

	Lambda1.resize(len);
	Lambda2.resize(len);

	g11.resize(len);
	g10.resize(len);

	alfa.resize(alfaLen);
	beta.resize(alfaLen);

}

bool SoftTurboDecoder::Do(const float *dk0, const float *dk1, const float *dk2, CrcType crcType, uint32_t* res) {
	bool crc = false;

	Init(dk0, dk1, dk2);

	rsc.pLu  = Lu.data();
	rsc.pG11 = g11.data();
	rsc.pG10 = g10.data();
	rsc.pA   = alfa.data();
	rsc.pB   = beta.data();
	rsc.FrameLen = frameLen;

	for (size_t i = 0; !crc && (i < TurboIterCount); ++i) {

		memcpy(&Lu[frameLen], Lu1Tail.data(), sizeof(Lu1Tail));
		rsc.pLc = Lc1.data();
		rsc.pLambda = Lambda1.data();

		RscDecoder(rsc);

		Interleaver(Lambda1.data(), Lu.data(), frameLen, f1, f2);
		memcpy(&Lu[frameLen], Lu2Tail.data(), sizeof(Lu2Tail));

		rsc.pLc = Lc2.data();
		rsc.pLambda = Lambda2.data();

		RscDecoder(rsc);
		Deinterleaver(Lambda2.data(), Lu.data(), frameLen, f1, f2);

		uint8_t* bits = reinterpret_cast<uint8_t *>(res);
		HardDecisions(Lu, frameLen, bits);

		uint32_t rv = ExtractRvCrc(res,payloadSize, TurboCRC_Size);

		crc = CheckCrc(bits,crcType, rv,frameLen);
	}

	return crc;
}

void SoftTurboDecoder::Init(const float *dk0, const float *dk1, const float *dk2) {

	std::memcpy(Lu.data(), dk0, sizeof(float)* frameLen);
	std::memcpy(Lc1.data(), dk1, sizeof(float)* frameLen);
	std::memcpy(Lc2.data(), dk2, sizeof(float)* frameLen);

	Lu1Tail[0] = dk0[frameLen];
	Lu1Tail[1] = dk2[frameLen];
	Lu1Tail[2] = dk1[frameLen + 1];

	Lc1[frameLen]	  = dk1[frameLen];
	Lc1[frameLen + 1] = dk0[frameLen + 1];
	Lc1[frameLen + 2] = dk2[frameLen + 1];

	Lc2[frameLen]     = dk1[frameLen + 2];
	Lc2[frameLen + 1] = dk0[frameLen + 3];
	Lc2[frameLen + 2] = dk2[frameLen + 3];

	Lu2Tail[0] = dk0[frameLen + 2];   //-V525
	Lu2Tail[1] = dk2[frameLen + 2];
	Lu2Tail[2] = dk1[frameLen + 3];
}

void SoftTurboDecoder::RscDecoder(RSCDecoder& pRSC) {

	Gamma(pRSC.pLu, pRSC.pLc, pRSC.FrameLen, pRSC.pG11, pRSC.pG10);
	Alfa(pRSC.pG10, pRSC.pG11, pRSC.FrameLen, pRSC.pA);
	Beta(pRSC.pG10, pRSC.pG11, pRSC.FrameLen, pRSC.pB);
	Extrinsic(pRSC.pA, pRSC.pB, pRSC.pG10, pRSC.pG11, pRSC.FrameLen, pRSC.pLambda);
}

void SoftTurboDecoder::Alfa(const float *pG10, const float *pG11, size_t vFrameLen, float *pA) {
	float m_t, m_b, g11, g10;
	pA[0] = 0;
	pA[1] = pA[2] = pA[3] = pA[4] = pA[5] = pA[6] = pA[7] = -max_llr;

	int32_t idx1,idx2,idx3;

	for (size_t i = 1; i < vFrameLen; ++i) {
		idx1 = i-1;
		idx2 = idx1 << TurboCodeRate;
		idx3 = i << TurboCodeRate;
		g11 = pG11[idx1];
		g10 = pG10[idx1];
		m_t = pA[idx2 + 0] - g11;
		m_b = pA[idx2 + 1] + g11;
		pA[idx3 + 0] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 2] + g10;
		m_b = pA[idx2 + 3] - g10;
		pA[idx3 + 1] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 4] - g10;
		m_b = pA[idx2 + 5] + g10;
		pA[idx3 + 2] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 6] + g11;
		m_b = pA[idx2 + 7] - g11;
		pA[idx3+ 3] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 0] + g11;
		m_b = pA[idx2 + 1] - g11;
		pA[idx3 + 4] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 2] - g10;
		m_b = pA[idx2 + 3] + g10;
		pA[idx3 + 5] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 4] + g10;
		m_b = pA[idx2 + 5] - g10;
		pA[idx3 + 6] = MaxStar(m_t, m_b);
		m_t = pA[idx2 + 6] - g11;
		m_b = pA[idx2 + 7] + g11;
		pA[idx3+ 7] = MaxStar(m_t, m_b);
	}

	// ��������������
	//============================
	idx1 = vFrameLen-1;
	idx2 = idx1 << TurboCodeRate;
	idx3 = vFrameLen << TurboCodeRate;

	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pA[idx2 + 0] - g11;
	m_b = pA[idx2 + 1] + g11;
	pA[idx3 + 0] = MaxStar(m_t, m_b);
	m_t = pA[idx2 + 2] + g10;
	m_b = pA[idx2 + 3] - g10;
	pA[idx3 + 1] = MaxStar(m_t, m_b);
	m_t = pA[idx2 + 4] - g10;
	m_b = pA[idx2 + 5] + g10;
	pA[idx3 + 2] = MaxStar(m_t, m_b);
	m_t = pA[idx2 + 6] + g11;
	m_b = pA[idx2 + 7] - g11;
	pA[idx3 + 3] = MaxStar(m_t, m_b);
	pA[idx3 + 4] = pA[idx3 + 5] = pA[idx3 + 6] = pA[idx3 + 7] = -max_llr;
	//============================

	idx1 = vFrameLen + 1 - 1;
	idx2 = idx1 << TurboCodeRate;
	idx3 = vFrameLen << TurboCodeRate;

	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pA[idx2 + 0] - g11;
	m_b = pA[idx2 + 1] + g11;
	pA[idx3 + 0] = MaxStar(m_t, m_b);
	m_t = pA[idx2 + 2] + g10;
	m_b = pA[idx2 + 3] - g10;
	pA[idx3 + 1] = MaxStar(m_t, m_b);
	pA[idx3 + 2] = pA[idx3 + 3] = pA[idx3 + 4] = pA[idx3 + 5] = pA[idx3 + 6] = pA[idx3 + 7] = -max_llr;
	//============================
	idx1 = vFrameLen + 2 - 1;
	idx2 = idx1 << TurboCodeRate;
	idx3 = vFrameLen << TurboCodeRate;
	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pA[idx2 + 0] - g11;
	m_b = pA[idx2 + 1] + g11;
	pA[idx3 + 0] = MaxStar(m_t, m_b);
	pA[idx3 + 1] = pA[idx3 + 2] = pA[idx3 + 3] = pA[idx3 + 4] = pA[idx3 + 5] = pA[idx3 + 6] = pA[idx3 + 7] =
		-max_llr;
}

void SoftTurboDecoder::Beta(const float *pG10, const float *pG11, size_t vFrameLen, float *pB) {

	for (size_t i = 0, j = (vFrameLen + 4) * 8 - 1; i < 8; ++i, --j) //-V112
		pB[j] = -max_llr;
	pB[(vFrameLen + 3) * 8] = 0;

	int32_t idx1,idx2,idx3;

	float m_t, m_b, g11, g10;
	for (size_t i = vFrameLen + 2; i > 2; --i) {

		idx1 = i;
		idx2 = (idx1+1) << TurboCodeRate;
		idx3 = idx1 << TurboCodeRate;

		g11 = pG11[idx1];
		g10 = pG10[idx1];
		m_t = pB[idx2 + 0] - g11;
		m_b = pB[idx2 + 4] + g11;
		pB[idx3 + 0] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 0] + g11;
		m_b = pB[idx2 + 4] - g11;
		pB[idx3 + 1] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 1] + g10;
		m_b = pB[idx2 + 5] - g10;
		pB[idx3 + 2] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 1] - g10;
		m_b = pB[idx2 + 5] + g10;
		pB[idx3 + 3] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 2] - g10;
		m_b = pB[idx2 + 6] + g10;
		pB[idx3 + 4] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 2] + g10;
		m_b = pB[idx2 + 6] - g10;
		pB[idx3 + 5] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 3] + g11;
		m_b = pB[idx2 + 7] - g11;
		pB[idx3 + 6] = MaxStar(m_t, m_b);
		m_t = pB[idx2 + 3] - g11;
		m_b = pB[idx2+ 7] + g11;
		pB[idx3 + 7] = MaxStar(m_t, m_b);
	}

	//==================
	idx1 = 2;
	idx2 = (idx1+1) << TurboCodeRate;
	idx3 = idx1 << TurboCodeRate;

	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pB[idx2 + 0] - g11;
	m_b = pB[idx2 + 4] + g11;
	pB[idx3 + 0] = MaxStar(m_t, m_b);
	m_t = pB[idx2 + 1] + g10;
	m_b = pB[idx2 + 5] - g10;
	pB[idx3 + 2] = MaxStar(m_t, m_b);
	m_t = pB[idx2 + 2] - g10;
	m_b = pB[idx2 + 6] + g10;
	pB[idx3+ 4] = MaxStar(m_t, m_b);
	m_t = pB[idx2 + 3] + g11;
	m_b = pB[idx2 + 7] - g11;
	pB[idx3 + 6] = MaxStar(m_t, m_b);
	pB[idx3 + 1] = pB[idx3 + 3] = pB[idx3 + 5] = pB[idx3 + 7] = -max_llr;
	//==================

	idx1 = 1;
	idx2 = (idx1+1) << TurboCodeRate;
	idx3 = idx1 << TurboCodeRate;
	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pB[idx2 + 0] - g11;
	m_b = pB[idx2 + 4] + g11;
	pB[idx3 + 0] = MaxStar(m_t, m_b);
	m_t = pB[idx2 + 2] - g10;
	m_b = pB[idx2 + 6] + g10;
	pB[idx3+ 4] = MaxStar(m_t, m_b);
	pB[idx3+ 1] = pB[idx3 + 2] = pB[idx3 + 3] = pB[idx3 + 5] = pB[idx3 + 6] = pB[idx3 + 7] = -max_llr;
	//==================
	idx1 = 0;
	idx2 = (idx1+1) << TurboCodeRate;
	idx3 = idx1 << TurboCodeRate;
	g11 = pG11[idx1];
	g10 = pG10[idx1];
	m_t = pB[idx2 + 0] - g11;
	m_b = pB[idx2 + 4] + g11;
	pB[idx3 + 0] = MaxStar(m_t, m_b);
	pB[idx3 + 1] = pB[idx3 + 2] = pB[idx3 + 3] = pB[idx3 + 4] = pB[idx3 + 5] = pB[idx3 + 6] = pB[idx3 + 7] =
		-max_llr;
}

void SoftTurboDecoder::Gamma(const float *pLu, const float *pLc, size_t vFrameLen, float *pG11, float *pG10) {
	for (size_t i = 0; i < (vFrameLen + 3); ++i) {
		pG11[i] = Math::Div(-(pLu[i] + pLc[i]), 2);
		pG10[i] = Math::Div(-(pLu[i] - pLc[i]),2);
	}
}

void SoftTurboDecoder::Extrinsic(const float *pA, const float *pB, const float *pG10, const float *pG11,
								 size_t vFrameLen, float *pExtrinsic) {
	float t_d, t_e, enumerator, denominator;

	uint32_t idx1 = 0;
	uint32_t idx2 = 0;
	uint32_t idx3 = 0;

	for (size_t i = 1; i <= vFrameLen; ++i) {
		enumerator = -max_llr;
		denominator = -max_llr;
		idx1 = i-1;
		idx2 = idx1 << TurboCodeRate;
		idx3 = (idx1+1) << TurboCodeRate;

		float g11 = pG11[idx1];
		float g10 = pG10[idx1];
		t_d = pA[idx2 + 0] + pB[idx3 + 0] - g11;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 0] + pB[idx3 + 4] + g11;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 1] + pB[idx3 + 4] - g11;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 1] + pB[idx3 + 0] + g11;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 2] + pB[idx3 + 5] - g10;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 2] + pB[idx3 + 1] + g10;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 3] + pB[idx3 + 1] - g10;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 3] + pB[idx3 + 5] + g10;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 4] + pB[idx3 + 2] - g10;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 4] + pB[idx3 + 6] + g10;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 5] + pB[idx3 + 6] - g10;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 5] + pB[idx3 + 2] + g10;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 6] + pB[idx3 + 7] - g11;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 6] + pB[idx3 + 3] + g11;
		enumerator = MaxStar(enumerator, t_e);
		t_d = pA[idx2 + 7] + pB[idx3 + 3] - g11;
		denominator = MaxStar(denominator, t_d);
		t_e = pA[idx2 + 7] + pB[idx3 + 7] + g11;
		enumerator = MaxStar(enumerator, t_e);
		pExtrinsic[idx1] = denominator - enumerator;
	}
}


void SoftTurboDecoder::Interleaver(const float *pSrc, float *pDst, size_t vLen, size_t f1, size_t f2) {
	for (size_t i = 0, f = f1; i < vLen; ++i, f += f2) {
		if (f >= vLen)	f -= vLen;
		size_t p = (i * f) % vLen;
		pDst[i] = pSrc[p];
	}
}

void SoftTurboDecoder::Deinterleaver(const float *pSrc, float *pDst, size_t vLen, size_t f1, size_t f2) {
	for (size_t i = 0, f = f1; i < vLen; ++i, f += f2) {
		if (f >= vLen)	f -= vLen;
		size_t p = (i * f) % vLen;
		pDst[p] = pSrc[i];
	}
}

} // namespace Lte
