/*
 * Decoder.h
 *
 *  Created on: Sep 13, 2016
 *      Author: aplotnikov
 */

#ifndef LTE_DECODER_H_
#define LTE_DECODER_H_

#include "Common/LteTypes.h"
#include "Common/TranspBlock.h"
#include "Common/Crc.h"
#include <string>
#include <memory>

namespace Chip {
    class Tcp;
    class Vcp;
}

namespace System {
    class DebugInfo;
}


namespace Lte {

class RateRecovery;
class SoftConvDecoder;
class SoftTurboDecoder;

class Decoder {
public:
	Decoder();
    ~Decoder( );

    bool    TurboDecodeSCH( SoftDecision *data, CtcRmParams& rm, uint32_t fillerBits, CrcType crcType );
    bool    ViterbiDecodeBCH( SoftDecision* data, uint32_t srcLen, uint32_t dstLen, TxAntPorts& exPorts );
    bool    ViterbiDecodeCCH( SoftDecision* data, uint32_t srcLen, uint32_t dstLen );

    void    GetBits( uint8_t* ptr, uint32_t msgLength )
    {
        std::memcpy( ptr, bits.data( ), msgLength );
    }

private:
// 	Chip::Tcp*		 	tcp;
    Chip::Vcp*			vcp;

    uint32_t                            frameLength;

    std::unique_ptr<SoftConvDecoder>    convDec;
    std::unique_ptr<SoftTurboDecoder>   turboDec;
    std::unique_ptr<RateRecovery>       rateRecovery;

    std::vector<uint8_t>	 	bits;
    std::vector<uint32_t>               hd;
    std::vector<int8_t>			bm;
    std::vector<int8_t>			encoded;
    std::vector<SoftDecision>           encoded16;
    std::vector<uint16_t>               ilTable;
    std::vector<float>                  encodedf;
    System::DebugInfo&                  debug;

    void		GenInterleaverTable( );
    uint32_t            ExtractCrc( uint32_t msgLength, uint32_t crcLength );
    void                ExtractMsg( uint32_t msgLength);
};

}


#endif /* TURBODECODER_H_ */
