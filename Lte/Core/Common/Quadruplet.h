#ifndef LTE_QUADRUPLET_H
#define LTE_QUADRUPLET_H

#include "Lte/Core/Common/LteTypes.h"
#include "Common/Buffer.h"
#include "Common/Allocator.h"

namespace Lte {

/**
 *  @struct tQuadruplet
 *  @brief  Quadruplet.
 */
struct Quadruplet
{
	Complex16 Q4[ QuadrupletSize ];
};


struct Quadruplet32
{
	Complex32 Q4[ QuadrupletSize ];
};
/**
 *  @struct tQuadrupletMap
 *  @brief  Quadruplet position
 */
struct QuadrupletMap
{
	uint16_t REG;
	uint8_t  REGsInSymbol;
	uint8_t  Symbol;
	/**
        *  vSymbol          OFDM-symbol.
	*  vREG             Resource Element Group.
        *  vREGsInSymbol    The number of REGs in the OFDM symbol.
	*/
        inline QuadrupletMap( uint32_t symbol, uint32_t reg, uint32_t regsInSymbol ) :
                        Symbol( symbol ), REG( reg ), REGsInSymbol( regsInSymbol ) { }

	bool operator < ( const QuadrupletMap& map ) const
	{
		return ( ( REG == map.REG ) ? ( Symbol < map.Symbol ) : ( REG < map.REG ) );
	}
};

using QuadrupletsCnt = std::vector<uint32_t>;
using QuadrupletPtrs = std::vector<Quadruplet*>;
using Quadruplets    = std::vector<Quadruplet>;
using QuadrupletMaps = std::vector<QuadrupletMap>;

using CfiVariant     = std::vector<QuadrupletPtrs>;
using CfiVariants    = std::vector< CfiVariant>;



} // namespace LTE


#endif
