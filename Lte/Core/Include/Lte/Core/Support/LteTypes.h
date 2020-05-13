/*
 * lte_types.h
 *
 *  Created on: 24.06.2014
 *      Author: dblagov
 */

#ifndef LTE_TYPES_H_
#define LTE_TYPES_H_

#include "LteConst.h"
#include <vector>
#include <map>
#include <Math/IntComplex.h>
#include <Math/Complex.h>

namespace Lte {


// Radio Network Temporary Identifier
using Rnti = uint16_t;

using UBitVector = uint8_t;
using PBitVector = uint8_t;
using Index = std::vector<uint32_t>;
using Index2D = std::vector<Index>;
using SoftDecisions = std::vector<SoftDecision>;

using Symbols = std::vector<Complex16>;
using FSymbols = std::vector<ComplexFloat>;
using Accumulator = std::vector<Complex32>;


enum CyclicPrefix {
	lteCP_Short,
	lteCP_Long

};

enum TxAntPorts {
	lteTxAntPorts1,  /*! 1 antenna port  */
	lteTxAntPorts2,  /*! 2 antenna ports */
	lteTxAntPorts4   /*! 4 antenna ports */
};

enum TxAntPort {
	lteTxAntPort1,  /*! TX antenna port 1 */
	lteTxAntPort2,  /*! TX antenna port 2 */
	lteTxAntPort3,  /*! TX antenna port 3 */
	lteTxAntPort4   /*! TX antenna port 4 */
};

enum BandWidth {
	lteBW_Sync,       /*! пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ           */
	lteBW_1_4_MHz,    /*! 1.4 MHz, 128 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ */
	lteBW_3_MHz,      /*! 3   MHz, 256 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ   */
	lteBW_5_MHz,      /*! 5   MHz, 512 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ   */
	lteBW_10_MHz,     /*! 10  MHz, 1024 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ */
	lteBW_15_MHz,     /*! 15  MHz, 1536 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ */
	lteBW_20_MHz      /*! 20  MHz, 2048 пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ */
};

enum Modulation {
	lteQPSK  = 0,   /*! QPSK   */
	lteQAM16 = 1,   /*! QAM-16 */
	lteQAM64 = 2    /*! QAM-64 */
};

enum PhichDuration {
	ltePHICHDuration_Normal,   /*! PHICH duration normal   */
	ltePHICHDuration_Extended  /*! PHICH duration extended */
};
/**
 *  @enum  PhichResource
 *  @brief PHICH resource.
 */
enum PhichResource {
	ltePHICHResource_1_6, /*! PHICH resource one sixth */
	ltePHICHResource_1_2, /*! PHICH resource half      */
	ltePHICHResource_1,   /*! PHICH resource one       */
	ltePHICHResource_2    /*! PHICH resource two       */
};

/**
 *  @enum  TxMode
 *  @brief Transmission mode.
 */
enum TxMode {
	lteTM1 = 0,   /*! Transmission mode 1 */
	lteTM2 = 1,   /*! Transmission mode 2 */
	lteTM3 = 2,   /*! Transmission mode 3 */
	lteTM4 = 3,   /*! Transmission mode 4 */
	lteTM5 = 4,   /*! Transmission mode 5 */
	lteTM6 = 5,   /*! Transmission mode 6 */
	lteTM7 = 6    /*! Transmission mode 7 */
};

/**
 *  @enum  Rv
 *  @brief Redundancy version.
 */
enum Rv {
	lteRV0 = 0,   /*! Redundancy version 0 */
	lteRV1 = 1,   /*! Redundancy version 1 */
	lteRV2 = 2,   /*! Redundancy version 2 */
	lteRV3 = 3    /*! Redundancy version 3 */
};

enum PrbType {
	prbPdsch,          /*! PRB пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PDSCH                 */
	prbCommon,         /*! PRB пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PBCH/PSS/SSS          */
	prbPdschLeftHalf,  /*! пїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PRB пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PDSCH  */
	prbPdschRightHalf  /*! пїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PRB пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ PDSCH */
};

enum Cfi {
	lteCFI1, /*! CFI 1 */
	lteCFI2, /*! CFI 2 */
	lteCFI3, /*! CFI 3 */
	lteCFI4 /*! CFI 4 (Reserved) */
};

enum UeTxAntPort {
	lteUeTxAntPort0,  /*! UE port 0  */
	lteUeTxAntPort1  /*! UE port 1  */
};

enum CrcResult {
	lteCRCFail,
	lteCRCOk,
	lteCRCNonExists
};

enum Duplex {
	lteFDD = 0,   /*! Freq duplex */
	lteTDD = 1  /*! Time duplex */
};

/**
 *  @enum  Gap
 *  @brief Distributed virtual resource block gap.
 */
enum Gap {
	gap1, /*! 1-st gap */
	gap2  /*! 2-nd gap */
};

//struct CodeSegment {
//	Common::Buffer< uint8_t > Bits;
//	uint32_t    FillBitsCount;
//};
/**
 *  @struct CodeBlockInfo
 *  @brief  пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ.
 */
struct CodeBlockInfo {
    uint32_t B;         // Code block size
    uint32_t Bs;        // Code block size with additional CRC
    uint32_t L;         // Additional CRC size
    uint32_t C;         // Number of coded blocks
    uint32_t C_plus;    // Number of coded blocks with lenght K+
    uint32_t C_minus;   // Number of coded blocks with lenght K-
    uint32_t K_plus;    // First segmentation size K+
    uint32_t K_minus;   // Second segmentation size K-
    uint32_t F;         // Number of filler bits
};

struct CtcRmConsts {
    bool       Uplink;          // reverse channel flag
    uint32_t     Nsoft;           // Total number of soft channel bits (TS 36.306, table 4.1-1)
    TxMode     TM;              // Transmission mode
    uint32_t     MaxDlHarqProc;   // Maximum number of DL HARQ processes
    Rv         rv;              // Redundancy version
    uint32_t     G;               // пїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅ
    uint32_t     Nl;              // пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ transmission layer
    Modulation Mod;             // пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ

