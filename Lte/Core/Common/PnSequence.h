
#ifndef LTE_PNSEQUENCE_H
#define LTE_PNSEQUENCE_H

#include "Lte/Core/Common/LteTypes.h"

namespace Lte
{

void PnSeqInit( uint32_t cInit, uint32_t* cx1Init, uint32_t* cx2Init );
void Scrambler( uint32_t cx1Init, uint32_t cx2Init, SoftDecision *sd, uint32_t len, uint32_t skipLen = 0 );

} // namespace LTE


#endif
