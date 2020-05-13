/**
 *  @file   lte_vrb.cpp
 *  @author Dolgov Sergey
 *  @date   24/02/2014
 */
#include "LogicalProc/VrbMap.h"
#include "LogicalProc/DciSupport.h"
#include <math.h>

namespace Lte {

VrbMap::VrbMap( uint32_t nDlRb_ ):
	  nDlRb( nDlRb_ )
{
	uint32_t rbg_size     = lteGetType0ResAllocRbgSize( nDlRb );
	uint32_t nGap1        = lteGetNgap1( nDlRb );
	uint32_t nDlVrbGap1   = lteGetNvrbGap1( nDlRb );


	MapEven( nDlVrbGap1, rbg_size, mGap1Even );
	MapOdd(  nDlVrbGap1, mGap1Even, mGap1Odd );

	CorrectMap( nGap1, nDlVrbGap1, mGap1Even );
	CorrectMap( nGap1, nDlVrbGap1, mGap1Odd );

	if ( nDlRb >= 50 )	{
		uint32_t nGap2       = lteGetNgap2( nDlRb );
		uint32_t nDlVrbGap2  = 2 * lteGetNvrbGap2( nDlRb );

		MapEven( nDlVrbGap2, rbg_size, mGap2Even );
		MapOdd(  nDlVrbGap2, mGap2Even, mGap2Odd );

		CorrectMap( nGap2, nDlVrbGap2, mGap2Even );
		CorrectMap( nGap2, nDlVrbGap2, mGap2Odd );
	}
}

VrbMap::~VrbMap( )
{

}

size_t VrbMap::MapPrb( uint32_t n_vrb, uint32_t n_s, Gap gap )
{
	size_t* map = nullptr;
	switch ( gap ) {
	case gap1:
		map = ( n_s & 1 ) ? mGap1Odd.data() : mGap1Even.data();
		break;

	case gap2:
		map = ( n_s & 1 ) ? mGap2Odd.data() : mGap2Even.data();
		break;
	}

	return  map[ n_vrb ];
}
// Вычисление n PRB тильда штрих.
inline uint32_t VrbMap::CalcNprbTwiddle1( uint32_t n_row, uint32_t n_vrb_t, uint32_t n_dl_vrb_t, uint32_t n_vrb )
{
	return 2 * n_row * ( n_vrb_t % 2 ) + ( n_vrb_t / 2 ) + n_dl_vrb_t * ( n_vrb / n_dl_vrb_t );
}
// Вычисление n PRB тильда два штриха.
inline uint32_t VrbMap::CalcNprbTwiddle2( uint32_t n_row, uint32_t n_vrb_t, uint32_t n_dl_vrb_t, uint32_t n_vrb )
{
	return n_row * ( n_vrb_t % 4 ) + ( n_vrb_t / 4 ) + n_dl_vrb_t * ( n_vrb / n_dl_vrb_t ); //-V112
}

void VrbMap::MapEven( size_t nDlVrb, uint32_t rgb_size, Vrb2PrbMap& map )
{
	// Задаем размер карты
	map.resize(nDlRb);
	// Определяем количество строк в матрице перемежения
	uint32_t n_row  = static_cast<uint32_t>(ceilf( nDlVrb / ( 4.0f * rgb_size ) ) * rgb_size);
	// Определяем количество нулевых элементов в матрице перемежения
	uint32_t n_null = 4 * n_row - nDlVrb; //-V112
	// Вычисляем соответствие n PRB и n VRB.
	for ( uint32_t n_vrb = 0; n_vrb < nDlRb; ++n_vrb )	{
		uint32_t n_vrb_t = n_vrb % nDlVrb;
		if ( n_null )		{
			if ( n_vrb_t >= ( nDlVrb - n_null ) )			{
				switch ( n_vrb_t % 2 ) {
				case 0:
					map[ n_vrb ] = CalcNprbTwiddle1( n_row, n_vrb_t, nDlVrb, n_vrb ) - n_row + ( n_null / 2 );
					break;

				case 1:
					map[ n_vrb ] = CalcNprbTwiddle1( n_row, n_vrb_t, nDlVrb, n_vrb ) - n_row;
					break;
				}
				continue;
			}
			else			{
				if ( ( n_vrb_t % 4 ) >= 2 ) { // -V112
					map[ n_vrb ] = CalcNprbTwiddle2( n_row, n_vrb_t, nDlVrb, n_vrb ) - ( n_null / 2 );
					continue;
				}
			}
		}
		// otherwise
		map[ n_vrb ] = CalcNprbTwiddle2( n_row, n_vrb_t, nDlVrb, n_vrb );
	}
}

void VrbMap::MapOdd( size_t n_dl_vrb_t, const Vrb2PrbMap& evenMap, Vrb2PrbMap& map )
{
	// Задаем размер карты
	map.resize(nDlRb);
	// Вычисляем соответствие n PRB и n VRB.
	for ( uint32_t n_vrb = 0; n_vrb < nDlRb; ++n_vrb )
	{
		map[ n_vrb ] = ( evenMap[ n_vrb ] + ( n_dl_vrb_t / 2 ) ) % n_dl_vrb_t + n_dl_vrb_t * ( n_vrb / n_dl_vrb_t );
	}
}

void VrbMap::CorrectMap( uint32_t n_gap, uint32_t nDlVrb, Vrb2PrbMap& map )
{
	for ( uint32_t i = 0; i < nDlRb; ++i )
	{
		if ( map[ i ] >= ( nDlVrb / 2 ) )
			map[ i ] = ( map[ i ] + n_gap ) - ( nDlVrb / 2 );
	}
}

} // namespace LTE
