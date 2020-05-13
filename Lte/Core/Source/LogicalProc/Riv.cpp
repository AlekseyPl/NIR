/**
 *  @file   lte_riv.cpp
 *  @author Dolgov Sergey
 *  @date   25/02/2014
 */
#include "LogicalProc/Riv.h"
#include "LogicalProc/DciSupport.h"

namespace Lte {

RivStore::RivStore( uint32_t nDlRb )
{
	// 3GPP TS 36.213 7.1.6.3
	uint32_t n_step = lteGetNrb_step( nDlRb );

	// Generate RIV for local VRB
	GenerateRivs( nDlRb, 1, mLocal );

	// Generate RIV for distributed VRB (gap 1)
	GenerateRivs( lteGetNvrbGap1( nDlRb ), n_step, mDistrGap1 );
	// Generate RIV for distributed VRB (gap 2)
	if ( nDlRb >= 50 ) {
		GenerateRivs( lteGetNvrbGap2( nDlRb ), n_step, mDistrGap2 );
	}
}
RivStore::~RivStore( )
{

}

bool RivStore::CheckLocalVrb( uint32_t riv )
{
  return riv < mLocal.size();
}

bool RivStore::CheckDistrVrbGap1( uint32_t riv )
{
  return riv < mDistrGap1.size();
}

bool RivStore::CheckDistrVrbGap2( uint32_t riv )
{
  return riv < mDistrGap2.size( );
}

bool RivStore::CheckDistrVrb( uint32_t riv, Gap gap )
{
  return ( gap == gap1 ) ? CheckDistrVrbGap1( riv ) : CheckDistrVrbGap2( riv );
}

Riv& RivStore::GetLocalVrb( uint32_t riv )
{
  return mLocal[ riv ];
}

Riv& RivStore::GetDistrVrbGap1( uint32_t riv )
{
  return mDistrGap1[ riv ];
}

Riv& RivStore::GetDistrVrbGap2( uint32_t riv )
{
  return mDistrGap2[ riv ];
}

Riv& RivStore::GetDistrVrb( uint32_t riv, Gap gap )
{
  return ( gap == gap1 ) ? GetDistrVrbGap1( riv ) : GetDistrVrbGap2( riv );
}

void RivStore::GenerateRivs( uint32_t n_dl_rb, uint32_t n_step, Rivs& rivs )
{
	uint32_t n   = n_dl_rb / n_step;
	size_t len = ((1 + n) * n) / 2; // Размер = сумма первых N членов арифметической прогрессии
	rivs.resize(len);

	for ( uint32_t l = 1; l <= n; ++l )
		for ( uint32_t r = 0; r <= ( n - l ); ++r )		{
			size_t riv;
			if ( ( l - 1 ) <= ( n / 2 ) )	riv = n * ( l - 1 ) + r;
			else							riv = n * ( n - l + 1 ) + ( n - 1 - r );
			rivs[ riv ].mLcrbs   = l * n_step;
			rivs[ riv ].mRBstart = r * n_step;
		}
}

} // namespace LTE