    inline CtcRmConsts( uint32_t size, Rv rv_, Modulation mod ):
        Uplink( false ), Nsoft( SoftChannelBits[ 0 ] ), TM( lteTM1 ), MaxDlHarqProc( M_DL_HARQ_Limit ),
        rv( rv_ ), G( size ),   Mod( mod )
    {
            Nl = ( Mod == lteQPSK  ) ? 1 : 2;
    }
};

struct SbInfo
{
  uint32_t D;     // Длина входной последовательности.
  uint32_t Kp;    // Длина выходной последовательности перемежителя
  uint32_t R_tc;  // Количество строк матрицы перемежения
  uint32_t Nd;    // Количество dummy бит
};

struct CtcRmParams
{
  SbInfo SbiInfo;
  uint32_t Ncb;
  uint32_t E;
  uint32_t k0;
};

class RefSymbPos {
public:
	RefSymbPos( const RefSymbPos& rs );
	RefSymbPos( uint32_t rs0_0, uint32_t rs0_1, uint32_t rs1_0, uint32_t rs1_1, uint32_t rs2_0, uint32_t rs3_0 );
	bool IsRefSymbolPresent( uint32_t antPort, uint32_t symb );

private:
    uint32_t m_RS0_0;
    uint32_t m_RS0_1;
    uint32_t m_RS1_0;
    uint32_t m_RS1_1;
    uint32_t m_RS2_0;
    uint32_t m_RS3_0;
};

class Time {
public:
	Time( CyclicPrefix cp = lteCP_Short );
	Time( const Time& t );
        ~Time( ){ }

	uint32_t	NextSubframe( );
	inline uint32_t FrameSlot( ) const
	{
		return ( subframe * 2 + slot );
	}
	void	Reset( CyclicPrefix cp = lteCP_Short );
	uint32_t 	sfn;
	uint32_t	subframe;
	uint32_t	slot;
	uint32_t	symbol;

	void	CorrectSfn( int32_t );
	void	NextSfn( );

	Time& operator++( );
	uint32_t	symbolCount;
//private:

};

/**
 *  @enum  Ran
 *  @brief Radio Access Network
 */
enum Ran {
	ranEutran, /*! Envolved UTRAN */
	ranUtran,  /*! UTRAN          */
	ranGeran  /*! GERAN          */
};

/**
 *  @enum  tNeighFreq
 *  @brief пїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅ.
 */
enum NeighFreq {
	freqIntra, /*! Intra frequency */
	freqInter /*! Inter frequency */
};

/**
 *  @struct MsgSystemInfo
 *  @brief  пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ.
 */
struct  SystemInfo {
	struct PLMN {
		char MCC[ 4 ]; //-V112
		char MNC[ 4 ]; //-V112
	};
	typedef std::vector< PLMN > PLMNs;
	PLMNs   	mPLMNs;
	uint32_t 	mCID;
	uint16_t	mTAC;
	int8_t   	mRxLevMin;
	int8_t  	mRxLevMinOffset;
	bool     	mBarred;
	bool     	mIntraFreqReselection;
	bool     	mTdd;
	uint32_t 	mUlDlConfig;         // TDD only
	uint32_t 	mSpecSubframeConfig; // TDD only
	int32_t	 	mSNR;
};
/**
 *  @struct MsgNeighCells
 *  @brief  пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅ пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ пїЅпїЅпїЅпїЅпїЅ/пїЅпїЅпїЅпїЅпїЅпїЅпїЅпїЅ/RAN.
 */
struct NeighCells {
	Ran        mRan;
	NeighFreq  mFreq;
	char      *mpXml;
};

using SysInformationBlock1 = std::vector<uint8_t>;
using PrbTypes = std::vector<PrbType>;

struct PilotInfo {
        bool  		mPresent;
        uint32_t 	mShift;
};
using PilotsInfo = std::vector<PilotInfo>;

struct M0M1{
    int32_t M0;
    int32_t M1;
    M0M1(int32_t M0, int32_t M1) : M0(M0), M1(M1) {}

    bool operator < (const M0M1& right) const {
        if(M0 < right.M0)           return true;
        else if(M0 > right.M0)      return false;
        else                        return M1 < right.M1;
    }

};


class M0M1Converter {
public:
    M0M1Converter();
    int32_t GetNid1(const M0M1& m0m1);
private:
    std::map<M0M1,int32_t> nid1Map;
    void    FillMap();      // 36211-a40 , Table 6.11.2.1-1

};

struct RdmCell {
	std::vector<uint32_t> rdmCorr;
	uint32_t peakPosition; // Позиция пика корреляции в отсчётах
};
using RdmCells = std::map<uint32_t, RdmCell>;


}


#endif /* LTE_TYPES_H_ */
