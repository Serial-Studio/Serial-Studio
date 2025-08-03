// This example tests the performance for detecting and decoding frames
// with the framegen64 and framesync64 objects.
// SEE ALSO: framesync64_example.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "liquid.h"
#define OUTPUT_FILENAME  "framesync64_performance_example.m"

// add noise to channel
void frame64_add_noise(float complex * _buf, float _SNRdB)
{
    float nstd = powf(10.0f, -_SNRdB/20.0f);
    nstd *= M_SQRT2; // scale noise to account for signal being over-sampled by 2
    unsigned int i;
    for (i=0; i<LIQUID_FRAME64_LEN; i++)
        _buf[i] += nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2;
}

int main(int argc, char*argv[])
{
    // create frame generator, synchronizer objects
    framegen64  fg = framegen64_create();
    framesync64 fs = framesync64_create(NULL,NULL);
    unsigned int min_errors =   50;
    unsigned int min_trials =  500;
    unsigned int max_trials = 5000;
    float per_target = 1e-2f;

    // create buffer for the frame samples
    float complex frame[LIQUID_FRAME64_LEN];
    float SNRdB = -3.0f;
    float per_0 =-1.0f, per_1 = -1.0f;
    float snr_0 = 0.0f, snr_1 =  0.0f;
    FILE* fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s: auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all; close all;\n");
    fprintf(fid,"SNR=[]; pdetect=[]; pvalid=[];\n");
    printf("# %8s %6s %6s %6s %12s\n", "SNR", "detect", "valid", "trials", "PER");
    while (SNRdB < 10.0f) {
        framesync64_reset_framedatastats(fs);
        unsigned int num_trials = 0, num_errors = 0;
        while (1) {
            unsigned int i;
            for (i=0; i<min_trials; i++) {
                // reset frame synchronizer
                framesync64_reset(fs);

                // generate the frame with random header and payload
                framegen64_execute(fg, NULL, NULL, frame);

                // add channel effects
                frame64_add_noise(frame, SNRdB);

                // synchronize/receive the frame
                framesync64_execute(fs, frame, LIQUID_FRAME64_LEN);
            }
            num_trials += min_trials;
            num_errors = num_trials - framesync64_get_framedatastats(fs).num_payloads_valid;
            if (num_errors >= min_errors)
                break;
            if (num_trials >= max_trials)
                break;
        }
        // print results
        framedatastats_s stats = framesync64_get_framedatastats(fs);
        float per = (float)(num_trials - stats.num_payloads_valid)/(float)num_trials;
        if      (per >= per_target) { per_0 = per; snr_0 = SNRdB; }
        else if (per_1 < 0.0f     ) { per_1 = per; snr_1 = SNRdB; }
        printf("  %8.3f %6u %6u %6u %12.4e\n",
            SNRdB,stats.num_frames_detected,stats.num_payloads_valid,num_trials,per);
        fprintf(fid,"SNR(end+1)=%g; pdetect(end+1)=%g; pvalid(end+1)=%g;\n",
                SNRdB,
                (float)stats.num_frames_detected / (float)num_trials,
                (float)stats.num_payloads_valid  / (float)num_trials);
        if (num_errors < min_errors)
            break;
        SNRdB += 0.5f;
    }
    float m = (logf(per_1) - logf(per_0)) / (snr_1 - snr_0);
    float snr_target = snr_0 + (logf(per_target) - logf(per_0)) / m;
    printf("per:{%12.4e,%12.4e}, snr:{%5.2f,%5.2f} => %.3f dB for PER %12.4e\n",
            per_0, per_1, snr_0, snr_1, snr_target, per_target);
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  semilogy(SNR, 1-pdetect+eps,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"  semilogy(SNR, 1-pvalid +eps,'-o', 'LineWidth',2, 'MarkerSize',2);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('Prob. of Error');\n");
    fprintf(fid,"legend('detect','decoding','location','northeast');\n");
    fprintf(fid,"axis([-6 10 1e-3 1]);\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    // clean up allocated objects
    framegen64_destroy(fg);
    framesync64_destroy(fs);
    return 0;
}

