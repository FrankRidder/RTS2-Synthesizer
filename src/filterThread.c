#include "filterThread.h"
#include "filter.h"
#include "shared.h"

TASK filterThread()
{
    BWLowPass* filter_bw = create_bw_low_pass_filter(4, SAMPLE_RATE, filter_freq);

    while (1) 
    {
        // for(int i = 1; i < buf_size; i++) {
        //     filtered_samples[i] = bw_low_pass(filter_bw, samples[i] * 10);
        // }
    }

    free_bw_low_pass(filter_bw);
}