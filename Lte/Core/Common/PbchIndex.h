/*
 * pbch_index.h
 *
 *  Created on: 04.07.2014
 *      Author: dblagov
 */

#ifndef LTE_PBCH_INDEX_H_
#define LTE_PBCH_INDEX_H_

#include "Common/LteTypes.h"

namespace Lte {

struct 	CellInfo;

/**
 * @class PbchIndex
 * Generates PBCH sample indices for each character, nCellId is used for generation
 */

class PbchIndex {
public:
    PbchIndex( );
    ~PbchIndex( );
    void Generate( CellInfo& cellInfo );
    inline Index& operator[ ]( uint32_t i )
    {
        return indexes[ i ];
    }

private:
    Index2D	indexes;
};

}


#endif /* LTE_PBCH_INDEX_H_ */
