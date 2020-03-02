#include "LteSupport.h"
#include <cstring>


namespace Lte
{

bool RefSymbPresent( CyclicPrefix prefix, uint32_t antPort, uint32_t symbol )
{
	struct tRefSymbPos
	{
		unsigned RS0_0;
		unsigned RS0_1;
		unsigned RS1_0;
		unsigned RS1_1;
		unsigned RS2_0;
		unsigned RS3_0;
	};

	static const tRefSymbPos NormRefSymb = { 0, 4, 0, 4, 1, 1 };
	static const tRefSymbPos ExtRefSymb  = { 0, 3, 0, 3, 1, 1 };

	const tRefSymbPos* p;
	switch ( prefix ) {
		case lteCP_Long : p = &ExtRefSymb;  break;
		case lteCP_Short: p = &NormRefSymb; break;
	}

	bool present;
	switch ( antPort ) {
		case 0: present = ( p->RS0_0 == symbol ) || ( p->RS0_1 == symbol ); break;
		case 1: present = ( p->RS1_0 == symbol ) || ( p->RS1_1 == symbol ); break;
		case 2: present = ( p->RS2_0 == symbol ); break;
		case 3: present = ( p->RS3_0 == symbol ); break;
	}
	return present;
}

bool RefSymbPresent( CyclicPrefix prefix, TxAntPorts antPorts, uint32_t symbol )
{
	bool p = false;
	unsigned ap = AntPortsCnt( antPorts );
	for ( unsigned i = 0; !p && ( i < ap ); ++i )
		p = RefSymbPresent( prefix, i, symbol );

	return p;
}
unsigned RefSymbShift( uint32_t n_id_cell, uint32_t antPort, uint32_t slot, uint32_t symbol )
{
	uint32_t v;
	switch ( antPort ) {
		case 0: v = ( symbol == 0 ) ? 0 : 3;   break;
		case 1: v = ( symbol == 0 ) ? 3 : 0;   break;
		case 2: v = 3 * ( slot % 2 );          break;
		case 3: v = 3 + 3 * ( slot % 2 );      break;
	}
	return ( v + n_id_cell ) % 6;
}

uint32_t AntPortsCnt( TxAntPorts antPorts )
{
	uint32_t ap;
	switch ( antPorts ) {
		case lteTxAntPorts1: ap = 1; break;
		case lteTxAntPorts2: ap = 2; break;
		case lteTxAntPorts4: ap = 4; break; //-V112
	}

	return ap;
}

uint32_t RefSymbStep( TxAntPorts antPorts )
{
	uint32_t step;
	switch ( antPorts ) {
		case lteTxAntPorts1:
			step = 6;
			break;

		default:
			step = 3;
	}

	return step;
}

uint32_t QuadrupletsPerPRB( CyclicPrefix prefix, TxAntPorts antPorts, uint32_t symbols )
{
  static const uint32_t QuadWithoutRS = 3;
  static const uint32_t QuadWithRS    = 2;
  uint32_t quad = 0;
  antPorts = PdcchAntPorts( antPorts );
  for (uint32_t i = 0; i < symbols; ++i)
    quad += RefSymbPresent(prefix, antPorts, i) ? QuadWithRS : QuadWithoutRS;

  return quad;
}

uint32_t Bw2Rb( BandWidth bw  )
{
	uint32_t r = Nrb_max_1_4_MHz;
	switch ( bw ) {
		case lteBW_Sync:
			r = Nrb_max_1_4_MHz;
		break;

		case lteBW_1_4_MHz:
			r = Nrb_max_1_4_MHz;
		break;

		case lteBW_3_MHz:
			r = Nrb_max_3_MHz;
		break;

		case lteBW_5_MHz:
			r = Nrb_max_5_MHz;
		break;

		case lteBW_10_MHz:
			r = Nrb_max_10_MHz;
		break;

		case lteBW_15_MHz:
			r = Nrb_max_15_MHz;
		break;

		case lteBW_20_MHz:
			r = Nrb_max_20_MHz;
		break;

	}
	return r;
}

uint32_t Rb2Fft( uint32_t nDlRb )
{
	uint32_t fft = LTEFFTLen_1_4_MHz;

	switch( nDlRb ) {
		case Nrb_max_3_MHz:
			fft = LTEFFTLen_3_MHz;
		break;

		case Nrb_max_5_MHz:
			fft = LTEFFTLen_5_MHz;
		break;

		case Nrb_max_10_MHz:
			fft = LTEFFTLen_10_MHz;
		break;

		case Nrb_max_15_MHz:
			fft = LTEFFTLen_15_MHz;
		break;

		case Nrb_max_20_MHz:
			fft = LTEFFTLen_20_MHz;
		break;
	}
	return fft;
}

uint32_t ExtractRvCrc(const uint32_t *hd, uint32_t msgLength, uint32_t crcLength)
{

	uint32_t crcPos = 0;
	uint32_t rvCrc = 0;

	// extract message
	uint32_t div = msgLength / BitsInUInt8;
	uint32_t mod = msgLength % BitsInUInt8;

	const uint8_t* ptr = reinterpret_cast< const uint8_t* >( hd );

	uint8_t byte = 0;

	if( mod ) { // if msgLen no divide to 8 extract bit by bit
		for( uint32_t bpos = div * BitsInUInt8; bpos < msgLength; ++bpos ) {
			if( !( bpos % BitsInUInt8) ) byte = ptr[ bpos / BitsInUInt8 ];
			byte >>= 1;
		}

		for( uint32_t bpos = msgLength; bpos < ( msgLength + crcLength ); ++bpos ) {

			if( !( bpos % BitsInUInt8)  ) byte = ptr[bpos / BitsInUInt8];

			if( byte & 1 ) 		SetBit( &rvCrc, crcPos++ );
			else			  	ResetBit( &rvCrc, crcPos++ );

			byte >>= 1;
		}
	}
	else // if msgLen divide to 8 extract byte by byte
		std::memcpy( &rvCrc, &ptr[ div ], crcLength / BitsInUInt8 );


	return rvCrc;
}

void ExtractMsgBits(const uint32_t* hd, uint32_t msgLength, uint8_t* dst)
{
	uint8_t byte;

	// extract message
	uint32_t div = msgLength / BitsInUInt8;
	uint32_t mod = msgLength % BitsInUInt8;

	const uint8_t* ptr = reinterpret_cast< const uint8_t* >( hd );

	memcpy( dst, ptr, div );

	if( mod ) { // if msgLen no divide to 8 extract bit by bit
		for( uint32_t bpos = div * BitsInUInt8; bpos < msgLength; ++bpos ) {

			if( !( bpos % BitsInUInt8) ) byte = ptr[ bpos / BitsInUInt8 ];

			if( byte & 1 ) 	SetBit( dst, bpos );
			else 			ResetBit( dst, bpos );

			byte >>= 1;
		}
	}
}

} // namespace LTE

