/*
 * pcfich_index.h
 *
 *  Created on: 09.07.2014
 *      Author: dblagov
 */

#ifndef LTE_PCFICH_INDEX_H_
#define LTE_PCFICH_INDEX_H_

#include "Lte/Core/Common/LteTypes.h"
#include "Lte/Core/Common/CellInfo.h"

namespace Lte {

class PcfichIndex {
public:
    PcfichIndex( );
    ~PcfichIndex( );
    void    Generate( CellInfo& cellInfo );
    inline uint32_t operator [ ]( uint32_t i )
    {
            return index[ i ];
    }

private:
    Index	index;
};

}


#endif /* LTE_PCFICH_INDEX_H_ */
