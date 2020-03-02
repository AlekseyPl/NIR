/*
 * lte_types.cpp
 *
 *  Created on: 24.06.2014
 *      Author: dblagov
 */
#include "Common/LteTypes.h"
#include "Common/LteDemConst.h"
#include <algorithm>

namespace Lte {

RefSymbPos::RefSymbPos(const RefSymbPos& rs)
{
	*this = rs;
}

RefSymbPos::RefSymbPos( uint32_t rs0_0, uint32_t rs0_1, uint32_t rs1_0, uint32_t rs1_1, uint32_t rs2_0, uint32_t rs3_0 )
{
	m_RS0_0 = rs0_0;
	m_RS0_1 = rs0_1;
	m_RS1_0 = rs1_0;
	m_RS1_1 = rs1_1;
	m_RS2_0 = rs2_0;
	m_RS3_0 = rs3_0;
}

bool RefSymbPos::IsRefSymbolPresent( uint32_t antPort, uint32_t symb )
{
  bool p;
  switch ( antPort ) {
    case 0: p = (m_RS0_0 == symb) || (m_RS0_1 == symb); break;
    case 1: p = (m_RS1_0 == symb) || (m_RS1_1 == symb); break;
    case 2: p = (m_RS2_0 == symb); break;
    case 3: p = (m_RS3_0 == symb); break;
  }

  return p;
}

Time::Time( CyclicPrefix cp ) :
		sfn( 0 ), subframe( 0 ), slot( 0 ), symbol( 0 )
{
	if( cp == lteCP_Long ) symbolCount = LTESymbolsInSlotL;
	else if( cp == lteCP_Short ) symbolCount = LTESymbolsInSlotS;
}

Time::Time( const Time& t ):
	sfn( t.sfn ), subframe( t.subframe ), slot( t.slot ), symbol( t.symbol ),
	symbolCount( t.symbolCount )
{

}

void Time::Reset( CyclicPrefix cp )
{
	sfn 	 = 0;
	subframe = 0;
	slot 	 = 0;
	symbol 	 = 0;

	symbolCount = ( cp == lteCP_Long ) ? LTESymbolsInSlotL : LTESymbolsInSlotS;
}

Time& Time::operator++ ( )
{
	symbol++;
	if( symbol == symbolCount ) {
		symbol = 0;
		slot++;
		if( slot == LTESlotsInSubframe ) {
			slot = 0;
			subframe++;
			if( subframe == LTESubframeInFrame ) {
				subframe = 0;
				sfn++;
			}
		}
	}
	return *this;
}

void Time::NextSfn( )
{
	sfn = ( sfn + 1 ) % SFN_Max;
}


uint32_t Time::NextSubframe( )
{
	uint32_t tmp = subframe + 1;
	if( tmp == LTESubframeInFrame ) return 0;
	else return tmp;
}

void Time::CorrectSfn( int32_t sfnC )
{
	if( sfn < sfnC )	sfn = sfn + SFN_Max - sfnC;
	else 				sfn = sfn - sfnC;
}

M0M1Converter::M0M1Converter()
{
	FillMap();
}

int32_t M0M1Converter::GetNid1(const M0M1 &m0m1)
{
	auto ret = nid1Map.find(m0m1);
	return ( ret != nid1Map.end()) ? ret->second : -1;
}

void	M0M1Converter::FillMap()
{
	for( int32_t nid1 = 0; nid1 <= LTENid1Max; ++nid1 ) {
		int32_t	qs = nid1 / 30;
		int32_t q  = ( ( nid1 + qs *( qs + 1 ) /2 ) / 30 );
		int32_t ms = nid1 + q * ( q + 1 ) / 2;

		int32_t m0 = ms % LTESyncCodeHalfLen;
		int32_t m1 = ( m0 + ( ms / LTESyncCodeHalfLen ) + 1 ) % LTESyncCodeHalfLen;

		nid1Map.emplace(M0M1(m0,m1),nid1);
	}
}

}




