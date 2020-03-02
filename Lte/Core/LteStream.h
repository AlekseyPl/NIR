/*
 * lte_stream.h
 *
 *  Created on: 19.03.2015
 *      Author: dblagov
 */

#ifndef LTE_STREAM_H_
#define LTE_STREAM_H_

#include "Lte/Core/Common/LteTypes.h"
#include "Lte/Core/Common/TranspBlock.h"
#include "Lte/Core/Common/PbchIndex.h"
#include "Lte/Core/Common/PcfichIndex.h"
#include "Lte/Core/Common/CellInfo.h"
#include "Lte/Core/Common/PbchSymbStore.h"

#include "Lte/Core/PcfichProcessing.h"
#include "Lte/Core/PdcchProcessing.h"
#include "Lte/Core/ControlChansMap.h"

#include "Lte/Core/Estimator.h"
#include "Lte/Core/Equalizer.h"

#include <vector>
#include <memory>
#include <math.h>

namespace System {
	class DebugInfo;
}

namespace Lte {

class	Pbch;
class 	Pdsch;
class 	Decoder;
class 	RivStore;
class 	VrbMap;

class LteStream {
public:
    enum ProcState {
            empty,
            MIBdecoded,
            SIBdecoded
    };

    LteStream( bool mrc  = false);
    virtual ~LteStream( );

    void    InitCellProc( const CellInfo& cell );

    ProcState ProcessSubframe( const void* data );

    SysInformationBlock1&	GetSib( );

    CellInfo& GetCellInfo( )
    {
        return cellInfo;
    }

    float GetRSRP( )
    {
        if( estimator ) 	return estimator->GetRSRP( );
        else return 0;
    }

    float GetRSSI( )
    {
        if( equalizer ) 	return equalizer->GetRSSI( );
        else return 0;
    }

    float    GetCarrierRSSI( )
    {
        if( equalizer ) 	return equalizer->GetCarrierRSSI( );
        else return 0;
    }

    float GetRSRQ( )
    {
        if( ( equalizer )&&( estimator ) ) return  GetRSRP( ) * cellInfo.nDlRb / GetCarrierRSSI( ) ;
        else return 0;
    }

    void CorrectSFN( uint32_t sfnOffset )
    {
        time.CorrectSfn( sfnOffset + 1 ); // +1 cause we detect sfn from bch and starting from this is
    }

private:
    static const uint32_t MAX_SD_COUNT = N_dl_rb_max * Nsc_rb * LTESymbolsInSlotS * LTESlotsInSubframe;

    enum State {
        PBCHstate,
        waitPDCCHstate,
        PDCCHstate,
        PDSCHstate
    };
    State			state;
    ProcState			procState;

    std::shared_ptr<Decoder>	decoder;
    std::shared_ptr<Equalizer>	equalizer;
    std::shared_ptr<Estimator>	estimator;

    std::array<Complex16, MAX_SD_COUNT>		softDecisions;
    std::array<ComplexFloat, MAX_SD_COUNT>	equalizedData;

    ComplexFloat		hAnt0[ N_dl_rb_max * PilotSymbCnt ];
    ComplexFloat		hAnt1[ N_dl_rb_max * PilotSymbCnt ];

    Pcfich			pcfich;
    Pdcch			pdcch;
    std::shared_ptr<Pbch>	pbch;
    std::shared_ptr<Pdsch>	pdsch;
    ControlChansMap             controlChansMap;

    std::vector<Complex16>	pdschSymb;
    Accumulator			accum;
    TrBlocks			pdschTrBlocks;
    uint32_t			pdschTrbCnt;
    Complex16*			freePdsch;
    Complex32*			freeAccum;

    PbchSymbStore               pbchStore;
    PbchIndex			pbchIndex;
    PcfichIndex			pcfichIndex;
    PrbTypes                    prbTypes;
    std::shared_ptr<VrbMap>	vrbMap;
    std::shared_ptr<RivStore>	rivStore;
    PilotsInfo			pilotsInfo;

    CellInfo			cellInfo;
    uint32_t			fft;
    uint32_t			cfiIndex;
    int32_t			accumCntr;
    uint32_t			symbPerSlot;

    Time			time;
    bool			mrc;

    System::DebugInfo&  	debug;
    void			EqualizeSubframe( const ComplexFloat* data );

    bool 			PBCH_StateImpl( const Complex16* sd );
    bool 			PDCCH_StateImpl( const Complex16* sd );
    bool			PDSCH_StateImpl( const Complex16* sd );

    void 			GeneratePilotsInfo( );
    void			AllocatePdsch( );

    void			ExtractPdcch( const Complex16* sd, uint32_t symbol );
    void 			ExtractPdsch( const Complex16* sd, uint32_t slot, uint32_t symbol );

    void 			ProcessDci( const MsgDciInd &msg );
    void 			UpdateFreePdsch( uint32_t l_crbs );
    TrBlock*    		AssignLocalVrb( uint32_t riv, Rnti rnti );
    TrBlock*    		AssignDistrVrb( uint32_t riv, Gap gap, Rnti rnti );

    inline bool 		IsPbchPresent( uint32_t slot, uint32_t symbol ) const
    {
    	return ( ( slot == PBCH_Slot ) && ( symbol < PBCH_SymbolsPerRadioFrame ) );
    }
    bool 			IsSyncPresent( uint32_t slot, uint32_t symbol ) const;

    static void 		GenPrbTypeMap( uint32_t nDlRb, PrbTypes& prbs );
};

}

#endif /* LTE_STREAM_H_ */
