/*
 * lte_correlator.h
 *
 *  Created on: 17.02.2015
 *      Author: dblagov
 */

#ifndef LTE_CORRELATOR_H_
#define LTE_CORRELATOR_H_

#include "SyncCode.h"
#include "Lte/Core/Support/LteDemConst.h"
#include "Lte/Core/Support/LteTypes.h"
#include "Lte/Core/Support/CellInfo.h"
#include <stdint.h>
#include <memory>
#include <array>

namespace System {
	class DebugInfo;
}

namespace Lte {

class SssCorrelator;
class SssCorrelatorSlow;
class PssCorrelator;
class Filter;

class LteCorrelator {
public:

	LteCorrelator();
	~LteCorrelator( );

    void    SetThreshold( float threshold_ )	{	threshold = threshold_;	}
    void	ConfigureSearchParams( uint32_t searchDepth, uint32_t decimFactor);
    void    ConfigureRdm( uint32_t rdmGap );

	bool	Process( const Complex16* data, uint32_t step );
	bool    ProcessCells( const Complex16* data, uint32_t step, Cells& cellQue);

    Cells& 	GetSyncInfo( )
    {
        return syncInfo;
    }

	RdmCells& GetRdmCellInfo()
	{
		return rdmCells;
	}

private:
	static const uint32_t CorrCount = LTEFrameLength / 2;

	float               threshold;

	uint32_t pssPos;
	uint32_t framePos;
	uint32_t nCellId;
	uint32_t decimFactor;

	uint32_t rdmGap;
	uint32_t rdmGapDiv16;
	bool isNeedRdm;
	RdmCells rdmCells;
	uint32_t rdmOffset;

	Cells	   syncInfo;

	std::vector<ComplexFloat>	filteredData;
	std::vector<ComplexFloat>   rdmSlice;
	std::vector<float> rdmCorr;
	std::shared_ptr<PssCorrelator> pssCorrelator;
	std::shared_ptr<SssCorrelator> sssCorrelator;
	std::shared_ptr<SssCorrelatorSlow> sssCorrelatorSlow;
	std::shared_ptr<Filter>		filter;

	System::DebugInfo&		debug;

	void CalcSyncCorr( const ComplexFloat* data );
	void CalcSyncCorr( const ComplexFloat* data, const CellInfo& cellInfo );

	void SetRdmCorr( uint32_t pssPos, uint32_t nid2 );

	inline uint32_t GetResNid( uint32_t nid1, uint32_t nid2 )
	{
		return nid1 * SyncCode::PSS_COUNT + nid2;
	}
};

}



#endif /* LTE_CORRELATOR_H_ */
