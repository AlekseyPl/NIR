/*
 * rs_shift.h
 *
 *  Created on: 04.07.2014
 *      Author: dblagov
 */

#ifndef LTE_RS_SHIFT_H_
#define LTE_RS_SHIFT_H_

#include "LteTypes.h"
#include "LteDemConst.h"
#include <vector>
#include <array>

namespace Lte {

struct CellInfo;

static const uint32_t InvRSShift = static_cast< uint32_t >( -1 );

uint32_t	GetRSShift( CellInfo& cellInfo, TxAntPorts antPorts, uint32_t slot, uint32_t symbol );

class RsIndex {
public:
	RsIndex();
	~RsIndex() {}

	void Generate(uint32_t nCellId, uint32_t nDlRb);

	inline std::vector<uint32_t>& operator [ ]( uint32_t symb )
	{
		return index[ symb ];
	}
private:
	const uint32_t rsCount;
	uint32_t modCID;
	std::array<std::vector<uint32_t>, PilotSymbCnt> index;
};

}


#endif /* LTE_RS_SHIFT_H_ */
