
#ifndef LTE_DEM_CONST_H
#define LTE_DEM_CONST_H

#include "LteTypes.h"

namespace Lte
{

const uint32_t	LTESyncCodeLen		= 62;
const uint32_t	LTESyncCodeHalfLen	= 31;

const uint32_t 	LTEBaseSamplingRate = 30720000;
const uint32_t 	LTEBaseSymbolLen    = 2048;
// Bands
const uint32_t	LTEBand_20_MHz        = 20000000;
const uint32_t	LTEBand_15_MHz        = 15000000;
const uint32_t	LTEBand_10_MHz        = 10000000;
const uint32_t	LTEBand_5_MHz         = 5000000;
const uint32_t	LTEBand_3_MHz         = 3000000;
const uint32_t	LTEBand_1_4_MHz       = 1400000;

const uint32_t 	LTEDownFactorSync     = 16;
const uint32_t 	LTEDownFactor_1_4_MHz = 16;
const uint32_t 	LTEDownFactor_3_MHz   = 8;
const uint32_t 	LTEDownFactor_5_MHz   = 4; //-V112
const uint32_t 	LTEDownFactor_10_MHz  = 2;
const uint32_t 	LTEDownFactor_15_MHz  = 1;
const uint32_t 	LTEDownFactor_20_MHz  = 1;

const uint32_t 	LTESamplingRate_Sync    = LTEBaseSamplingRate / LTEDownFactorSync;
const uint32_t 	LTESamplingRate_1_4_MHz = LTEBaseSamplingRate / LTEDownFactor_1_4_MHz;
const uint32_t 	LTESamplingRate_3_MHz   = LTEBaseSamplingRate / LTEDownFactor_3_MHz;
const uint32_t 	LTESamplingRate_5_MHz   = LTEBaseSamplingRate / LTEDownFactor_5_MHz;
const uint32_t 	LTESamplingRate_10_MHz  = LTEBaseSamplingRate / LTEDownFactor_10_MHz;
const uint32_t 	LTESamplingRate_15_MHz  = LTEBaseSamplingRate / LTEDownFactor_15_MHz;
const uint32_t 	LTESamplingRate_20_MHz  = LTEBaseSamplingRate / LTEDownFactor_20_MHz;

const uint32_t 	LTESymbolsInFrameS        = 140;
const uint32_t 	LTESymbolsInFrameL 		  = 120;
const uint32_t 	LTEShortCP0SymbolsInFrame = 20;
const uint32_t 	LTEShortCPXSymbolsInFrame = 120;
// Cyclic prefix
const uint32_t 	LTELongCP           = 512;
const uint32_t 	LTEShortCP0         = 160;
const uint32_t 	LTEShortCPX         = 144;
// Time
const uint32_t	LTESlotLength	    = 15360;
const uint32_t 	LTESymbolsInSlotS   = 7;
const uint32_t 	LTESymbolsInSlotL   = 6;
const uint32_t 	LTESlotsInSubframe  = 2;
const uint32_t 	LTESubframeInFrame  = 10;
const uint32_t 	LTEFramePerSec      = 100;
const uint32_t 	LTESymbolsPerSecS   = LTEFramePerSec * LTESubframeInFrame * LTESlotsInSubframe * LTESymbolsInSlotS;
const uint32_t 	LTESymbolsPerSecL   = LTEFramePerSec * LTESubframeInFrame * LTESlotsInSubframe * LTESymbolsInSlotL;
const uint32_t 	LTESubframeLength   = LTESlotLength * LTESlotsInSubframe;
const uint32_t	LTEFrameLength      = LTESlotLength * LTESlotsInSubframe * LTESubframeInFrame;
//Freq
const uint32_t 	LTEFFTLenSync     = 128;
const uint32_t 	LTEFFTLen_1_4_MHz = 128;
const uint32_t 	LTEFFTLen_3_MHz   = 256;
const uint32_t 	LTEFFTLen_5_MHz   = 512;
const uint32_t 	LTEFFTLen_10_MHz  = 1024;
const uint32_t 	LTEFFTLen_15_MHz  = 2048;
const uint32_t 	LTEFFTLen_20_MHz  = 2048;

const uint32_t	LTEFFTSubframeLenS = LTEFFTLen_20_MHz * LTESymbolsInSlotS * LTESlotsInSubframe;
const uint32_t	LTEFFTSubframeLenL = LTEFFTLen_20_MHz * LTESymbolsInSlotL * LTESlotsInSubframe;
const uint32_t 	LTEFFTFrameLenS    = LTEFFTSubframeLenS * LTESubframeInFrame;
const uint32_t 	LTEFFTFrameLenL	   = LTEFFTSubframeLenL * LTESubframeInFrame;
// PSS
const uint32_t 	LTENid2Min        = 0;
const uint32_t 	LTENid2Max        = 2;
const uint32_t 	LTEPSSPeriodInSub = 5;  // ������ PSS � ����������
const uint32_t	LTEPssShiftFddS   = LTEFFTLen_20_MHz * ( LTESymbolsInSlotS - 1 ) + LTEShortCPX * ( LTESymbolsInSlotS - 1 ) + LTEShortCP0;
const uint32_t	LTEPssShiftFddL   = LTEFFTLen_20_MHz * ( LTESymbolsInSlotL - 1 ) + LTELongCP * ( LTESymbolsInSlotL );
const uint32_t	LTEPssShiftTddS   = 2 * LTESlotLength + 2 * LTEFFTLen_20_MHz  + 2 * LTEShortCPX + LTEShortCP0;
const uint32_t	LTEPssShiftTddL   = 2 * LTESlotLength + 2 * LTEFFTLen_20_MHz  + 3 * LTELongCP;

// SSS
const uint32_t 	LTESSSSlot        = 0;
const uint32_t 	LTESSSSymbL_Fdd   = (LTESymbolsInSlotL - 2);
const uint32_t 	LTESSSSymbS_Fdd   = (LTESymbolsInSlotS - 2);
const uint32_t 	LTESSSSymbL_Tdd   = (LTESymbolsInSlotL - 1);
const uint32_t 	LTESSSSymbS_Tdd   = (LTESymbolsInSlotS - 1);
const uint32_t 	LTENid1Min        = 0;
const uint32_t 	LTENid1Max        = 167;
const uint32_t 	LTECellIdGroup    = ( LTENid1Max - LTENid1Min ) + 1;

const uint32_t	LTESssShiftFddS   = LTEShortCPX + LTEFFTLen_20_MHz;
const uint32_t	LTESssShiftFddL   = LTELongCP + LTEFFTLen_20_MHz;
const uint32_t	LTESssShiftTddS   = 3 * LTEFFTLen_20_MHz + 2 * LTEShortCPX + LTEShortCP0;
const uint32_t	LTESssShiftTddL   = 3 * LTEFFTLen_20_MHz + 3 * LTELongCP;

// TDD
const uint32_t	LTEConfigureCount = 7;
const uint32_t 	LTEDownlinkSubframeCntr[ LTEConfigureCount ] = { 2, 4, 6, 6, 7, 8, 3 };
const uint32_t	LTETDDConfigTable[ LTEConfigureCount ][ LTESubframeInFrame ] =
{
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
		{ 1, 0, 0, 0, 1, 1, 0, 0, 0, 1 },
		{ 1, 0, 0, 1, 1, 1, 0, 0, 1, 1 },
		{ 1, 0, 0, 0, 0, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 1, 1, 1, 1, 1, 1, 1 },
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 1 },
};

