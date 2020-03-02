/**
 *  @file   lte_transp_block.h
 *  @author Dolgov Sergey
 *  @date   25/02/2014
 */
#ifndef LTE_TRANSP_BLOCK_H
#define LTE_TRANSP_BLOCK_H

#include "Lte/Core/Common/LteTypes.h"
#include "Lte/Core/Common/DciSupport.h"
#include <vector>
#include <string.h>

namespace Lte
{

struct CellInfo;

struct TranspBlockParam {
	SoftDecisions		mSd;
        uint32_t		mSize;
	uint32_t      		mRb;
	uint32_t      		mMcs;
	bool          		mFormat1C;
	uint32_t      		mTbsIndex;
	Rv            		mRv;
	uint32_t      		mCinit;
	int32_t       		mSnr;

        inline TranspBlockParam( SoftDecision* sd, size_t len, int32_t snr )
	{
		Init( sd, len, 0, 0, false, 0, lteRV0, 0, snr );
	}


	inline TranspBlockParam( SoftDecision *sd, size_t len, uint32_t rb, uint32_t mcs,
                                                  bool format1C, uint32_t tbsIndex, Rv rv, uint32_t cInit, int32_t snr )
	{
		Init( sd, len, rb, mcs, format1C, tbsIndex, rv, cInit, snr );
	}

	inline void Init( SoftDecision *sd, uint32_t len, uint32_t rb, uint32_t mcs,
							bool format1C, uint32_t tbsIndex, Rv rv, uint32_t cInit, int32_t snr)
	{
                mSd.resize( len );
                memcpy( mSd.data(), sd, len * sizeof( SoftDecision ) );
		mSize     = len;
		mRb       = rb;
		mMcs      = mcs;
		mFormat1C = format1C;
		mTbsIndex = tbsIndex;
		mRv       = rv;
		mCinit    = cInit;
		mSnr      = snr;
	}

};

using PrbList = std::vector<uint32_t>;

/**
 *  @struct TrBlock
 *  @brief  ������������ ����.
 */
class TrBlock {
public:
	PrbList mEvenPrbList;
	PrbList mOddPrbList;

	TrBlock( Rnti rnti, Complex16* buf, uint32_t n_prb, const CellInfo& cell, Complex32* accumulator, uint32_t accumCntr );
	~TrBlock( );
	void SetFormat( const DciFormat1A& format );
	void SetFormat( const DciFormat1C& format );
	void AddRB( const Complex16* rb, PrbType type );
	void AddRB( const Complex16* rb, PrbType type, uint32_t rsShift );
	void AddRbMrc( const Complex16* rb, PrbType type );
	void AddRbMrc( const Complex16* rb, PrbType type, uint32_t rsShift );
	Rnti RNTI( ) const;
	TranspBlockParam GetTranspBlockParam( uint32_t sfn, uint32_t Ns );
	DciFormat	GetFormat( )
	{
		return mFormat;
	}
private:
    static const uint32_t Nsc_rb_half = Nsc_rb / 2;
    bool      		mInit;
    DciFormat 		mFormat;
    uint32_t  		mNcell_id;
    Rnti      		mRNTI;
    Complex16*		mpSymbols;
    Complex32*		mpAccum;
    uint32_t    	mNprb;
    uint32_t  		mRSStep;
    uint32_t    	mPos;
    uint32_t  		mMCS;
    uint32_t  		mTbsIndex;
    Rv        		mRV;
    uint32_t    	mAccumCount;

    uint32_t		Cinit( uint32_t Ns, uint32_t q );
    int32_t		ComputeSnr( );
    void     		Accumulate( const Complex16 *sc, size_t len );
};

using TrBlocks = std::vector<TrBlock>;


} // namespace LTE

#endif
