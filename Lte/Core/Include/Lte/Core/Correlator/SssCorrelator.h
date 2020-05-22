#ifndef SSS_CORRELATOR_H_
#define SSS_CORRELATOR_H_

#include "Lte/Core/Support/LteDemConst.h"
#include "Lte/Core/Support//LteSupport.h"
#include <Math/FftSP.h>
#include "SyncCode.h"
#include <array>
#include <iostream>
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

    struct searchRes
    {
        float32 val    = 0;
        int place = 0;
    };

	void			Configure(SearchDepth s);
	SearchResult	Do(const ComplexFloat* data, uint32_t nid2, uint32_t pssPos);
    uint32_t GetPssOffsetFrame(CyclicPrefix cp, Duplex dx);
private:
	static const uint32_t DecimFactor = 16;
	static const uint32_t CorrCount = LTEFrameLength / DecimFactor;

    static const uint32_t VariantsCount = 6;
    static const uint32_t SssPartsCount = 2;

	struct 	SearchParams {
		int32_t			shiftPssToSss;
		int32_t			shiftToFramePos;
		std::string		string;
		Duplex			duplex;
		CyclicPrefix	cp;
	};

	struct SssCorrRes {
		uint32_t M0;
		uint32_t M1;
		uint32_t subframeNum;
		float	corrRes;
        inline bool operator< (const SssCorrRes& rhs) const {
            return corrRes < rhs.corrRes;
		}

	};


	const uint32_t				startPos0;
	const uint32_t				startPos1;
	const uint32_t				endPos0;
	const uint32_t				endPos1;
    searchRes                   est_m0;
    searchRes                   est_m1;
    searchRes                   est_m;

	std::vector<SearchParams>	searchParams;
	Math::FftSP					fft128;


   std::vector<ComplexFloat>	corrRes;
   std::vector<ComplexFloat> fftCorrRes;
    std::vector<float>    absCorrRes;

    int32_t nid2;
    std::vector <SssCorrRes> CorrResults;


	std::vector<ComplexFloat>	sssSignal;
	std::vector<ComplexFloat>	sssSpectrum;
    std::array <std::vector<ComplexFloat>, SssPartsCount>	sssParts;

    std::vector<ComplexFloat>   tempEven;
    std::vector<ComplexFloat>   tempOdd;

	SecondarySyncCode			sss;
	M0M1Converter				m0m1;
    std::array<SecondarySyncCode::SpecCode, VariantsCount>	sssCode;

	uint32_t					m0;
	uint32_t					m1;

	inline uint32_t		CalcM0(uint32_t pos);
	inline uint32_t		CalcM1(uint32_t pos);

	System::DebugInfo&	debug;
	SssCorrRes	Correlate(uint32_t nid2);
	void		ExtractSignalSss(const ComplexFloat* data, const SearchParams& params, uint32_t pssPos);
	searchRes FindSeq(const std::vector<ComplexFloat> &sssSpectrumPart, uint32_t evenOrOdd);
};


inline uint32_t SssCorrelator::CalcM0(uint32_t pos)
{
    //return (LTESyncCodeHalfLen - pos);
    return (LTESyncCodeHalfLen - pos);
}

inline uint32_t SssCorrelator::CalcM1(uint32_t pos)
{
    return (LTESyncCodeHalfLen - pos + 1)%LTESyncCodeHalfLen; // look at 36211. Table 6.11.2.1-1
}

}

#endif // SSS_CORRELATOR