/**
 *  @file   lte_vrb.h
 *  @author Dolgov Sergey
 *  @date   24/02/2014
 */
#ifndef LTE_VRB_MAP_H
#define LTE_VRB_MAP_H

#include "Common/LteTypes.h"

namespace Lte {


/**
 *  @class VrbMap
 *  @brief Карта соответствия виртуальных ресурсных блоков физическим ресурсным блокам (3GPP TS 36.211 6.2.3.2).
 */
class VrbMap {
public:
	VrbMap( uint32_t nDlRb_ );
	~VrbMap( );

	size_t  MapPrb( uint32_t n_vrb, uint32_t n_s, Gap gap);
private:
    using Vrb2PrbMap = std::vector<size_t>;

    Vrb2PrbMap	mGap1Even;
    Vrb2PrbMap	mGap1Odd;
    Vrb2PrbMap	mGap2Even;
    Vrb2PrbMap	mGap2Odd;

    uint32_t		nDlRb;

    void 		MapEven( size_t n_dl_vrb_t, uint32_t rgb_size, Vrb2PrbMap& map );
    void 		MapOdd( size_t n_dl_vrb_t, const Vrb2PrbMap& evenMap, Vrb2PrbMap& map );
    void 		CorrectMap( uint32_t n_gap, uint32_t n_dl_vrb_t, Vrb2PrbMap& map );
    uint32_t  		CalcNprbTwiddle1( uint32_t n_row, uint32_t n_vrb_t, uint32_t n_dl_vrb_t, uint32_t n_vrb );
    uint32_t  		CalcNprbTwiddle2( uint32_t n_row, uint32_t n_vrb_t, uint32_t n_dl_vrb_t, uint32_t n_vrb );
};

} // namespace LTE

#endif
