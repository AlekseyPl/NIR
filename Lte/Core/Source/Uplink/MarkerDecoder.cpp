/*
 * MarkerDecoder.cpp
 *
 *  Created on: May 7, 2018
 *      Author: aplotnikov
 */

#include "Lte/Core/Uplink/MarkerDecoder.h"
#include "Lte/Core/Uplink/MarkerConst.h"
#include <Chip/Vcp.h>
#include <Common/Allocator.h>

namespace Lte {

MarkerDecoder::MarkerDecoder( ) :
	metric( Common::AllocatorHeapInt::Locate() ), metricSize( ( ConvCodeRate + 1 ) * ( PayloadCount + CodeTailLength ) ),
	vcp(Chip::VcpFactory::Create( Constraint, Chip::Vcp::rate3, Polynomial0, Polynomial1, Polynomial2, 0 ))
{
	metric.Allocate( metricSize );
	vcp->Init( PayloadCount );
}

MarkerDecoder::~MarkerDecoder( )
{
    metric.Free();
	Chip::VcpFactory::Release(vcp);
}

void MarkerDecoder::Process( const int8_t* in, uint32_t* out )
{
    vcp->GenerateBranchMetrics( in, metric.Ptr( ) );
    vcp->Decode( metric.Ptr( ), out );
}

}



