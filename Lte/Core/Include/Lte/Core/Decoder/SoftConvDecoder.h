/*
 * SoftConvDecoder.h
 *
 *  Created on: Sep 10, 2018
 *      Author: aplotnikov
 */

#ifndef INCLUDE_LTE_SOFTCONVDECODER_H_
#define INCLUDE_LTE_SOFTCONVDECODER_H_

#include "Support/LteConst.h"
#include "Support/LteTypes.h"


namespace Lte {

class SoftConvDecoder {
public:
    SoftConvDecoder( );
    ~SoftConvDecoder( );

    void    Process( const int16_t* in, uint32_t* out, uint32_t frameLen );

private:
    static const uint32_t   RegLen  = Constraint - 1;
    static const uint32_t   TrellisNodesCount = 1 << RegLen;

    struct tEncTrellisNode {
        uint8_t State0;
        uint8_t State1;
        uint8_t Out0_G0;
        uint8_t Out0_G1;
        uint8_t Out0_G2;
        uint8_t Out1_G0;
        uint8_t Out1_G1;
        uint8_t Out1_G2;
    };

    struct tDecTrellisNode {
        uint8_t State0;
        uint8_t State1;
        int32_t In0_G0;
        int32_t In0_G1;
        int32_t In0_G2;
        int32_t In1_G0;
        int32_t In1_G1;
        int32_t In1_G2;
    };

	std::vector<uint64_t>      trace;
	std::vector<int32_t>       weights;
    tEncTrellisNode gEncTrellisNode[ TrellisNodesCount ];
    tDecTrellisNode gDecTrellisNode[ TrellisNodesCount ];

    uint32_t    poly1;
    uint32_t    poly2;
    uint32_t    poly3;
    uint32_t    poly4;

    uint32_t    constraint;

    void        DecoderTrellis( );
    void        EncoderTrellis( );

    inline uint8_t TestBit( uint8_t r, uint32_t pos );
    inline uint8_t Polynome( uint8_t r, uint8_t poly, uint32_t K );
};


inline uint8_t SoftConvDecoder::TestBit( uint8_t r, uint32_t pos )
{
    return ( r >> pos ) & 1;
}

inline uint8_t SoftConvDecoder::Polynome( uint8_t r, uint8_t poly, uint32_t K )
{
    uint8_t res = 0;
    for ( uint32_t i = 0; i < K; ++i )
        if ( TestBit( poly, i ) ) res ^= TestBit( r, i );

    return res;
}

}



#endif /* INCLUDE_SOFTCONVDECODER_H_ */
