/**
 *  @file   lte_dci_support.h
 *  @author Dolgov Sergey
 *  @date   28/03/2012
 *  @brief  TS 36.212 5.3.3 Downlink control information.
 */
#ifndef LTE_DCI_SUPPORT_H
#define LTE_DCI_SUPPORT_H

#include "Lte/Core/Support/LteSupport.h"
#include <cstring>

namespace Lte
{

struct Environment
{
	Duplex     mDuplex;
	unsigned   mDL_BandWidth;
	unsigned   mUL_BandWidth;
	TxAntPorts mAntPorts;
	// Formats
	uint32_t   mFormat0_RBALen;
	uint32_t   mFormat1_RBALen;
	uint32_t   mFormat1A_RBALen;
	uint32_t   mFormat1B_RBALen;
	uint32_t   mFormat1C_RBALen;
	uint32_t   mFormat1D_RBALen;

	Environment( )
	{
		std::memset( this, 0, sizeof( *this ) );
	}
};

struct FormatLen
{
	uint32_t mFormat0Len;
	uint32_t mFormat1Len;
	uint32_t mFormat1ALen;
	uint32_t mFormat1BLen;
	uint32_t mFormat1CLen;
	uint32_t mFormat1DLen;
	uint32_t mFormat2Len;
	uint32_t mFormat2ALen;
	uint32_t mFormat3Len;
	uint32_t mFormat3ALen;

	FormatLen( )
	{
		std::memset( this, 0, sizeof( *this ) );
	}
};

struct Vector
{
	uint32_t   mLen;
	uint32_t   mBits;
};

struct Optional
{
	bool     mPresent;
	uint32_t   mBits;
};

struct DciFormat0
{
	uint32_t  	mFormatDiff;
	uint32_t   	mHopping;
	uint32_t   	mNprb;
	Vector   	mRBA;
	uint32_t  	mMCS;
	uint32_t   	mNewDataInd;
	uint32_t   	mTPC;
	uint32_t   	mCyclicShift;
	uint32_t   	mULIndex;
	Optional 	mDAI;
	uint32_t   	mCQIRequest;
};
/**
 *  @struct DciFormat1
 *  @brief  DCI, Format1
 */
struct DciFormat1
{
	uint32_t   	mRAH;
	Vector   	mRBA;
	uint32_t   	mShift;
	uint32_t   	mMCS;
	uint32_t   	mHARQProc;
	uint32_t   	mNewDataInd;
	uint32_t   	mRV;
	uint32_t   	mTPC;
	Optional 	mDAI;
};
/**
 *  @struct DciFormat1A
 *  @brief  DCI, Format1A
 */
struct DciFormat1A
{
	uint32_t   	mFormatDiff;
	uint32_t   	mLocVRBAssignment;
	Optional 	mNgap;
	Vector   	mRBA;
	bool     	mRACH;
	uint32_t   	mPreambleIndex;
	uint32_t   	mPRACHMaskIndex;
	uint32_t   	mMCS;
	uint32_t   	mHARQProc;
	uint32_t   	mNewDataInd;
	uint32_t   	mRV;
	uint32_t   	mTPC;
	Optional 	mDAI;
};
/**
 *  @struct DciFormat1B
 *  @brief  DCI, Format1B
 */
struct DciFormat1B
{
	uint32_t 	mLocVRBAssignment;
	Optional 	mNgap;
	Vector   	mRBA;
	uint32_t  	mMCS;
	uint32_t   	mHARQProc;
	uint32_t   	mNewDataInd;
	uint32_t   	mRV;
	uint32_t   	mTPC;
	Optional 	mDAI;
	Vector   	mTPMI;
	uint32_t   	mPMI;
};
/**
 *  @struct DciFormat1C
 *  @brief  DCI, Format1C
 */
struct DciFormat1C
{
	Optional 	mNgap;
	Vector   	mRBA;
	uint32_t   	mTBSIndex;
};
/**
 *  @struct DciFormat1D
 *  @brief  DCI, Format1D
 */
struct DciFormat1D
{
	uint32_t   	mLocVRBAssignment;
	Vector   	mRBA;
	Optional 	mNgap;
	uint32_t   	mMCS;
	uint32_t   	mHARQProc;
	uint32_t   	mNewDataInd;
	uint32_t   	mRV;
	uint32_t   	mTPC;
	Optional 	mDAI;
	Vector   	mTPMI;
	uint32_t   	mDLPowerOffset;
};
/**
 *  @struct DciFormat2
 *  @brief  DCI, Format2
 */
struct DciFormat2
{
	uint32_t 	mFill;
};
/**
 *  @struct DciFormat2A
 *  @brief  DCI, Format2A
 */
struct DciFormat2A
{
	uint32_t 	mFill;
};
/**
 *  @struct DciFormat3
 *  @brief  DCI, Format3
 */
struct DciFormat3
{
	uint32_t 	mFill;
};
/**
 *  @struct DciFormat3A
 *  @brief  DCI, Format3A
 */
struct DciFormat3A
{
	uint32_t 	mFill;
};
/**
 *  @enum  DciFormat
 *  @brief ������ Downlink Control Indicator.
 */
enum DciFormat
{
	dciFormat0,   /*! DCI Format 0  */
	dciFormat1,   /*! DCI Format 1  */
	dciFormat1A,  /*! DCI Format 1A */
	dciFormat1B,  /*! DCI Format 1B */
	dciFormat1C,  /*! DCI Format 1C */
	dciFormat1D,  /*! DCI Format 1D */
	dciFormat2,   /*! DCI Format 2  */
	dciFormat2A,  /*! DCI Format 2A */
	dciFormat3,   /*! DCI Format 3  */
	dciFormat3A   /*! DCI Format 3A */
};
/**
 *  @struct DciMessage
 *  @brief  Downlink Contol Indicator
 */
struct DciMessage
{
	Rnti      mRNTI;
	DciFormat mFormat;
	union {
		DciFormat0  mFormat0;
		DciFormat1  mFormat1;
		DciFormat1A mFormat1A;
		DciFormat1B mFormat1B;
		DciFormat1C mFormat1C;
		DciFormat1D mFormat1D;
		DciFormat2  mFormat2;
		DciFormat2A mFormat2A;
		DciFormat3  mFormat3;
		DciFormat3A mFormat3A;
	} VF;
};
/**
 *  @class Context
 *  @brief �������� ������� ���������.
 */
class Context
{
private:
	PBitVector* mPtr;
	uint32_t    mLen;
	uint32_t    mOffset;
	bool        mError;

