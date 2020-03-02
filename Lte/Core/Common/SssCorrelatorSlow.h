#ifndef SSS_CORRELATOR_SLOW_H_
#define SSS_CORRELATOR_SLOW_H_

#include "Common/LteDemConst.h"
#include "Common/LteSupport.h"
#include <Math/FftSP.h>
#include "Common/SyncCode.h"
#include <array>

namespace System {
	class DebugInfo;
}

namespace Lte {

class SssCorrelator {
public:

	SssCorrelator();
	~SssCorrelator();

	struct SearchDepth {
		uint8_t		fdd_ncp : 1;	// Frequency Division Duplex and Normal Cyclic Prefix
		uint8_t		fdd_ecp : 1;	// Frequency Division Duplex and Extended Cyclic Prefix
		uint8_t		tdd_ncp : 1;	// Time Division Duplex and Normal Cyclic Prefix
		uint8_t		tdd_ecp : 1;	// Time Division Duplex and Extended Cyclic Prefix
		uint32_t	dummy:28;
	};

	struct SearchResult {
		SearchResult(uint32_t nid1_, uint32_t shiftToFrame, uint32_t sfNum, CyclicPrefix cp_, Duplex dx_):
			nid1(nid1_),shiftToFrame(shiftToFrame), subframeNum(sfNum), cp(cp_),dx(dx_) { }
		uint32_t		nid1;
		uint32_t		shiftToFrame;
		uint32_t		subframeNum;
		CyclicPrefix	cp;
		Duplex			dx;
	};

	void			Configure(SearchDepth s);
	SearchResult Do(const ComplexFloat* data, uint32_t nid2, uint32_t pssPos);
	uint32_t GetPssOffsetFrame(CyclicPrefix cp, Duplex dx);

private:
	static const uint32_t SssFftCorrLen = 32;
	static const uint32_t DecimFactor = 16;
	static const uint32_t CorrCount = (LTEFrameLength / 2)/DecimFactor;

	struct 	SearchParams {
		int32_t			shiftPssToSss;
		int32_t			shiftToFramePos;
		std::string		string;
		Duplex			duplex;
		CyclicPrefix	cp;
	};

	struct SssCorrRes {
		SssCorrRes( float corrMax, uint32_t sfNum, uint32_t nid1) :
			  corrMax(corrMax), sfNum(sfNum), nid1(nid1) {}
		float corrMax;

		uint32_t sfNum;
		uint32_t nid1;
	};

	float sssRxAmp;
	std::vector<SearchParams>	searchParams;
	std::vector<ComplexFloat>	sssSignal;

	SecondarySyncCode			sss;
	std::vector<std::vector<float>>		sssCorrRes;

	System::DebugInfo&	debug;

	SssCorrRes ProcessParams(const ComplexFloat *data, const SearchParams &params, uint32_t nid2);
	void	Correlate( const ComplexFloat* data, uint32_t nid2, uint32_t nid1);
	void    ExtractSymbolSss(const ComplexFloat* data, const SearchParams& params, uint32_t pssPos);

	float CrossCorr(const ComplexFloat* code, const ComplexFloat* signal,  uint32_t codeLen);
};


}

#endif // SSS_CORRELATOR
