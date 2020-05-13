/*
 * equalizer.h
 *
 *  Created on: 13.03.2015
 *      Author: dblagov
 */

#ifndef LTE_EQUALIZER_H_
#define LTE_EQUALIZER_H_

#include "Lte/Core/Support/LteTypes.h"
#include "Lte/Core/Support/LteDemConst.h"
#include <array>

namespace Lte {

class Equalizer {
public:
	Equalizer( TxAntPorts antPorts, uint32_t rbCount, CyclicPrefix cp );
	~Equalizer( );

	void 	EstimateSymbol( const ComplexFloat* h, uint32_t hSize, TxAntPort antPort, uint32_t rsShift_, int32_t symbolNum );
	void	EstimeSubframe( );

	void	Equalize1AntPort( const ComplexFloat* src, ComplexFloat* dst, int32_t sfNum );
	void	EqualizePrb2AntPort( const ComplexFloat* src, uint32_t prbIndex, bool rs, ComplexFloat* dst, int32_t sfSym );

	void	Normalize( ComplexFloat* src, Complex16* dst, uint32_t count );
	void    CalcRSSI( const ComplexFloat* symbol);

	void	SetScale( int16_t scale_ )  {   scale = scale_;     }
	float 	GetCarrierRSSI( )           {   return carrierRSSI; }
	float	GetRSSI( )                  {   return rssi;        }


private:
	static const int32_t SF_PILOT_SYM_COUNT = PilotSymbCnt * LTESlotsInSubframe;

	using Matrix = std::vector< FSymbols >;

	std::array<Matrix, MaxAntennaPorts>			afc;
	std::array<std::vector<int32_t>, MaxAntennaPorts>	rsShift;

	float			scale;

	int32_t 		symCount;
	uint32_t		scCount;
	uint32_t		rbCount;
	uint32_t		antCount;
	int32_t			sfPilotSym[ SF_PILOT_SYM_COUNT ];
	CyclicPrefix            cp;
	float			carrierRSSI;
	float			rssi;
	uint32_t		fft;

	ComplexFloat            Hdelta( ComplexFloat s1, ComplexFloat s2, float step = static_cast<float>(PilotStep));
	void 			InterpolateTime( std::vector<ComplexFloat>& h, uint32_t vRSShift );
	void 			InterpolateLeft( ComplexFloat *v, uint32_t len, ComplexFloat delta );
	void 			InterpolateRight( ComplexFloat *v, uint32_t len, ComplexFloat delta );
	void			InterpolateFreq( Matrix& h );
	void 			Alamouti2x1( const ComplexFloat& x1, const ComplexFloat& x2, const ComplexFloat& h1,
                                             const ComplexFloat& h2, ComplexFloat& r1, ComplexFloat& r2 );
};

}



#endif /* EQUALIZER_H_ */
