/*
 * pbch.h
 *
 *  Created on: 26.06.2014
 *      Author: dblagov
 */

#ifndef LTE_PBCH_H_
#define LTE_PBCH_H_

#include "Lte/Core/Common/LteSupport.h"
#include <memory>
#include <array>

namespace Lte {

struct 	CellInfo;
class 	Decoder;

class Pbch {
public:
	Pbch( std::shared_ptr<Decoder>	decoder );
	~Pbch( ) { }
	void	InitCellInfo( CellInfo& cellInfo );
	bool 	Process( const Complex16* dec, uint32_t len, CellInfo& cellInfo, Time& time, uint32_t frame );

private:
    static const uint32_t PhyBlockLenNormCPInBits = 1920;
    static const uint32_t PhyBlockLenExtCPInBits  = 1728;
	static const uint32_t MibSize = BCH_MIB_Size / BitsInUInt8;

    uint32_t       	m_CX1init;
    uint32_t        m_CX2init;

	SoftDecisions sbits;
	std::array<uint8_t, MibSize> mib;
	std::shared_ptr<Decoder>	decoder;
};

}


#endif /* PBCH_H_ */
