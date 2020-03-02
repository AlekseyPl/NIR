
#ifndef LTE_SUPPORT_H
#define LTE_SUPPORT_H

#include "LteTypes.h"
#include "LteDemConst.h"
#include <string>

namespace Lte {

template <class T> T CeilLog2(T k)
{
   uint32_t index = 0;

   uint32_t mask = 0x80000000;
   for( int32_t i = 0; i < 32; ++i ) {
	   if( k & mask ) {
		   index = 31 - i;
		   break;
	   }
	   mask >>= 1;
   }

  return static_cast< T >(index + 1);
}

inline uint32_t Bits2Int8( uint32_t sizeInBits )
{
	return ( sizeInBits + BitsInUInt8 - 1 ) / BitsInUInt8;
}

inline uint32_t Bits2Int16( uint32_t sizeInBits )
{
  return (sizeInBits + BitsInUInt16 - 1) / BitsInUInt16;
}

inline uint32_t Bits2Int32( uint32_t sizeInBits )
{
  return (sizeInBits + BitsInUInt32 - 1) / BitsInUInt32;
}

inline bool TestBit( const uint8_t *ptr, uint32_t pos )
{
	return ( ptr[ pos >> 3 ] & ( Mask1In8 << ( pos & 7 ) ) ) != 0;
}

inline bool TestBit( const uint16_t *ptr, uint32_t pos )
{
  return (ptr[pos >> 4] & (Mask1In16 << (pos & 15))) != 0;
}

inline bool TestBit( const uint32_t *ptr, uint32_t pos )
{
  return (ptr[pos >> 5] & (Mask1In32 << (pos & 31))) != 0;
}

inline void SetBit( uint8_t *ptr, uint32_t pos )
{
	ptr[ pos >> 3 ] |= ( Mask1In8 << ( pos & 7 ) );
}

inline void SetBit( uint16_t *ptr, uint32_t pos )
{
  ptr[pos >> 4] |= (Mask1In16 << (pos & 15));
}

inline void SetBit( uint32_t *ptr, uint32_t pos )
{
  ptr[pos >> 5] |= (Mask1In32 << (pos & 31));
}

inline void ResetBit( uint8_t *ptr, uint32_t pos )
{
	ptr[ pos >> 3 ] &= ~( Mask1In8 << ( pos & 7 ) );
}

inline void ResetBit( uint16_t *ptr, uint32_t pos )
{
  ptr[pos >> 4] &= ~(Mask1In16 << (pos & 15));
}

inline void ResetBit( uint32_t *ptr, uint32_t pos )
{
  ptr[pos >> 5] &= ~(Mask1In32 << (pos & 31));
}

inline void ToggleBit( uint8_t *ptr, uint32_t pos )
{
  ptr[pos >> 3] ^= (Mask1In8 << (pos & 7));
}

inline void ToggleBit( uint16_t *ptr, uint32_t pos )
{
  ptr[pos >> 4] ^= (Mask1In16 << (pos & 15));
}

inline void ToggleBit( uint32_t *ptr, uint32_t pos )
{
  ptr[pos >> 5] ^= (Mask1In32 << (pos & 31));
}

inline int16_t round16( float val )
{
	int16_t res;
	if( val >= 0.0 ) {
		res = ( val < 32767.0 ) ? ( int16_t )val : 32767;
	}
	else {
		res = ( val > -32768 ) ? ( int16_t )val : -32767;
	}

	return res;
}

template <typename TYPE> 
void ExtractBitsRev( const uint8_t* src, TYPE* dst, uint32_t pos, uint32_t count )
{
	for ( uint32_t i = 0, j = pos, k = count - 1; i < count; ++i, ++j, --k )
		if ( TestBit( src, j ) )
			SetBit( dst, k );
		else
			ResetBit( dst, k );
}

template <typename TYPE> 
void TopsyBits( const uint8_t* src, TYPE* dst, uint32_t count )
{
	for ( uint32_t i = 0, j = count - 1; i < count; ++i, --j )
		if ( TestBit( src, i ) )
			SetBit( dst, j );
		else
			ResetBit( dst, j );
}

template <typename TYPE> 
void TopsyBits( TYPE *srcDst, uint32_t count )
{
    for ( uint32_t i = 0, j = count - 1; i < (count / 2 ); ++i, --j ) {
        bool bit_i = TestBit( srcDst, i );
        bool bit_j = TestBit( srcDst, j );
        if ( bit_i != bit_j )        {
            if ( bit_i )  SetBit( srcDst, j );
            else          ResetBit( srcDst, j );
            if ( bit_j )  SetBit( srcDst, i );
            else          ResetBit( srcDst, i );
        }
    }
}

template <typename TYPE> TYPE DivideToPlusInf(TYPE dividend, TYPE divider)
{
    TYPE r = dividend / divider;
    if ( ( r * divider ) != dividend )
        r++;
    return r;
}

bool RefSymbPresent( CyclicPrefix prefix, uint32_t antPort, uint32_t symbol );

bool RefSymbPresent( CyclicPrefix prefix, TxAntPorts antPorts, uint32_t symbol );

unsigned RefSymbShift( uint32_t n_id_cell, uint32_t antPort, uint32_t slot, uint32_t symbol );

unsigned AntPortsCnt( TxAntPorts antPorts );

unsigned RefSymbStep( TxAntPorts antPorts );

uint32_t QuadrupletsPerPRB( CyclicPrefix prefix, TxAntPorts antPorts, uint32_t symbols );

inline TxAntPorts PdcchAntPorts( TxAntPorts antPorts )
{    
	TxAntPorts ap = antPorts;
	if ( ap == lteTxAntPorts1 )
		ap = lteTxAntPorts2;

	return ap;
}

inline uint32_t SymbolsPerSlot( CyclicPrefix prefix )
{
	return ( prefix == lteCP_Long ) ? LTESymbolsInSlotL : LTESymbolsInSlotS;
}

inline uint32_t Qm( Modulation mod )
{
	uint32_t q;
	switch ( mod ) {
		case lteQPSK : q = 2; break;
		case lteQAM16: q = 4; break; //-V112
		case lteQAM64: q = 6; break;
	}

	return q;
}

inline TxAntPort Num2TxAntPort(uint32_t number)
{
	TxAntPort p;
	switch (number) {
		case 0: p = lteTxAntPort1; break;
		case 1: p = lteTxAntPort2; break;
		case 2: p = lteTxAntPort3; break;
		case 3: p = lteTxAntPort4; break;
	}

	return p;
}

inline uint32_t TxAntPort2Num(TxAntPort port)
{
	unsigned n;
	switch (port) {
		case lteTxAntPort1: n = 0; break;
		case lteTxAntPort2: n = 1; break;
		case lteTxAntPort3: n = 2; break;
		case lteTxAntPort4: n = 3; break;
	}

	return n;
}

uint32_t Rb2Fft( uint32_t nDlRb );

uint32_t Bw2Rb(BandWidth bw ) ;

inline BandWidth Rb2Bw( uint32_t n_dl_rb, bool& error )
{
	BandWidth r;
	switch ( n_dl_rb ) {
		case Nrb_max_1_4_MHz:
			r = lteBW_1_4_MHz;
		break;

		case Nrb_max_3_MHz:
			r = lteBW_3_MHz;
		break;

		case Nrb_max_5_MHz:
			r = lteBW_5_MHz;
		break;

		case Nrb_max_10_MHz:
			r = lteBW_10_MHz;
		break;

		case Nrb_max_15_MHz:
			r = lteBW_15_MHz;
		break;

		case Nrb_max_20_MHz:
			r = lteBW_20_MHz;
		break;

		default:
			error = true;
	}

	return r;
}

inline uint32_t Bw2Fft( BandWidth bw ) {
	uint32_t fft = LTEFFTLen_1_4_MHz;
	switch( bw ) {
	case lteBW_1_4_MHz:
	case lteBW_Sync:
		fft = LTEFFTLen_1_4_MHz;
		break;
	case lteBW_3_MHz:
		fft = LTEFFTLen_3_MHz;
		break;

	case lteBW_5_MHz:
		fft = LTEFFTLen_5_MHz;
		break;

	case lteBW_10_MHz:
		fft = LTEFFTLen_10_MHz;
		break;

	case lteBW_15_MHz:
		fft = LTEFFTLen_15_MHz;
		break;

	case lteBW_20_MHz:
		fft = LTEFFTLen_20_MHz;
		break;
	}
	return fft;
}

inline std::string Bw2Str( BandWidth bw ) {
    std::string str;
    switch( bw ) {
            case lteBW_3_MHz:
                    str = "BandWidth 3 MHz, 15 resource blocks";
                    break;
            case lteBW_5_MHz:
                    str = "BandWidth 5 MHz, 25 resource blocks";
                    break;
            case lteBW_10_MHz:
                    str = "BandWidth 10 MHz, 50 resource blocks";
                    break;
            case lteBW_15_MHz:
                    str = "BandWidth 15 MHz, 75 resource blocks";
                    break;
            case lteBW_20_MHz:
                    str = "BandWidth 20 MHz, 100 resource blocks ";
                    break;
            default:
                    str = "BandWidth 1.4 MHz, 6 resource blocks, Sync Channel";
    }
    return str;
}

uint32_t ExtractRvCrc(const uint32_t* hdPtr, uint32_t msgLength, uint32_t crcLength );
void ExtractMsgBits(const uint32_t* hdPtr, uint32_t msgLength, uint8_t* dst);

inline void Add_I( Complex16 &s1, Complex16 s2 )
{
        s1.re += s2.re;
        s1.im += s2.im;
}

inline Complex16 Accumulate( Complex16 &value, Complex16 &delta )
{
        Add_I( value, delta );
        return value;
}


static inline uint32_t ComplexAbs( int32_t im, int32_t re )
{
        re = _abs( re ) ;
        im = _abs( im ) ;
        return ( re > im ) ? ( re + (im >> 2) ) : ( im + (re >> 2) ) ;
}



} // namespace LTE

#endif
