
#ifndef __LTE_CONST_H__
#define __LTE_CONST_H__

#include <stdint.h>

namespace Lte {

const uint32_t BitsInUInt32 = 32; //-V112
const uint32_t BitsInUInt16 = 16;
const uint32_t BitsInUInt8  = 8;

const uint8_t  Mask1In8  = 1;
const uint16_t Mask1In16 = 1;
const uint32_t Mask1In32 = 1;
const uint64_t Mask1In64 = 1;

/************************************************************************/
/*                            3GPP TS 36.212                            */
/************************************************************************/
// TS 36.212, 5.1.2 Code block segmentation and code block CRC attachment.
const uint8_t Bit0    = 1;
const uint8_t Bit1    = static_cast<uint8_t>(-1);
const uint8_t BitNull = 0;

using SoftDecision=int16_t;


const SoftDecision SoftDec0 = 16384;
const SoftDecision SoftDec1 = -16384;
const SoftDecision SoftDecPunct = 0;

const uint32_t 	BitPerSymbol_QPSK  = 2;
const uint32_t 	BitPerSymbol_QAM16 = 4; //-V112
const uint32_t 	BitPerSymbol_QAM64 = 6;
// Generic
const uint32_t 	MaxCTCBlockSize = 6144;
const uint32_t 	MinCTCBlockSize = 40;

const uint32_t 	Polynomial0	  = 0133; // dec
const uint32_t 	Polynomial1	  = 0171; // dec
const uint32_t 	Polynomial2	  = 0165; // dec

const uint32_t 	TurboCodeRate  = 3;
const uint32_t 	TurboCodeTail  = 4; //-V112
const uint32_t 	ConvCodeRate   = 3;
const uint32_t 	Constraint     = 7;
const uint32_t 	TurboCRC_Size  = 24;
const uint32_t  TurboIterCount = 16;
// SFN
const uint32_t	SFN_Mask = ((1 << 10) - 1);
const uint32_t 	SFN_Min  = 0;
const uint32_t 	SFN_Max  = 1023;
// PBCH
const uint32_t 	PBCH_Subframe                = 0;
const uint32_t 	PBCH_Slot                    = 1;
const uint32_t  PBCH_SubcarriersPerSymbol 	 = 72;
const uint32_t 	PBCH_SubcarriersPerRefSymbol = 48;
const uint32_t  PBCH_SymbolsPerRadioFrame    = 4; //-V112
const uint32_t  PBCH_RadioFrames             = 4; //-V112
// CFI
const uint32_t 	PCFICH_Slot       = 0;
const uint32_t 	PCFICH_Symbol     = 0;
const uint32_t 	CFI_CodeWordsSize = 32; //-V112
// HI
const uint32_t 	HI_CodeWordSize   = 3;
// PDCCH
const uint32_t  PDCCH_Subframe 		  = 5;
const uint32_t 	PDCCH_Slot 			  = 0;
const uint32_t	MaxDciLength 		  = 57;
const uint32_t	DCI_CRC_Size 		  = 16;
const uint32_t  DCCH_Length 		  = MaxDciLength + DCI_CRC_Size;
const uint32_t	DCCH_Length8 		  = ( DCCH_Length + 7 ) / BitsInUInt8;
const uint16_t 	UE_TX_AntSelMaskPort0 = 0x0000;
const uint16_t 	UE_TX_AntSelMaskPort1 = 0x0001;
// DL-SCH
const uint32_t 	MaxDL_SCH_BlockSize = 75400;
const uint32_t  Sib1FramePeriod = 8;
/************************************************************************/
/*                            3GPP TS 36.211                            */
/************************************************************************/
const uint32_t 	MaxAntennaPorts              = 4; //-V112
const uint32_t 	QuadrupletSize               = 4; //-V112
// BCH
const uint32_t 	BCH_MIB_Size                 = 24;
const uint32_t 	BCH_CRC_Size                 = 16;
const uint32_t 	BCH_TransportBlockSize       = (BCH_MIB_Size + BCH_CRC_Size);
const uint32_t 	BCH_CodedBlockSize           = (BCH_TransportBlockSize * ConvCodeRate);
// PBCH
const uint32_t 	PBCH_PRB                     = 6;
// CFI
const uint32_t 	CFI_QuadrupletsCount         = 4; //-V112
const uint32_t 	CFI_Symbols                  = (QuadrupletSize * CFI_QuadrupletsCount);
const int32_t 	CFI_Variants                 = 3;
const uint32_t 	CFI_N_DL_RB                  = 10;  // ���������� ��������� ������, ��� ������� �� PDCCH ���������� �������������� OFDM ������
const uint32_t  SC_WithPilots_InREG			 = 6;
// HI
const uint32_t 	PHICH_MaxUseSymbol           = 3;
const uint32_t 	PHICH_GroupREGs              = 3;
// PDCCH
const uint32_t 	MaxCssSize        = 16;
const uint32_t 	AggLev4           = 4; //-V112
const uint32_t 	AggLev8           = 8;
const uint32_t 	QuadrupletsPerCce = 9;
const uint32_t 	CceLen            = ( BitPerSymbol_QPSK * QuadrupletSize * QuadrupletsPerCce );
const uint32_t 	AggLev4Len        = AggLev4 * CceLen;
const uint32_t 	AggLev8Len        = AggLev8 * CceLen;
const uint32_t 	AggLev4Count      = MaxCssSize / AggLev4;
const uint32_t 	AggLev8Count      = MaxCssSize / AggLev8;

const uint32_t	PDCCH_MaxREGsPRB			 = 11;
const uint32_t  PDCCH_MaxREGs				 = 796;
const uint32_t 	PDCCH_MaxUseSymbols          = 4; //-V112
const uint32_t 	PDCCH_NormDurationMinSymbols = 1;   // ����������� ���������� ��������, ���������� PDCCH � ������ ������������� PHICH normal duration
const uint32_t 	PDCCH_ExtDurationMinSymbols  = 3;   // ����������� ���������� ��������, ���������� PDCCH � ������ ������������� PHICH extended duration
/************************************************************************/
/*                            3GPP TS 36.213                            */
/************************************************************************/

/**
 * 7. Physical downlink shared channel related procedures.
 */
const uint32_t 	M_DL_HARQ_Limit                    = 8;
const uint32_t 	M_DL_HARQ_FDD_Count                = 1;
const uint32_t 	M_DL_HARQ_FDD[M_DL_HARQ_FDD_Count] = { 8 };
const uint32_t 	M_DL_HARQ_TDD_Count                = 7;
const uint32_t 	M_DL_HARQ_TDD[M_DL_HARQ_TDD_Count] = { 4, 7, 10, 9, 12, 15, 6 };
// Modulation Coding Scheme
const uint32_t 	Min_MCS_Index = 0;
const uint32_t 	Max_MCS_Index = 28;
const uint32_t 	Min_TBS_Index = 0;
const uint32_t 	Max_TBS_Index = 26;
// Transport block size table (format 1C)
const uint32_t 	Max_TBS_Index_1C = 31;

const uint32_t    LocalVRB = 0;
const uint32_t    DistrVRB = 1;

/************************************************************************/
/*                            3GPP TS 36.321                            */
/************************************************************************/
// Table 7.1-1 RNTI values
const uint32_t   RNTI_Len    = 16;
const uint16_t   EmptyRNTI   = 0x0000;
const uint16_t   P_RNTI      = 0xFFFE;
const uint16_t   SI_RNTI     = 0xFFFF;
const uint16_t   RA_RNTI_Min = 0x0001;
const uint16_t   RA_RNTI_Max = 0x003C;
/************************************************************************/
/*                            3GPP TS 36.306                            */
/************************************************************************/

/**
 * 4. UE radio access capability parameters.
 */
// Table 4.1-1: Downlink physical layer parameter values set by the field ue-Category 
// Total number of soft channel bits
const uint32_t SoftChannelBits[] =
{
  250368,  // UE category 1
  1237248, // UE category 2
  1237248, // UE category 3
  1827072, // UE category 4
  3667200, // UE category 5
};

} // namespace LTE


#endif // __LTE_CONST_H__
