/*
 * lte_correlator.h
 *
 *  Created on: 17.02.2015
 *      Author: dblagov
 */

#ifndef LTE_CORRELATOR_H_
#define LTE_CORRELATOR_H_

#include "Common/SyncCode.h"
#include "Common/LteDemConst.h"
#include "Common/LteTypes.h"
#include "Common/CellInfo.h"

#include <stdint.h>
#include <memory>
#include <array>


namespace System {
	class DebugInfo;
}

namespace Lte {

class SssCorrelator;
class PssCorrelator;
class Filter;

class LteCorrelator {
public:

	LteCorrelator();
	~LteCorrelator( );

    void    SetThreshold( float threshold_ )	{	threshold = threshold_;	}
    void	ConfigureSearchParams( uint32_t searchDepth, uint32_t decimFactor );

	bool	Process( const Complex16* data, uint32_t step ); // process only one bts
	bool    ProcessThinCorr( const Complex16* data, uint32_t step, Cells& cellQue);

    Cells& 	GetSyncInfo( )
    {
        return syncInfo;
    }

private:
	static const uint32_t MaxCorrsCount = 6;
	static const uint32_t CorrCount = LTEFrameLength / 2;

	const uint32_t               clearPosDelta;

	float               threshold;
	std::array<std::vector<float>,SyncCode::PSS_COUNT>  pssCorrRes;

	uint32_t filterSize;
	uint32_t			pssPos;
	int32_t				framePos;
	uint32_t			resNid2;
	uint32_t			nCellId;
	uint32_t            decimFactor;

	Cells	   syncInfo;

	std::vector< ComplexFloat >	filteredData;
	std::shared_ptr<PssCorrelator> pssCorrelator;
	std::shared_ptr<SssCorrelator> sssCorrelator;
	std::shared_ptr<Filter>		filter;

	System::DebugInfo&		debug;

    void CalcSyncCorr( const ComplexFloat* data );
    void CalcSyncCorr( const ComplexFloat* data, const CellInfo& cellInfo );

	void ClearCorr( float* corr, uint32_t pos, uint32_t deltaPos, uint32_t size );

	inline uint32_t GetResNid( uint32_t nid1, uint32_t nid2 )
	{
		return nid1 * SyncCode::PSS_COUNT + nid2;
	}
};

}



#endif /* LTE_CORRELATOR_H_ */
