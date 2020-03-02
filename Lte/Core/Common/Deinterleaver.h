/*
 * Deinterleaver.h
 *
 *  Created on: May 15, 2017
 *      Author: aplotnikov
 */

#ifndef INCLUDE_LTE_DEINTERLEAVER_H_
#define INCLUDE_LTE_DEINTERLEAVER_H_

#include "Lte/Core/Common/LteSupport.h"
#include <array>

namespace Lte {

struct Quadruplet;

class Deinterleaver {
public:
    Deinterleaver( );


    template<typename T>
    uint32_t	CcSubBlockDeinterleaver( const T *src, uint32_t len, T *dst, uint32_t offset, const SbInfo& info );

    uint32_t	CtcSubBlockDeinterleaver1( const float *src, float *dst, const SbInfo& info );
    uint32_t	CtcSubBlockDeinterleaver2( const float *src, float *dst, const SbInfo& info );

    /**
     *  �������������� ������������ PDCCH.
     *  @param[in]    src          ��������� �� �������� ���� ������������.
     *  @param[in]    len          ����� ��������� �����.
     *  @param[in]    n_cell_id    Ncell_id.
     *  @param[in]    dst          ��������� �� �������������� ���� ������������.
     */

    void                PdcchQuadDeinterleaver( const Quadruplet* src, uint32_t len, uint32_t nCellId, Quadruplet* dst );
    static void         SubBlockInterleaverInfo( uint32_t size, SbInfo& info );

    void 		InterleaveFillerBits1( float *dk, uint32_t fillerBits, const SbInfo& info  );
    void 		InterleaveFillerBits2( float *dk, uint32_t fillerBits, const SbInfo& info  );
private:
    static  const uint32_t	C_tc = 32;

    std::array<uint32_t,C_tc> gCcPermPattern;
    std::array<uint32_t,C_tc> gCtcPermPattern;
};

}

#endif