	inline bool Check( uint32_t len )
	{
		if ( !mError )
			mError = ( ( mOffset + len ) > mLen );

	return mError;
	}
public:
	inline Context( PBitVector* bits, uint32_t len, uint32_t offset ) :
			mPtr( bits ), mLen( len ), mOffset( offset ), mError( false )
	{

	}

	inline Context( PBitVector* bits, uint32_t len) :
			mPtr( bits ), mLen( len ), mOffset( 0 ), mError( false )
	{

	}

	inline bool IsError( ) const
	{
		return mError;
	}

	inline void ExtractField( uint32_t& field, uint32_t len )
	{
		if ( Check( len ) )
			return;
		ExtractBitsRev( mPtr, &field, mOffset, len );
		mOffset += len;
	}

	inline void ExtractField( Vector& field, uint32_t len )
	{
		if ( Check( len ) )
			return;
		field.mLen = len;
		ExtractBitsRev( mPtr, &field.mBits, mOffset, len );
		mOffset += len;
	}

	inline void ExtractField( Optional& field, uint32_t len )
	{
		if ( !field.mPresent || Check( len ) )
			return;
		ExtractBitsRev( mPtr, &field.mBits, mOffset, len );
		mOffset += len;
	}
};
/**
 *  ������ Downlink Control Information Format 1A.
 *  @param[in]     env     ���������� � �����.
 *  @param[in]     cntx    �������� �������.
 *  @param[out]    dci     ��������� � ����������� ����������.
 *  @return        false � ������ ������ �������.
 */
bool ParseDCIFormat1A( const Environment& env, Context& cntx, DciFormat1A& dci );
/**
 *  ������ Downlink Control Information Format 1A.
 *  @param[in]     env     ���������� � �����.
 *  @param[in]     cntx    �������� �������.
 *  @param[out]    dci     ��������� � ����������� ����������.
 *  @return        false � ������ ������ �������.
 */
bool ParseDCIFormat1C( const Environment& env, Context& cntx, DciFormat1C& dci );
/**
 *  ������ ���� DCI ��� ���� ��������� ��������.
 *  @param[in]     env    ���������� � ������������ �����.
 *  @param[out]    len    ����� DCI ���� ��������.
 */
void DefineDCIFormatLen( Environment& env, FormatLen& len );
/**
 *  ���������� RBG size.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       RBG size.
 */
uint32_t lteGetType0ResAllocRbgSize( unsigned n_dl_rb );
/**
 *  ���������� Nrb_step.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       Nrb_step.
 */
uint32_t lteGetNrb_step( unsigned n_dl_rb );
/**
 *  ���������� Ngap1.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       Ngap1.
 */
uint32_t lteGetNgap1( unsigned n_dl_rb );
/**
 *  ���������� Ngap2.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       Ngap2.
 */
uint32_t lteGetNgap2( unsigned n_dl_rb );
/**
 *  ���������� Nvrb_gap1.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       Nvrb_gap1.
 */
uint32_t lteGetNvrbGap1( unsigned n_dl_rb );
/**
 *  ���������� Nvrb_gap2.
 *  @param[in]    n_dl_rb    ������ ������� (� ��������� ������).
 *  @return       Nvrb_gap2.
 */
uint32_t lteGetNvrbGap2( unsigned n_dl_rb );

} // namespace LTE

#endif
