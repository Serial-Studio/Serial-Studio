// This example tests the performance for detecting and decoding frames
// with the dsssframe64gen and dsssframe64sync objects.
// SEE ALSO: dsssframe64sync_example.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME  "dsssframe64sync_performance_example.m"

// add noise to channel
void frame64_add_noise(float complex * _buf,
        unsigned int _buf_len, float _SNRdB)
{
    float nstd = powf(10.0f, -_SNRdB/20.0f);
    nstd *= M_SQRT2; // scale noise to account for signal being over-sampled by 2
    unsigned int i;
    for (i=0; i<_buf_len; i++)
        _buf[i] += nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2;
}

int main(int argc, char*argv[])
{
    // create frame generator, synchronizer objects
    dsssframe64gen  fg = dsssframe64gen_create();
    dsssframe64sync fs = dsssframe64sync_create(NULL,NULL);
    unsigned int min_errors =    5;
    unsigned int min_trials =   80;
    unsigned int max_trials = 1000;

    // create buffer for the frame samples
    unsigned int  frame_len = dsssframe64gen_get_frame_len(fg);
    float complex * frame = (float complex *)malloc(frame_len*sizeof(float complex));
    float SNRdB = -25.0f;
    FILE* fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s: auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"SNR=[]; pdetect=[]; pvalid=[];\n");
    printf("# %8s %6s (%7s) %6s (%7s) %6s\n", "SNR", "missed", "percent", "errors", "percent", "trials");
    fclose(fid);
    while (SNRdB <  -5.0f) {
        dsssframe64sync_reset_framedatastats(fs);
        unsigned int num_trials = 0, num_errors = 0;
        while (1) {
            unsigned int i;
            for (i=0; i<min_trials; i++) {
                // reset frame synchronizer
                dsssframe64sync_reset(fs);

                // generate the frame with random header and payload
                dsssframe64gen_execute(fg, NULL, NULL, frame);

                // add channel effects
                frame64_add_noise(frame, frame_len, SNRdB);

                // synchronize/receive the frame
                dsssframe64sync_execute(fs, frame, frame_len);
            }
            num_trials += min_trials;
            num_errors = num_trials - dsssframe64sync_get_framedatastats(fs).num_payloads_valid;
            if (num_errors >= min_errors)
                break;
            if (num_trials >= max_trials)
                break;
        }
        // print results
        framedatastats_s stats = dsssframe64sync_get_framedatastats(fs);
        unsigned int num_misses = num_trials - stats.num_frames_detected;
        float pmd = (float) num_misses / (float) num_trials;
        float per = (float) num_errors / (float) num_trials;
        printf("  %8.3f %6u (%6.2f%%) %6u (%6.2f%%) %6u\n",
            SNRdB,num_misses,pmd*100,num_errors,per*100,num_trials);
        fid = fopen(OUTPUT_FILENAME,"a");
        fprintf(fid,"SNR(end+1)=%g; pdetect(end+1)=%12g; pvalid(end+1)=%12g;\n",
                SNRdB,
                (float)stats.num_frames_detected / (float)num_trials,
                (float)stats.num_payloads_valid  / (float)num_trials);
        fclose(fid);
        if (num_errors < min_errors)
            break;
        SNRdB += 1.0f;
    }
    fid = fopen(OUTPUT_FILENAME,"a");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  semilogy(SNR, 1-pdetect+eps,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"  semilogy(SNR, 1-pvalid +eps,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('Prob. of Error');\n");
    fprintf(fid,"legend('detect','decoding','location','northeast');\n");
    fprintf(fid,"axis([-30 10 1e-3 1]);\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    // clean up allocated objects and memory blocks
    dsssframe64gen_destroy(fg);
    dsssframe64sync_destroy(fs);
    free(frame);
    return 0;
}
