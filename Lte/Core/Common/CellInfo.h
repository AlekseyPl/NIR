/**
 *  @file   cell_info.h
 *  @author Dolgov Sergey
 *  @date   25/02/2014
 */
#ifndef LTE_CELL_INFO_H_
#define LTE_CELL_INFO_H_

#include "LteTypes.h"
#include "LteSupport.h"
#include <queue>

namespace Lte {
/**
 *  @struct CellInfo
 */
struct CellInfo {
	uint32_t framePos;
	Duplex    		duplex;
	uint32_t   		nCellId;
	CyclicPrefix  	cp;
	TxAntPorts    	antPorts;
	uint32_t       	nDlRb;
	PhichDuration 	duration;
	PhichResource 	resource;
	BandWidth		bw;

	struct {
		uint32_t Mi;
	}	TDD_Config;

	inline uint32_t SymbolsPerSlot( ) const
	{
		return Lte::SymbolsPerSlot( cp );
	}

	inline bool RefSymbPresent( uint32_t symbol ) const
	{
		return Lte::RefSymbPresent( cp, antPorts, symbol );
	}

	inline bool RefSymbPresent( uint32_t antPort, uint32_t symbol) const
	{
		return Lte::RefSymbPresent( cp, antPort, symbol );
	}

	inline bool RefSymbPresent( TxAntPorts antPorts_, uint32_t symbol ) const
	{
		return Lte::RefSymbPresent( cp, antPorts_, symbol );
	}

	inline uint32_t RefSymbShift( uint32_t antPort, uint32_t slot, uint32_t symbol ) const
	{
		return Lte::RefSymbShift( nCellId, antPort, slot, symbol );
	}

	inline TxAntPorts PDCCHAntPorts( ) const
	{
		return Lte::PdcchAntPorts( antPorts );
	}

	inline uint32_t QuadrupletsPerPRB( uint32_t symbols) const
	{
		return Lte::QuadrupletsPerPRB( cp, antPorts, symbols );
	}
};

using Cells = std::vector<CellInfo>;
using CellQueue = std::queue<CellInfo>;

} // namespace LTE

#endif
