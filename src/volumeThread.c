#include "volumeThread.h"
#include "ADSR.h"
#include "shared.h"

TASK volumeThread()
{
    ADSR* adsr = createADSR();
    initADSR(adsr);

    // initialize settings
    setAttackRate(adsr,    .1 * SAMPLE_RATE );  // .1 second
    setDecayRate(adsr,     .3 * SAMPLE_RATE );
    setReleaseRate(adsr,    5 * SAMPLE_RATE );
    setSustainLevel(adsr,  .8 );
    
    // gate(&adsr, true);

    while(1)
    {

    }

}