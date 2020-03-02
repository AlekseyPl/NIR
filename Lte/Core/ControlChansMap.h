/*
 * control_chans_map.h
 *
 *  Created on: 22.08.2014
 *      Author: dblagov
 */

#ifndef LTE_CONTROL_CHANS_MAP_H_
#define LTE_CONTROL_CHANS_MAP_H_

#include "Lte/Core/Common/CellInfo.h"
#include "Lte/Core/Common/PcfichIndex.h"
#include "Lte/Core/Common/Quadruplet.h"

#include "Common/Buffer.h"

namespace Lte {

/**
 * 		����� ControlInfoMap ������� ���������� ������ ��� ����������� ������� PCFICH, PHICH � PDCCH
 */

class ControlChansMap {
public:
    ControlChansMap( );
    ~ControlChansMap( );

    void    SetCellInfo( CellInfo& cellInfo_)
    {
        cellInfo = cellInfo_;
    }

    void	InsertPhich( uint32_t subframe );
    void	InsertPcfich( PcfichIndex& pcfichIndex );
    void	InsertPdcch( );

    inline Quadruplets&		GetPhichQuad( )
    {
            return phichQuad;
    }

    inline Quadruplets&	GetPcfichQuad( )
    {
            return pcfichQuad;
    }

    inline Quadruplets&	GetPdcchQuad( )
    {
            return pdcchQuad;
    }

    inline QuadrupletsCnt GetPdcchQuadCnt( )
    {
            return pdcchQuadCnt;
    }

    inline uint32_t size( ) const
    {
            return map.size( );
    }

    inline	CfiVariant& operator [ ] ( uint32_t i )
    {
            return map[ i ];
    }

private:
    CfiVariants		map;

    uint32_t			pcfichPos[ CFI_QuadrupletsCount ];
    uint32_t			pcfichOffset;
    uint32_t			pcfichIndexOrder[ CFI_QuadrupletsCount ];

    Quadruplets 		pcfichQuad;
    Quadruplets 		phichQuad;
    Quadruplets  		pdcchQuad;
    QuadrupletsCnt               pdcchQuadCnt;
    Common::Buffer<uint32_t>    mapRegsCount[ CFI_Variants ];
    QuadrupletMaps              quadMap;

    uint32_t			mapSize;
    uint32_t			REG;
    CellInfo			cellInfo;
    PcfichIndex 		pcfichIndex;


    void				InsertPcfich( uint32_t variant );
    uint32_t			GetPhichGroup( );
    uint32_t 			GetPhichSymbol( uint32_t i, uint32_t m, uint32_t subframe );
    uint32_t			GetPhichIndexReg( uint32_t freeRegs, uint32_t n_div, uint32_t m, uint32_t i );
    uint32_t			GetPhichIndexRegPcfichSymb( uint32_t freeRegs, uint32_t n_div, uint32_t m, uint32_t i );
    uint32_t			GetPhichIndexRegDiv( uint32_t n0, uint32_t n1, uint32_t subframe );
    void				AllocatePdcch( );

    uint32_t			AllocatedRegs( uint32_t symbols );
    void				PrbMap( QuadrupletMaps& quadMap, uint32_t symbols );


    inline bool 	IsType2SpecSubframe( uint32_t subframe ) const
    {
    	return ( ( cellInfo.duplex == lteTDD )  && ( ( subframe == 1 ) || ( subframe == 6 ) ) );
    }

};

}



#endif /* CONTROL_INFO_MAP_H_ */
