/*
 * VbsMarker.h
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_VBSMARKER_H_
#define LTE_VBSMARKER_H_

#include "Lte/Core/Common/LteTypes.h"
#include "Lte/Core/Uplink/MarkerDeinterleaver.h"
#include "Lte/Core/Uplink/MarkerDecoder.h"
#include "Lte/Core/Uplink/MarkerCrc.h"
#include "Lte/Core/SoftConvDecoder.h"

namespace System {
	class	DebugInfo;
}

namespace Lte {

class VbsMarker {
public:
	struct MarkerInfo {
		uint32_t	subframeOffset;
		uint32_t	startRB;
		uint32_t	countRB;
	};

	VbsMarker( );
	virtual ~VbsMarker( );

	void        Reset( );
	bool		AddSymbol( const Complex16* sym );
	bool		Process( );
	MarkerInfo&	GetMarkerInfo( )
	{
	    return info;
	}

private:

	Symbols             data;
	Symbols             preamble;
	Symbols             afc;
#if 1
	std::vector<int8_t> demodData;
	std::vector<int8_t> deinterlData;
#else
    SoftDecisions       demodData;
    SoftDecisions       deinterlData;
#endif

	uint32_t			decodedData[ 2 ];
	uint32_t 			progress;


	MarkerDeinterleaver	deinterleaver;
	MarkerDecoder		decoder;
	MarkerCrc16			crc;
	MarkerInfo			info;

	SoftConvDecoder     convDec;
	System::DebugInfo&	debug;

	Complex16			CalcFreqError( Complex16* marker );
	void				CompensateFreqError( Complex16* marker, Complex16& freqError );
	void				CalcAFC( Complex16* marker, Complex16* afc );
	void				Equalize( Complex16* marker, Complex16* afc );
	void				DemodulateSoft( Complex16* marker, int8_t* demod );
	void				DemodulateHard( Complex16* marker, int8_t* demod );

	void				GeneratePreamble( );
	void				Descramble( uint32_t* inout );

	void				ParseInfo( const uint32_t* decoded, MarkerInfo& mInfo );
};

}


#endif /* VBSMARKER_H_ */
