/**
 *  @file   lte_dci_support.cpp
 *  @author Dolgov Sergey
 *  @date   28/03/2012
 *  @brief  TS 36.212 5.3.3 Downlink control information.
 */
#include <vector>
#include <math.h>

#include "Lte/Core/Common/Crc.h"
#include "Lte/Core/Common/LteConst.h"
#include "Lte/Core/Common/LteSupport.h"
#include "Lte/Core/Common/DciSupport.h"

using namespace std;

namespace Lte
{
uint32_t lteGetType0ResAllocRbgSize( unsigned n_dl_rb )
{
	uint32_t p;
	if ( n_dl_rb <= 10 )
		p = 1;
	else
		if ( n_dl_rb <= 26 )
			p = 2;
		else
			if ( n_dl_rb <= 63 )
				p = 3;
			else
				p = 4; //-V112

	return p;
}
/* 3GPP TS 36.212 5.3.3.1.3A, Table 5.3.3.1.3A-1 */
uint32_t lteGetTPMILen( TxAntPorts vAntPorts )
{
	uint32_t l;
	switch ( vAntPorts ) {
		case lteTxAntPorts2: l = 2; break;
		case lteTxAntPorts4: l = 4; break; //-V112
	}

	return l;
}
// 3GPP 36.213 7.1.6.3, Table 7.1.6.3-1
uint32_t lteGetNrb_step( unsigned n_dl_rb )
{
  return ( n_dl_rb <= 49 ) ? 2 : 4; //-V112
}
// 3GPP TS 36.211 6.2.3.2
uint32_t lteGetNgap1(unsigned n_dl_rb)
{
	uint32_t Ngap1;
	if ( n_dl_rb <= 10 )
		Ngap1 = static_cast< uint32_t >( ceil(n_dl_rb / 2.0));
	else
		if (n_dl_rb == 11)
			Ngap1 = 4; //-V112
		else
			if (n_dl_rb <= 19)
				Ngap1 = 8;
			else
				if (n_dl_rb <= 26)
					Ngap1 = 12;
				else
					if (n_dl_rb <= 44)
						Ngap1 = 18;
					else
						if ( n_dl_rb <= 49 )
							Ngap1 = 27;
						else
							if ( n_dl_rb <= 63 )
								Ngap1 = 27;
							else
								if ( n_dl_rb <= 79 )
									Ngap1 = 32; //-V112
								else
									if ( n_dl_rb <= 110 )
										Ngap1 = 48;

	return Ngap1;
}
// 3GPP TS 36.211 6.2.3.2

uint32_t lteGetNgap2( unsigned n_dl_rb )
{
	uint32_t Ngap2;
	if ( n_dl_rb <= 63 )
		Ngap2 = 9;
		else
			if ( n_dl_rb <= 79 )
				Ngap2 = 16;
			else
				if ( n_dl_rb <= 110 )
					Ngap2 = 16;

	return Ngap2;
}
// 3GPP TS 36.211 6.2.3.2
uint32_t lteGetNvrbGap1( unsigned n_dl_rb )
{
	uint32_t  n_gap1 = lteGetNgap1( n_dl_rb );
	return static_cast<uint32_t>( 2 * min( n_gap1, n_dl_rb - n_gap1 ) );
}
// 3GPP TS 36.211 6.2.3.2
uint32_t lteGetNvrbGap2( unsigned n_dl_rb )
{
	uint32_t n_gap2 = lteGetNgap2( n_dl_rb );
	return static_cast<uint32_t>( 2 * n_gap2 * ( n_dl_rb / ( 2 * n_gap2 ) ) );
}
/**
 *  3GPP 36.212 Table 5.3.3.1.2-1: Ambiguous Sizes of Information Bits
 *  @param[in]    vLen   Information Bit Block Length
 *  @return       true   If the specified length is present in the list of invalid.
 */
bool lteCheckAmbiguousSize( uint32_t len )
{
	static const uint32_t AmbSizesLen           = 10;
	static const uint32_t AmbSizes[ AmbSizesLen ] = { 12, 14, 16 ,20, 24, 26, 32, 40, 44, 56 };

	bool amb = false;
	for ( uint32_t i = 0; !amb && ( i < AmbSizesLen ); ++i )
		amb = ( len == AmbSizes[ i]  );

	return amb;
}
/**
 *  Calculation of DCI lengths for all format options.
 *  @param[in/out]     Env    Environment configuration information.
 *  @param[out]        Len    DCI lengths for all format.
 */
void DefineDCIFormatLen( Environment& env, FormatLen& len )
{
	uint32_t rba_len;
	/* Format 0 3GPP 36.212 5.3.3.1.1  */
	rba_len             = CeilLog2((env.mUL_BandWidth * (env.mUL_BandWidth + 1)) / 2);
	len.mFormat0Len     = 1 + 1 + rba_len + 5 + 1 + 2 + 3 + 1 + ((env.mDuplex == lteFDD) ? 0 : 2);
	env.mFormat0_RBALen = rba_len;
	/* Format 1 3GPP 36.212 5.3.3.1.2  */
	rba_len             = (uint32_t)(ceil((double)env.mDL_BandWidth) / (double)lteGetType0ResAllocRbgSize(env.mDL_BandWidth));
	len.mFormat1Len     = 1 + rba_len + 5 + ((env.mDuplex == lteFDD) ? 3 : 4) + 1 + 2 + 2 + ((env.mDuplex == lteFDD) ? 0 : 2); //-V112
	env.mFormat1_RBALen = rba_len;
	/* Format 1A 3GPP 36.212 5.3.3.1.3 */
	rba_len              = CeilLog2((env.mDL_BandWidth * (env.mDL_BandWidth + 1)) / 2);
	len.mFormat1ALen     = 1 + 1 + rba_len + 5 + ((env.mDuplex == lteFDD) ? 3 : 4) + 1 + 2 + 2 + ((env.mDuplex == lteFDD) ? 0 : 2); //-V112
	env.mFormat1A_RBALen = rba_len;
	/* Format 1B 3GPP 36.212 5.3.3.1.3A*/
	if ( env.mAntPorts != lteTxAntPorts1 )	{
		rba_len              = CeilLog2((env.mDL_BandWidth * (env.mDL_BandWidth + 1)) / 2);
		len.mFormat1BLen     = 1 + rba_len + 5 + ((env.mDuplex == lteFDD) ? 3 : 4) + 1 + 2 + 2 + ((env.mDuplex == lteFDD) ? 0 : 2) + //-V112
						   lteGetTPMILen(env.mAntPorts) + 1;
		env.mFormat1B_RBALen = rba_len;
	}
	else	{
		len.mFormat1BLen     = 0;
		env.mFormat1B_RBALen = 0;
	}
	// Format 1C 3GPP 36.212 5.3.3.1.4
	uint32_t Nstep         = lteGetNrb_step(env.mDL_BandWidth);
	uint32_t Nvrb_gap1     = lteGetNvrbGap1(env.mDL_BandWidth);
	rba_len              = CeilLog2(((Nvrb_gap1 / Nstep) * (Nvrb_gap1 / Nstep + 1)) / 2);
	len.mFormat1CLen     = ((env.mDL_BandWidth < 50) ? 0 : 1) + rba_len + 5;
	env.mFormat1C_RBALen = rba_len;
	// Format 1D 3GPP 36.212 5.3.3.1.4A
	if (env.mAntPorts != lteTxAntPorts1)	{
		rba_len              = CeilLog2((env.mDL_BandWidth * (env.mDL_BandWidth + 1)) / 2);
		len.mFormat1DLen     = 1 + rba_len + 5 + ((env.mDuplex == lteFDD) ? 3 : 4) + 1 + 2 + 2 + ((env.mDuplex == lteFDD) ? 0 : 2) + //-V112
									lteGetTPMILen(env.mAntPorts) + 1;
		env.mFormat1D_RBALen = rba_len;
	}
	else	{
		len.mFormat1DLen     = 0;
		env.mFormat1D_RBALen = 0;
	}

	len.mFormat2Len  = 0;
	len.mFormat2ALen = 0;

	// Padding
	uint32_t l = max(len.mFormat0Len, len.mFormat1ALen);
	if ( lteCheckAmbiguousSize( l ) )		l++;
	len.mFormat0Len  = l;
	len.mFormat1ALen = l;

	while ( ( len.mFormat1Len == l ) || lteCheckAmbiguousSize( len.mFormat1Len ) )		len.mFormat1Len++;
	if ( lteCheckAmbiguousSize( len.mFormat1BLen ) )		len.mFormat1BLen++;
	if ( lteCheckAmbiguousSize( len.mFormat1DLen ) )		len.mFormat1DLen++;
	//	Blocks formats 0, 1A, 3, 3A must be the same size
	len.mFormat3Len  = len.mFormat0Len;
	len.mFormat3ALen = len.mFormat0Len;

}

bool ParseDCIFormat1A( const Environment& env, Context& cntx, DciFormat1A& dci )
{
	static const uint32_t LocalVRB = 0;

	cntx.ExtractField( dci.mFormatDiff, 1 );
	cntx.ExtractField( dci.mLocVRBAssignment, 1 );
	if ( dci.mLocVRBAssignment == LocalVRB )
		cntx.ExtractField( dci.mRBA, env.mFormat1A_RBALen );
	else 	{
		if ( env.mDL_BandWidth < 50 )
			cntx.ExtractField( dci.mRBA, env.mFormat1_RBALen );
		else	{
			cntx.ExtractField( dci.mRBA, env.mFormat1_RBALen );
			dci.mNgap.mPresent = true;
			cntx.ExtractField( dci.mNgap, 1 );
		}
	}
	cntx.ExtractField( dci.mMCS, 5 );
	cntx.ExtractField( dci.mHARQProc, ( env.mDuplex == lteFDD ) ? 3 : 4 ); //-V112
	cntx.ExtractField( dci.mNewDataInd, 1 );
	cntx.ExtractField( dci.mRV, 2 );
	cntx.ExtractField( dci.mTPC, 2 );
	dci.mDAI.mPresent = ( env.mDuplex == lteTDD );
	cntx.ExtractField( dci.mDAI, 2 );

	return !cntx.IsError( );
}

bool ParseDCIFormat1C( const Environment& env, Context& cntx, DciFormat1C& dci )
{
	dci.mNgap.mPresent = ( env.mDL_BandWidth >= 50 );
	cntx.ExtractField( dci.mNgap, 1 );
	cntx.ExtractField( dci.mRBA, env.mFormat1C_RBALen );
	cntx.ExtractField( dci.mTBSIndex, 5 );

	return !cntx.IsError( );
}

} // namespace LTE
