	/**
 *  @file   lte_riv.h
 *  @author Dolgov Sergey
 *  @date   25/02/2014
 */
#ifndef LTE_RIV_H
#define LTE_RIV_H

#include "Common/LteTypes.h"

namespace Lte {

struct Riv
{
	uint32_t mRBstart;
	uint32_t mLcrbs;
};

using Rivs = std::vector<Riv>;
class RivStore {
public:
	RivStore( uint32_t n_dl_rb );
	~RivStore( );
	bool CheckLocalVrb( uint32_t riv );
	bool CheckDistrVrbGap1( uint32_t riv );
	bool CheckDistrVrbGap2( uint32_t riv );
	bool CheckDistrVrb( uint32_t riv, Gap gap );
	Riv& GetLocalVrb( uint32_t riv );
	Riv& GetDistrVrbGap1( uint32_t riv );
	Riv& GetDistrVrbGap2( uint32_t riv );
	Riv& GetDistrVrb( uint32_t riv, Gap gap );
private:

	Rivs      	  mLocal;
	Rivs	 	  mDistrGap1;
	Rivs      	  mDistrGap2;

	void GenerateRivs( uint32_t n_dl_rb, uint32_t n_step, Rivs& rivs );
};

} // namespace LTE

#endif
