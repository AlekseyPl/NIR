/*
 * lte_stream.cpp
 *
 *  Created on: 19.03.2015
 *      Author: dblagov
 */
#include <algorithm>
#include <math.h>
#include "LteStream.h"
#include "Support/LteSupport.h"
#include "Support/RsShift.h"

#include "LogicalProc/PbchProcessing.h"
#include "LogicalProc/PdschProcessing.h"
#include "LogicalProc/VrbMap.h"
#include "LogicalProc/Riv.h"
#include "Decoder/Decoder.h"


#include "System/DebugInfo.h"

using namespace std;

namespace Lte {

LteStream::LteStream( bool mrc_ ):
	  softDecisions(MAX_SD_COUNT), equalizedData(MAX_SD_COUNT), vrbMap( nullptr ), rivStore( nullptr ),
	time( lteCP_Short ), state( PBCHstate ), freePdsch( nullptr ), freeAccum( nullptr ),
	accumCntr( 0 ), symbPerSlot( 0 ), mrc( mrc_ ), cfiIndex( 0 ), fft( LTEFFTLen_1_4_MHz ),
	pdschTrbCnt( 0 ),
	debug( System::DebugInfo::Locate() )
{
	decoder = std::make_shared<Decoder>();
	pbch   	= std::make_shared<Pbch>(decoder);
	pdcch   = std::make_shared<Pdcch>(decoder);
	pdsch   = std::make_shared<Pdsch>(decoder);
}

LteStream::~LteStream( )
{

}

void 	LteStream::InitCellProc( const CellInfo& cell)
{
	cellInfo = cell;
	equalizer.reset( new Equalizer( lteTxAntPorts2, N_dl_rb_min, cellInfo.cp ));
	estimator.reset( new Estimator( lteBW_1_4_MHz, cellInfo.cp, cellInfo.nCellId ));

	cfiIndex = 0;
	accumCntr = 0;

	cellInfo.nDlRb 	 	   = PBCH_SubcarriersPerSymbol / Nsc_rb;
	cellInfo.antPorts 	   = lteTxAntPorts2;
	cellInfo.TDD_Config.Mi = 0;
	pbchIndex.Generate( cellInfo );
	pbch->InitCellInfo( cellInfo );
	fft = LTEFFTLen_1_4_MHz;

	time.Reset( cellInfo.cp );
	pcfich.Init( cellInfo.nCellId );
	pdcch->SetCellInfo( cellInfo );

	procState = empty;
	state     = PBCHstate;

	std::fill(softDecisions.begin(), softDecisions.end(), Complex16{0,0});
	std::fill(equalizedData.begin(), equalizedData.end(), ComplexFloat{0.0f, 0.0f});
}

LteStream::ProcState LteStream::ProcessSubframe( const void* data )
{
	const ComplexFloat* symbol = reinterpret_cast<const ComplexFloat*>(data) + LTEFFTLen_20_MHz / 2 - fft / 2;
	int32_t symbolScCount = Nsc_rb * cellInfo.nDlRb;
	int32_t symbolCount = cellInfo.SymbolsPerSlot( ) * LTESlotsInSubframe;

	// BCH located in 0th subrame
	// sfn getting from BCH, so before bch decoding, time.sfn is no care;
	time.subframe = ( procState == empty ) ? PBCH_Subframe : PDCCH_Subframe;
	time.slot 	  = 0;
	time.symbol   = 0;

	EqualizeSubframe( symbol );

	equalizer->CalcRSSI( symbol);

	for( int32_t sym = 0; sym < symbolCount; ++sym ) {
	    if ( ( state != PBCHstate ) && ( time.slot == PCFICH_Slot ) && ( time.symbol == PCFICH_Symbol ) )  {
			freePdsch = pdschSymb.data( );
			if ( mrc ) freeAccum = accum.data( );
			state = PDCCHstate;
			pdschTrBlocks.clear( );
	    }

	    switch( state ) {
	    case PBCHstate:
	    	{
	    		bool complete = PBCH_StateImpl( &softDecisions[ sym * symbolScCount ] );
	    		if( complete )
	    			state = waitPDCCHstate;
	    	}
	    	break;

	    case waitPDCCHstate:
	    	break;

	    case PDCCHstate:
			{
				bool complete = PDCCH_StateImpl( &softDecisions[ sym * symbolScCount ] );
				if( complete )
					state = ( pdschTrBlocks.empty() ) ?  waitPDCCHstate : PDSCHstate;
	    	}
	    	break;

		case PDSCHstate:
			{
				PDSCH_StateImpl( &softDecisions[ sym * symbolScCount ] );
				break;
			}
	    }
	    ++time;
	}
	time.NextSfn();

	return procState;
}

void LteStream::EqualizeSubframe( const ComplexFloat* data )
{
	Time currTime = time;
	int32_t symbolScCount = Nsc_rb * cellInfo.nDlRb;
	int32_t symbolCount = cellInfo.SymbolsPerSlot( ) * LTESlotsInSubframe;
	const ComplexFloat* symbolData = data;
	for( int32_t sym = 0; sym < symbolCount; ++sym ) {
		bool rs[ MaxAntennaPorts ];
	    uint32_t rsshift[ MaxAntennaPorts ];
	    uint32_t len[ MaxAntennaPorts ];

	    estimator->GetRsInfo( currTime, rs, rsshift );

		if( rs[ 0 ] ) len[ 0 ] = estimator->EstimateH( currTime, symbolData, fft, rsshift[ 0 ], hAnt0.data() );
		if( rs[ 1 ] ) len[ 1 ] = estimator->EstimateH( currTime, symbolData, fft, rsshift[ 1 ], hAnt1.data() );

		if( rs[ 0 ] ) equalizer->EstimateSymbol( hAnt0.data(), len[ 0 ], lteTxAntPort1, rsshift[ 0 ], sym );
		if( rs[ 1 ] ) equalizer->EstimateSymbol( hAnt1.data(), len[ 1 ], lteTxAntPort2, rsshift[ 1 ], sym );

	    ++currTime;
	    symbolData += LTEFFTLen_20_MHz;
	}
	equalizer->EstimeSubframe( );


	currTime = time;
	uint32_t halfSC = ( cellInfo.nDlRb * Nsc_rb ) / 2;
	uint32_t offset = fft / 2 - halfSC;

	symbolData = data + offset;
	for( int32_t sym = 0; sym < symbolCount; ++sym ) {
		if( cellInfo.antPorts == lteTxAntPorts1 ){
			equalizer->Equalize1AntPort( symbolData, &equalizedData[ sym * symbolScCount ], sym );
	    }
	    else if( cellInfo.antPorts == lteTxAntPorts2 ) {
			const ComplexFloat* srcPrb = symbolData;
			ComplexFloat* dstPrb = &equalizedData[ sym * symbolScCount ];
			bool rsFlag = RefSymbPresent( cellInfo.cp, cellInfo.antPorts, currTime.symbol );

			for( int32_t rb = 0; rb < cellInfo.nDlRb; ++rb ) {
				equalizer->EqualizePrb2AntPort( srcPrb, rb, rsFlag, dstPrb, sym );
	    		srcPrb += Nsc_rb;
	    		dstPrb += Nsc_rb;
			}
	    }
	    ++currTime;
	    symbolData += LTEFFTLen_20_MHz;
	}

	equalizer->Normalize( equalizedData.data(), softDecisions.data(), symbolCount * symbolScCount );
}

bool LteStream::PBCH_StateImpl( const Complex16* sd )
{
	bool complete = false;

	if ( ( time.slot == PBCH_Slot ) && ( time.symbol < PBCH_SymbolsPerRadioFrame ) && ( time.subframe == PBCH_Subframe ) ) {

		if( time.symbol == 0 )			pbchStore.Init( );

		auto &index = pbchIndex[ time.symbol ];
		for ( uint32_t i = 0, j = pbchStore.pos; i < index.size(); ++i, ++j )
			pbchStore.symbols[ j ] = sd[ index[ i ] ];

		pbchStore.pos += index.size();
		pbchStore.count++;

		if( pbchStore.count == PBCH_SymbolsPerRadioFrame ) {
//			debug.SendText("Try to decode PBCH" );

			for( uint32_t fr = 0; fr < PBCH_RadioFrames; ++fr ) {
				bool ret = pbch->Process( pbchStore.symbols, pbchStore.pos, cellInfo, time, fr );
				if( ret ) { // BCH is decoded
					std::fill(softDecisions.begin(), softDecisions.end(), Complex16{0,0});
					std::fill(equalizedData.begin(), equalizedData.end(), ComplexFloat{0.0f, 0.0f});

					equalizer.reset(new Equalizer( cellInfo.antPorts, cellInfo.nDlRb, cellInfo.cp ));
					estimator.reset(new Estimator( cellInfo.bw, cellInfo.cp, cellInfo.nCellId ));

					vrbMap.reset(new VrbMap( cellInfo.nDlRb ));
					rivStore.reset( new RivStore( cellInfo.nDlRb ));
					symbPerSlot = cellInfo.SymbolsPerSlot( );

					GenPrbTypeMap( cellInfo.nDlRb, prbTypes );
					GeneratePilotsInfo( );
					pcfichIndex.Generate( cellInfo );

					controlChansMap.SetCellInfo( cellInfo );
					controlChansMap.InsertPhich( time.NextSubframe( ) );
					controlChansMap.InsertPcfich( pcfichIndex );
					controlChansMap.InsertPdcch( );

					debug.SendText( "BCH decoded, BCH Symbol - %d, SFN - %d ", time.symbol, time.sfn );
					debug.SendText( Bw2Str(cellInfo.bw).c_str() );

					AllocatePdsch( );
					pdcch->SetPBCHInfo( cellInfo.nDlRb, cellInfo.antPorts );

					fft = Bw2Fft( cellInfo.bw );
					procState = MIBdecoded;
					complete = true;
					break;
				}
			}
		}
	}
	return complete;
}

bool LteStream::PDCCH_StateImpl( const Complex16* sd )
{
	bool ret = false;
	if( time.subframe == 5 ) {
		if ( ( time.slot == PCFICH_Slot ) && ( time.symbol == PCFICH_Symbol ) ) {
			uint32_t p = 0;
			auto pcfichQuad = controlChansMap.GetPcfichQuad( );
			for ( uint32_t i = 0; i < CFI_QuadrupletsCount; ++i ) {
				for ( uint32_t j = 0; j < QuadrupletSize; ++j )
					pcfichQuad[ i ].Q4[ j ] = sd[ pcfichIndex[ p++ ] ];
			}

			SoftDecision* pcfichSD = reinterpret_cast< SoftDecision* >( pcfichQuad.data() );
			auto cfi = pcfich.OnReceiveData( pcfichSD, CFI_CodeWordsSize, time.FrameSlot( ) );
			cfiIndex = pcfich.Cfi2Index(cfi);

			int32_t d = CFI_Variants - controlChansMap.size( );
			if ( cfiIndex < d ) cfiIndex = 0;
			else	cfiIndex -= d;

		}
		// PDCCH
		uint32_t pdcchSymbCnt = controlChansMap[ cfiIndex ].size( );
		if ( ( time.slot == PDCCH_Slot ) && ( time.symbol < pdcchSymbCnt ) ) {
			ExtractPdcch( sd, time.symbol );
			if ( ( time.symbol + 1 ) == pdcchSymbCnt ) {
				auto pdcchQuad = controlChansMap.GetPdcchQuad( );
				auto pdcchQuadCnt = controlChansMap.GetPdcchQuadCnt( );
				DciIndQueue queue;
				pdcch->ProcessQuadruplets( pdcchQuad.data(), pdcchQuadCnt[ cfiIndex ], time.subframe, queue );
				pdschTrbCnt = 0;
				if( !queue.empty( ) ) {
					debug.SendText("DCI is decoded. TRB count %d ", queue.size());
					accumCntr++;
					while ( !queue.empty( ) ) 	{
						ProcessDci( queue.front( ) );
						queue.pop( );
					}
				}
				ret = true;
			}
		}
	}
    return ret;
}

bool LteStream::PDSCH_StateImpl( const Complex16* sd )
{
	ExtractPdsch( sd, time.FrameSlot( ), time.symbol );
	if ( ( time.slot == 1 ) && ( ( time.symbol + 1 )== symbPerSlot ) ) {
		for( auto& tr : pdschTrBlocks) {
			TranspBlockParam param = tr.GetTranspBlockParam( time.sfn, time.FrameSlot( ) );
			if(  tr.RNTI( )== SI_RNTI ) {

				if( tr.GetFormat( ) == dciFormat1A )
					debug.SendText( "Start decoding SIB, DCI-format1A at %d sfn", time.sfn );
				else if( tr.GetFormat( ) == dciFormat1C )
					debug.SendText( "Start decoding SIB, DCI-format1C at %d sfn", time.sfn );

				bool ret = pdsch->ProcessTranspBlock( param );
				if( ret ) procState = SIBdecoded;
			}
		}
	}
	return false;
}

void LteStream::ExtractPdcch( const Complex16* sd, uint32_t symbol )
{
	QuadrupletPtrs& p = controlChansMap[ cfiIndex ][ symbol ];


	uint32_t	regCount = p.size();
	TxAntPorts ap = cellInfo.PDCCHAntPorts( );
	uint32_t variant = ( cellInfo.RefSymbPresent( ap, symbol ) ) ? ( GetRSShift( cellInfo, ap, PDCCH_Slot, symbol ) + 1 ) : 0;

	switch ( variant ) {
	case 0:
		for ( uint32_t i = 0; i < regCount; ++i ) {
			p[ i ]->Q4[ 0 ] = sd[ 0 ];
			p[ i ]->Q4[ 1 ] = sd[ 1 ];
			p[ i ]->Q4[ 2 ] = sd[ 2 ];
			p[ i ]->Q4[ 3 ] = sd[ 3 ];
			sd += 4;
		}
		break;

	case 1:
		for ( uint32_t i = 0; i < regCount; ++i ) {
			p[ i ]->Q4[ 0 ] = sd[ 1 ];
			p[ i ]->Q4[ 1 ] = sd[ 2 ];
			p[ i ]->Q4[ 2 ] = sd[ 4 ];
			p[ i ]->Q4[ 3 ] = sd[ 5 ];
			sd += 6;
		}
		break;

	case 2:
		for ( uint32_t i = 0; i < regCount; ++i ) {
			p[ i ]->Q4[ 0 ] = sd[ 0 ];
			p[ i ]->Q4[ 1 ] = sd[ 2 ];
			p[ i ]->Q4[ 2 ] = sd[ 3 ];
			p[ i ]->Q4[ 3 ] = sd[ 5 ];
			sd += 6;
		}
		break;

	case 3:
		for ( uint32_t i = 0; i < regCount; ++i ) {
			p[ i ]->Q4[ 0 ] = sd[ 0 ];
			p[ i ]->Q4[ 1 ] = sd[ 1 ];
			p[ i ]->Q4[ 2 ] = sd[ 3 ];
			p[ i ]->Q4[ 3 ] = sd[ 4 ];
			sd += 6;
		}
		break;
	}
}

void LteStream::ExtractPdsch( const Complex16* sd, uint32_t slot, uint32_t symbol )
{
	bool common = IsPbchPresent( slot, symbol ) || IsSyncPresent( slot, symbol );
	uint32_t pilots_shift = static_cast<uint32_t>(pilotsInfo[ symbol ].mShift);
	bool pilots = pilotsInfo[ symbol ].mPresent;

	for ( auto& trb: pdschTrBlocks ) {
		PrbList& prb = ( slot & 1 ) ?  trb.mOddPrbList : trb.mEvenPrbList;

		for( auto& p : prb) {
			PrbType t = common ? prbTypes[ p ] : prbPdsch;
			if ( t == prbCommon ) continue;

			const Complex16* symb = &sd[ p * Nsc_rb ];
			uint32_t sw = ( mrc ? 2 : 0 ) | ( pilots ? 1 : 0 );
			switch ( sw ) {
			case 0:
				trb.AddRB( symb, t );
				break;

			case 1:
				trb.AddRB( symb, t, pilots_shift );
				break;

			case 2:
				trb.AddRbMrc( symb, t );
				break;

			case 3:
				trb.AddRbMrc( symb, t, pilots_shift );
				break;
			}
		}
	}
}

void LteStream::ProcessDci( const MsgDciInd &msg )
{
	TrBlock* b;

	switch ( msg.mFormat ) {
	case dciFormat1A:
		{
			const DciFormat1A& f = msg.VF.mFormat1A;
			if ( f.mLocVRBAssignment == LocalVRB ) b = AssignLocalVrb( f.mRBA.mBits, msg.mRNTI );
			else 		b = AssignDistrVrb( f.mRBA.mBits, f.mNgap.mBits ? gap2 : gap1, msg.mRNTI );
			if( b ) b->SetFormat( f );
		}
		break;

	case dciFormat1B:
	break;

	case dciFormat1C:
		{
			const DciFormat1C& f = msg.VF.mFormat1C;
			b = AssignDistrVrb( f.mRBA.mBits, f.mNgap.mBits ? gap2 : gap1, msg.mRNTI );
			if( b ) b->SetFormat( f );
		}
		break;
	}
}

void LteStream::GeneratePilotsInfo( )
{
	uint32_t symb  = cellInfo.SymbolsPerSlot( );
	pilotsInfo.resize( symb );

	for ( uint32_t i = 0; i < symb; ++i ) {
		pilotsInfo[ i ].mPresent = cellInfo.RefSymbPresent( i );
		if ( pilotsInfo[ i ].mPresent ) {
			switch ( cellInfo.antPorts ) {
			case lteTxAntPorts1:
				pilotsInfo[ i ].mShift = RefSymbShift( cellInfo.nCellId, 0, 0, i );
				break;

			default:
				pilotsInfo[ i ].mShift = min( ( cellInfo.nCellId + 0 ) % 6, ( cellInfo.nCellId + 3 ) % 6 );
				break;
			}
		}
	}
}
void LteStream::AllocatePdsch( )
{
	uint32_t symb = cellInfo.SymbolsPerSlot( );
	uint32_t pdcchSymb;
	if ( cellInfo.duration == ltePHICHDuration_Normal ) {
		pdcchSymb = PDCCH_NormDurationMinSymbols;
		if ( cellInfo.nDlRb <= 10 ) pdcchSymb++;
	}
	else pdcchSymb = PDCCH_ExtDurationMinSymbols;

	uint32_t s = ( symb * LTESlotsInSubframe - pdcchSymb ) * cellInfo.nDlRb * Nsc_rb * BitPerSymbol_QPSK;
	pdschSymb.resize(s);
	if ( mrc ) 	accum.resize( s, Complex32{0,0} );
}

void LteStream::UpdateFreePdsch( uint32_t l_crbs )
{
	uint32_t s      = controlChansMap[ cfiIndex ].size( );
	s = cellInfo.SymbolsPerSlot( ) * LTESlotsInSubframe - s;
	freePdsch += s * l_crbs * Nsc_rb;
	if ( mrc ) freeAccum += s * l_crbs * Nsc_rb;
}

TrBlock* LteStream::AssignLocalVrb( uint32_t riv, Rnti rnti )
{
	if ( !rivStore->CheckLocalVrb( riv ) )		return nullptr;

	const Riv &r = rivStore->GetLocalVrb( riv );

	pdschTrBlocks.push_back(TrBlock( rnti, freePdsch, r.mLcrbs, cellInfo,  mrc ? freeAccum : nullptr, accumCntr ));
	TrBlock *b = &pdschTrBlocks.back();

	UpdateFreePdsch( r.mLcrbs );

	b->mEvenPrbList.resize( r.mLcrbs );
	b->mOddPrbList.resize( r.mLcrbs );

	for ( uint32_t n = 0; n < r.mLcrbs; ++n )
		b->mEvenPrbList[ n ] = r.mRBstart + n;
	b->mOddPrbList.assign( b->mEvenPrbList.begin( ), b->mEvenPrbList.end( ) );
	return b;
}

TrBlock* LteStream::AssignDistrVrb( uint32_t riv, Gap gap, Rnti rnti )
{
	if ( !rivStore->CheckDistrVrb( riv, gap ) )		return nullptr;

	const Riv &r = rivStore->GetDistrVrb( riv, gap );

	pdschTrBlocks.push_back(TrBlock( rnti, freePdsch, r.mLcrbs, cellInfo, mrc ? freeAccum : nullptr, accumCntr ));
	TrBlock *b = &pdschTrBlocks.back( );

	UpdateFreePdsch( r.mLcrbs );

	b->mEvenPrbList.resize( r.mLcrbs );
	b->mOddPrbList.resize( r.mLcrbs );
	for ( uint32_t n = 0; n < r.mLcrbs; ++n ) {
		b->mEvenPrbList[ n ] = vrbMap->MapPrb( r.mRBstart + n, 0, gap );
		b->mOddPrbList[ n ]  = vrbMap->MapPrb( r.mRBstart + n, 1, gap );
	}
	// 3GPP TS 36.211 6.3.5 Mapping to resource elements
	sort( b->mEvenPrbList.begin(), b->mEvenPrbList.end() );
	sort( b->mOddPrbList.begin(), b->mOddPrbList.end());
 	return b;
}

bool LteStream::IsSyncPresent( uint32_t slot, uint32_t symbol ) const
{
	bool p;
	if ( cellInfo.duplex == lteTDD ) {
		static const uint32_t LTESyncSlot0 = 1;
		static const uint32_t LTESyncSlot5 = 11;
		p = ( ( slot == LTESyncSlot0 ) || ( slot == LTESyncSlot5 ) );
		if ( p ) p = symbol >= ( ( cellInfo.cp == lteCP_Short ) ? LTESSSSymbS_Tdd : LTESSSSymbL_Tdd );
	}
	else {
		static const uint32_t LTESyncSlot0 = 0;
		static const uint32_t LTESyncSlot5 = 10;
		p = ( ( slot == LTESyncSlot0 ) || ( slot == LTESyncSlot5 ) );
		if ( p ) p = symbol >= ( ( cellInfo.cp == lteCP_Short ) ? LTESSSSymbS_Fdd : LTESSSSymbL_Fdd );
	}
	return p;
}

void LteStream::GenPrbTypeMap( uint32_t nDlRb, PrbTypes& prbs )
{
	prbs.resize(nDlRb);

	if ( nDlRb == 6 ) 	prbs.assign(nDlRb, prbCommon );
	else {

		prbs.assign( nDlRb, prbPdsch );
		PrbTypes::iterator e = prbs.begin( ) + ( nDlRb / 2 - 3 );

		if ( nDlRb & 1 ) {
			*e++ = prbPdschLeftHalf;
			for ( uint32_t i = 0; i < 5; ++i )  *e++ = prbCommon;
			*e = prbPdschRightHalf;
		}
		else {
			for ( uint32_t i = 0; i < 6; ++i ) *e++ = prbCommon;
		}
	}
}

SysInformationBlock1&	LteStream::GetSib( )
{
	procState = MIBdecoded;
	return pdsch->GetSib1( );
}

}

