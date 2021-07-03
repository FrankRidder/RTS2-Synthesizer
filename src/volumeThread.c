#include "volumeThread.h"
#include "ADSR.h"
#include "shared.h"
#include <stdlib.h>    // gives malloc

TASK volumeThread()
{
    ADSR* adsr = createADSR();

    // initialize settings
    setAttackRate(adsr,    .1 * SAMPLE_RATE );  // .1 second
    setDecayRate(adsr,     .3 * SAMPLE_RATE );
    setReleaseRate(adsr,    5 * SAMPLE_RATE );
    setSustainLevel(adsr,  .8 );
    
    // gate(&adsr, true);

    while(!end_tasks)
    {
        // for(int i = 1; i < buf_size; i++) {
        //     sample[i] *= global_volume * process(adsr);
        // }
    }
    free(adsr);
}