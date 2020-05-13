/*
 * estimator.h
 *
 *  Created on: 24.06.2014
 *      Author: dblagov
 */

#ifndef LTE_ESTIMATOR_H_
#define LTE_ESTIMATOR_H_

#include "Lte/Core/Support/LteTypes.h"
#include "Lte/Core/Support/LteDemConst.h"

namespace Lte {

class Estimator {
public:
	Estimator( BandWidth bw, CyclicPrefix cp, uint32_t nid_ );
	~Estimator( );

	void		GetRsInfo( Time& time, bool* rs, uint32_t* rsShift );
	uint32_t	EstimateH( Time& time, const ComplexFloat* iq, uint32_t len, uint32_t shift, ComplexFloat* h );
	float           GetRSRP( ) { return rsrp; }

private:
    static const uint32_t rs_cnt = 2 * N_dl_rb_max;

    std::vector<int32_t> x1;
    std::vector<int32_t> x2;


    uint32_t                    nid;
    uint32_t                    nDlRb;
    std::vector<FSymbols>       rs;
    RefSymbPos                  rsPos;
    float                       rsrp;
    float                       scale;
    void			GenerateRs( CyclicPrefix cp );
    uint32_t			GetRsShift( uint32_t antennaPort, uint32_t slot, uint32_t symbol );
    uint32_t 			ExtractRs( const ComplexFloat* iq, uint32_t len, uint32_t rsShift, ComplexFloat* rs_ );
    ComplexFloat*               GetRs(Time& time);

    static void 		GenerateX1( uint32_t start, size_t count, int32_t* x1 );
    static void 		GenerateX2( uint32_t start, size_t count, uint32_t init, int32_t* x2 );
};

}


#endif /* ESTIMATOR_H_ */
