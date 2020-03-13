#include "PbchProcessing.h"
#include "Decoder.h"
#include "Common/PnSequence.h"
#include "Common/CellInfo.h"

using namespace std;

namespace Lte {

Pbch::Pbch( std::shared_ptr<Decoder>	decoder):
		decoder( decoder )
{
	sbits.reserve(PhyBlockLenNormCPInBits/(PBCH_SymbolsPerRadioFrame*BitPerSymbol_QPSK));
}

void Pbch::InitCellInfo( CellInfo& cell )
{
	PnSeqInit( cell.nCellId, &m_CX1init, &m_CX2init );
}

bool Pbch::Process( const Complex16* dec, uint32_t len, CellInfo& cellInfo, Time& time, uint32_t frame )
{
	Complex16* symbols  = const_cast< Complex16* >( dec );
	uint32_t offset = 2 * len * frame;
	sbits.resize(2*len);
	memcpy( sbits.data(), symbols, 2 * len * sizeof( SoftDecision ) );

	Scrambler( m_CX1init, m_CX2init, sbits.data(), 2 * len, offset );

 	TxAntPorts ports;
	bool crc  = decoder->ViterbiDecodeBCH( sbits.data(), 2 * len, BCH_MIB_Size, ports );

	if( crc ) {

		decoder->GetBits( mib.data(), BCH_MIB_Size / BitsInUInt8 );

		uint32_t dl_bandwidth   = 0;
		uint32_t phich_duration = 0;
		uint32_t phich_resource = 0;
		uint32_t sfn            = 0;

		uint32_t pos = 0;
		/* Bandwidth */
		ExtractBitsRev( mib.data(), &dl_bandwidth, pos, 3 );

		cellInfo.bw = static_cast< BandWidth >( dl_bandwidth + 1 );
		cellInfo.nDlRb = Bw2Rb( cellInfo.bw );
		pos += 3;

		/* PHICH duration */
		ExtractBitsRev( mib.data(), &phich_duration, pos, 1 );
		switch ( phich_duration ) {
			case 0: cellInfo.duration = ltePHICHDuration_Normal;   break;
			case 1: cellInfo.duration = ltePHICHDuration_Extended; break;
		}
		pos += 1;

		/* PHICH resource */
		ExtractBitsRev( mib.data(), &phich_resource, pos, 2 );
		switch ( phich_resource ) {
			case 0: cellInfo.resource = ltePHICHResource_1_6; break;
			case 1: cellInfo.resource = ltePHICHResource_1_2; break;
			case 2: cellInfo.resource = ltePHICHResource_1;   break;
			case 3: cellInfo.resource = ltePHICHResource_2;   break;
		}
		pos += 2;

		/* System frame number */
		ExtractBitsRev( mib.data(), &sfn, pos, 8 );
		time.sfn = sfn * 4 + frame;

		/* Ant ports configuration */
		cellInfo.antPorts = ports;

		return true;

	}
	else return false;
}

}
