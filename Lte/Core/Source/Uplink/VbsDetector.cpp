/*
 * VbsDetector.cpp
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */
#include "Lte/Core/Common/CellInfo.h"
#include "Lte/Core/Uplink/VbsDetector.h"
#include "Lte/Core/Uplink/MarkerConst.h"

namespace Lte {

VbsDetector::VbsDetector( CellInfo& ci, MarkerParams& markerParams ) :
	time( ci.cp ), markerSlot( markerParams.slot ), markerSf( markerParams.subframe ),
	cellInfo( ci )
{
	markerOffset = ( LTEFFTLen_20_MHz / 2 ) + ( cellInfo.nDlRb * Nsc_rb ) / 2 - Nsc_rb ;
}

VbsDetector::~VbsDetector( )
{

}

bool VbsDetector::Process( const Complex16* sym )
{
	bool res = false;

	if( /*( time.subframe == markerSf ) && */( time.slot == markerSlot ) ) {
		const Complex16* markerSym = sym + markerOffset;
		if( marker.AddSymbol( markerSym ) ){
			res = marker.Process( );
			marker.Reset( );
		}
	}

	++time;
	return res;
}

void VbsDetector::Reset( )
{
	time.Reset( );
    marker.Reset( );

}

}



