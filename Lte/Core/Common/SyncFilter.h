/*
 * filter.h
 *
 *  Created on: 25.02.2015
 *      Author: dblagov
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "Common/LteDemConst.h"
#include "Common/FilterAfr.h"
#include <stdint.h>
#include <vector>
#include <array>

namespace System{
    class DebugInfo;
}

namespace Lte {

class Filter {
public:
	Filter();
	virtual ~Filter( ){ }

    uint32_t GetFilterSize( ) const;
    void    SetDecimFactor(uint32_t decimFactor);
    void    Process( const Complex16* in, ComplexFloat* out, int32_t size, int32_t step );

private:
	std::vector< ComplexFloat >	iq;

	FilterAfr afr;
	uint32_t                    decimFactor;
	System::DebugInfo&          debug;
};

}



#endif /* FILTER_H_ */
