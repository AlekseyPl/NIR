/**
 *  @file   lte_transp_block.cpp
 *  @author Dolgov Sergey
 *  @date   25/02/2014
 */
#include "Lte/Core/Common/TranspBlock.h"
#include "Lte/Core/Common/CellInfo.h"

using namespace std;

namespace Lte
{

/************************************************************************/
/*                    ���������� ������ TrBlock                         */
/************************************************************************/

TrBlock::TrBlock( Rnti rnti, Complex16* buf, uint32_t n_prb, const CellInfo& cell,
											Complex32* accumulator, uint32_t accumCntr ) :
	mInit( false ),
	mRNTI( rnti),
	mpSymbols( buf ),
	mNprb( n_prb ),
	mPos( 0 ),
	mNcell_id( cell.nCellId ),
	mpAccum( accumulator),
	mAccumCount( accumCntr )
{
	mRSStep = ( cell.antPorts == lteTxAntPorts1) ? 6 : 3;
}

TrBlock::~TrBlock( )
{

}

void TrBlock::SetFormat( const DciFormat1A& format )
{
	mMCS   = format.mMCS;
	mRV    = Rv( format.mRV );
	// 3GPP 36.212 5.3.3.1.3
	if ( ( mRNTI == SI_RNTI ) || ( mRNTI == P_RNTI ) || ( ( mRNTI >= RA_RNTI_Min ) && ( mRNTI <= RA_RNTI_Max ) ) ) {
		mNprb = (format.mTPC & 1) ? 3 : 2;
	}
	mTbsIndex = 0;
	mInit   = true;
	mFormat = dciFormat1A;
}

void TrBlock::SetFormat( const DciFormat1C& format )
{
	// Redundancy version (TS 36.213 7.1.7.3)
	if ( ( mRNTI == P_RNTI ) || ( ( mRNTI >= RA_RNTI_Min ) && ( mRNTI <= RA_RNTI_Max ) ) ) {
		mRV = lteRV0;
	}
	else { //-V523
	// TS 36.321 5.3.1
		mRV = lteRV0; // TODO: RV ������� �� SFN
	}
	mTbsIndex = format.mTBSIndex;

	mInit   = true;
	mFormat = dciFormat1C;
}

uint32_t TrBlock::Cinit( uint32_t Ns, uint32_t q )
{
	return( static_cast< uint32_t >( mRNTI ) << 14 ) +  (q << 13 ) + ( ( Ns / 2 ) << 9 ) + mNcell_id;
}

int32_t TrBlock::ComputeSnr()
{
	/*
	if (mPos < 2)
	{
	return 0;
	}
	// Mean
	float m_re = 0, m_im = 0;
	for (uint32_t i = 0; i < mPos; ++i)
	{
	m_re += abs(mpSymbols[i].re);
	m_im += abs(mpSymbols[i].im);
	}
	m_re = m_re / (float)mPos;
	m_im = m_im / (float)mPos;
	// Std dev
	float s_re = 0, s_im = 0;
	for (uint32_t i = 0; i < mPos; ++i)
	{
	float t;
	t = (float)abs(mpSymbols[i].re) - m_re;
	s_re += t * t;
	t = (float)abs(mpSymbols[i].im) - m_im;
	s_im += t * t;
	}
	s_re = sqrt(s_re / (mPos - 1));
	s_im = sqrt(s_im / (mPos - 1));
	*/
	// Signal-to-noise ratio
	float snr = 0.0;
	/*
	try
	{
	snr = 20 * log10(((m_re + m_im) / (s_re + s_im)));
	}
	catch (...)
	{
	snr = 0;
	}
	*/
	return (int32_t)(snr + 0.5f);
}

void TrBlock::AddRB( const Complex16* rb,  PrbType type )
{
	Complex16* p = &mpSymbols[ mPos ];
	switch ( type ) {
	case prbPdsch:
		memcpy( p, rb, Nsc_rb * sizeof( Complex16 ) );
		mPos += Nsc_rb;
		break;

	case prbPdschLeftHalf:
		memcpy( p, rb, Nsc_rb_half * sizeof( Complex16 ) );
		mPos += Nsc_rb_half;
		break;

	case prbPdschRightHalf:
		memcpy( p, &rb[ Nsc_rb_half ], Nsc_rb_half * sizeof( Complex16 ) );
		mPos += Nsc_rb_half;
		break;
	}
}

void TrBlock::AddRB( const Complex16* rb, PrbType type, uint32_t rsShift )
{
	uint32_t len = ( uint32_t )( Nsc_rb - Nsc_rb / mRSStep );
	Complex16* p = &mpSymbols[ mPos ];

	uint32_t count;
	switch ( type ) {
	case prbPdsch:
		count = Nsc_rb;
		break;

	case prbPdschLeftHalf:
		count = Nsc_rb_half;
		len  /= 2;
		break;

	case prbPdschRightHalf:
		count = Nsc_rb_half;
		rb   += Nsc_rb_half;
		len  /= 2;
		break;
	}

	for ( uint32_t i = 0; i < count / mRSStep; ++i ) {
		for ( uint32_t j = 0; j < mRSStep; ++j, ++rb ) {
			if ( j != rsShift ) *p++ = *rb;
		}
	}
	mPos += len;
}

Rnti TrBlock::RNTI( ) const
{
	return mRNTI;
}

TranspBlockParam TrBlock::GetTranspBlockParam( uint32_t sfn, uint32_t Ns )
{
	uint32_t  len = mPos * 2;
	int32_t snr = ( mRNTI == SI_RNTI ) ? ComputeSnr( ) : 0;
	mPos = 0;
	SoftDecision *sd = reinterpret_cast< SoftDecision* >( mpSymbols );

	if ( ( mFormat == dciFormat1C ) && ( mRNTI == SI_RNTI ) ) {
		// Only for read System Information Type 1! TS 36.321 5.3.1
		const Rv rv[ ] = { lteRV0, lteRV2, lteRV3, lteRV1 };
		mRV = rv[ ( sfn / 2 ) % 4 ];
	}
	return TranspBlockParam( sd, len, mNprb, mMCS, ( mFormat == dciFormat1C ), mTbsIndex, mRV, Cinit( Ns, 0 ), snr );
}

void TrBlock::Accumulate( const Complex16 *sc, size_t len )
{
	Complex16* sc16 = &mpSymbols[ mPos ];
	Complex32* sc32 = &mpAccum[ mPos ];
	//ippsAdd_16s32s_I((const Ipp16s*)sc, (Ipp32s*)sc32, 2 * len);

	for( uint32_t i = 0; i < 2 * len; ++i ) {
		sc32[ i ].re += sc[ i ].re;
		sc32[ i ].im += sc[ i ].im;
	}

	for ( uint32_t i = 0; i < len; ++i ) {
		sc16[ i ].re = ( sc32[ i ].re / mAccumCount );
		sc16[ i ].im = ( sc32[ i ].im / mAccumCount );
	}
	mPos += len;
}

void TrBlock::AddRbMrc( const Complex16 *rb,  PrbType type )
{
	//assert(mInit && mpAccum);
	//Complex16* p = &mpSymbols[ mPos ];
	switch (type) {
	case prbPdsch:
		Accumulate( rb, Nsc_rb );
		break;

	case prbPdschLeftHalf:
		Accumulate( rb, Nsc_rb_half );
		break;

	case prbPdschRightHalf:
		Accumulate( &rb[ Nsc_rb_half ], Nsc_rb_half );
		break;
	}
}

void TrBlock::AddRbMrc( const Complex16 *rb, PrbType type, uint32_t rsShift )
{
	//assert(mInit && mpAccum);
	uint32_t len = ( uint32_t )( Nsc_rb - Nsc_rb / mRSStep );
	//Complex16* p = &mpSymbols[ mPos ];

	uint32_t count;
	switch ( type ) {
	case prbPdsch:
		count = Nsc_rb;
		break;

	case prbPdschLeftHalf:
		count = Nsc_rb_half;
		len  /= 2;
		break;

	case prbPdschRightHalf:
		count = Nsc_rb_half;
		rb   += Nsc_rb_half;
		len  /= 2;
		break;
	}

	uint32_t  sc_pos = 0;
	Complex16 sc[ Nsc_rb ];
	for ( uint32_t i = 0; i < count / mRSStep; ++i ) {
		for ( uint32_t j = 0; j < mRSStep; ++j, ++rb ) {
			if ( j != rsShift ) sc[ sc_pos++ ] = *rb;
		}
	}
	// Maximum ratio combining
	Accumulate( sc, len );
}

} // namespace LTE
