/*
 * VbsDetector.h
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#ifndef LTE_VBSDETECTOR_H_
#define LTE_VBSDETECTOR_H_

#include <stdint.h>
#include "Lte/Core/Uplink/VbsMarker.h"

namespace Lte {

struct CellInfo;

class VbsDetector  {
public:
	struct MarkerParams {
		uint32_t	subframe;
		uint32_t	slot;
	};

	VbsDetector( CellInfo& ci, MarkerParams& markerParams );
	virtual ~VbsDetector( );

	bool	Process( const Complex16* sym );
	VbsMarker::MarkerInfo& GetMarkerInfo( )
	{
		return marker.GetMarkerInfo( );
	}
	void	Reset( );

private:
	const uint32_t			markerSf;
	const uint32_t			markerSlot;

	VbsMarker				marker;
	uint32_t				markerOffset;
	CellInfo&				cellInfo;
	Time					time;
};

}



#endif /* VBSDETECTOR_H_ */