//extern const CellIdGroup MappingCellIdGroup[LTECellIdGroup];
const uint32_t 	LTESSSSymbLen     = 64; // ��������� ����� ������� Secondary Synchro Signal
// Cell ID
const uint32_t 	LTENidcellMin     = 0;
const uint32_t 	LTENidcellMax     = 3 * LTENid1Max + LTENid2Max;
// N(RB)max
const uint32_t 	N_dl_rb_max         = 110;
const uint32_t 	N_dl_rb_min         = 6;
const uint32_t 	Nrb_max_1_4_MHz     = 6;
const uint32_t 	Nrb_max_3_MHz       = 15;
const uint32_t 	Nrb_max_5_MHz       = 25;
const uint32_t 	Nrb_max_10_MHz      = 50;
const uint32_t 	Nrb_max_15_MHz      = 75;
const uint32_t 	Nrb_max_20_MHz      = 100;
// Nsc rb
const uint32_t 	Nsc_rb            = 12;
const uint32_t  Nsc_bw            = 15000;
// Reference signals

const RefSymbPos NormRefSymb( 0, 4, 0, 4, 1, 1 ); //-V112
const RefSymbPos ExtRefSymb( 0, 3, 0, 3, 1, 1 );

const uint32_t 	PilotSymbSecondPos0         = 3;
const uint32_t  PilotSymbCnt                = 2;

const uint32_t 	NormPilotSymb[PilotSymbCnt] = { 0, 4 };
const uint32_t 	ExtPilotSymb[PilotSymbCnt]  = { 0, 3 };
const uint32_t 	PilotStep                   = Nsc_rb / PilotSymbCnt;
const uint32_t  PilotScInRb                 = Nsc_rb/PilotStep;
const uint32_t	PilotShift					= 3;
const uint32_t	RefSymbInSf                 = PilotSymbCnt * LTESlotsInSubframe;
//

} // namespace LTE


#endif
