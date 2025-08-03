//
// agc_crcf_example.c
//
// Automatic gain control example demonstrating its transient
// response.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "agc_crcf_squelch_example.m"

// print usage/help message
void usage()
{
    printf("agc_crcf_squelch_example [options]\n");
    printf("  -h             : print usage\n");
    printf("  -b <bandwidth> : AGC bandwidth, b >=  0, default: 0.25\n");
}


int main(int argc, char*argv[])
{
    // options
    float bt = 0.25f;   // agc loop bandwidth

    int dopt;
    while((dopt = getopt(argc,argv,"hn:N:s:b:")) != EOF){
        switch (dopt) {
        case 'h': usage();           return 0;
        case 'b': bt = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (bt < 0.0f) {
        fprintf(stderr,"error: %s, bandwidth must be positive\n", argv[0]);
        exit(1);
    }
    unsigned int i;

    // create agc object, set loop bandwidth, and initialize parameters
    agc_crcf q = agc_crcf_create();
    agc_crcf_set_bandwidth(q, bt);
    agc_crcf_set_signal_level(q,1e-3f);     // initial guess at starting signal level

    // initialize squelch functionality
    agc_crcf_squelch_enable(q);             // enable squelch
    agc_crcf_squelch_set_threshold(q, -50); // threshold for detection [dB]
    agc_crcf_squelch_set_timeout  (q, 100); // timeout for hysteresis

    // initialize arrays
    unsigned int  num_samples = 2000;       // total number of samples to run
    float complex x   [num_samples];        // input
    float complex y   [num_samples];        // output
    float         rssi[num_samples];        // received signal strength
    int           mode[num_samples];        // squelch mode

    // print info
    printf("automatic gain control // loop bandwidth: %4.2e\n",bt);

    // generate signal, applying tapering window appropriately
    for (i=0; i<num_samples; i++) {
        float gamma = 0.0f;
        if      (i <  500) gamma = 1e-3f;
        else if (i <  550) gamma = 1e-3f + (1e-2f - 1e-3f)*(0.5f - 0.5f*cosf(M_PI*(float)(i- 500)/50.0f));
        else if (i < 1450) gamma = 1e-2f;
        else if (i < 1500) gamma = 1e-3f + (1e-2f - 1e-3f)*(0.5f + 0.5f*cosf(M_PI*(float)(i-1450)/50.0f));
        else               gamma = 1e-3f;

        // apply window to signal
        x[i] = gamma * cexpf(_Complex_I*2*M_PI*0.0193f*i);
    }

    // run agc
    for (i=0; i<num_samples; i++) {
        // apply gain
        agc_crcf_execute(q, x[i], &y[i]);

        // retrieve signal level [dB]
        rssi[i] = agc_crcf_get_rssi(q);

        // get squelch mode
        mode[i] = agc_crcf_squelch_get_status(q);

        // print status every so often
        if ( (i % 100)==0 || i==num_samples-1 || mode[i] == LIQUID_AGC_SQUELCH_RISE || mode[i] == LIQUID_AGC_SQUELCH_FALL) {
            char mode_str[20] = "";
            switch (mode[i]) {
            case LIQUID_AGC_SQUELCH_ENABLED:  sprintf(mode_str,"squelch enabled");  break;
            case LIQUID_AGC_SQUELCH_RISE:     sprintf(mode_str,"signal detected");  break;
            case LIQUID_AGC_SQUELCH_SIGNALHI: sprintf(mode_str,"signal high");      break;
            case LIQUID_AGC_SQUELCH_FALL:     sprintf(mode_str,"signal falling");   break;
            case LIQUID_AGC_SQUELCH_SIGNALLO: sprintf(mode_str,"signal low");       break;
            case LIQUID_AGC_SQUELCH_TIMEOUT:  sprintf(mode_str,"signal timed out"); break;
            case LIQUID_AGC_SQUELCH_DISABLED: sprintf(mode_str,"squelch disabled"); break;
            default:                          sprintf(mode_str,"(unknown)");        break;
            }
            printf("%4u : %18s (%1u), rssi = %8.2f dB\n", i, mode_str, mode[i], rssi[i]);
        }
    }

    // destroy AGC object
    agc_crcf_destroy(q);

    // 
    // export results
    //
    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", argv[0], OUTPUT_FILENAME);
        exit(1);
    }
    fprintf(fid,"%% %s: auto-generated file\n\n",OUTPUT_FILENAME);
    fprintf(fid,"clear all;\nclose all;\n\n");
    fprintf(fid,"n = %u;\n", num_samples);

    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"rssi(%4u)  = %12.4e;\n", i+1, rssi[i]);
        fprintf(fid,"mode(%4u)  = %d;\n", i+1, mode[i]);
    }

    // plot results
    fprintf(fid,"\n");
    fprintf(fid,"figure('position',[100 100 1200 900]);\n");
    fprintf(fid,"t = 0:(n-1);\n");
    fprintf(fid,"subplot(4,1,1);\n");
    fprintf(fid,"  plot(t, real(x), '-', 'LineWidth',1.2, 'Color',[0 0.2 0.5],...\n");
    fprintf(fid,"       t, imag(x), '-', 'LineWidth',1.2, 'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('input');\n");
    fprintf(fid,"  axis([0 %u -0.011 0.011]);\n", num_samples);
    fprintf(fid,"subplot(4,1,2);\n");
    fprintf(fid,"  plot(t, real(y), '-', 'LineWidth',1.2, 'Color',[0 0.2 0.5],...\n");
    fprintf(fid,"       t, imag(y), '-', 'LineWidth',1.2, 'Color',[0 0.5 0.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('output');\n");
    fprintf(fid,"subplot(4,1,3);\n");
    fprintf(fid,"  plot(t,rssi,'-','LineWidth',1.2,'Color',[0.5 0 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('rssi [dB]');\n");
    fprintf(fid,"subplot(4,1,4);\n");
    fprintf(fid,"  plot(t,mode,'-s','LineWidth',1.2,'MarkerSize',4,'Color',[0.5 0 0]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('sample index');\n");
    fprintf(fid,"  ylabel('squelch mode');\n");
    fprintf(fid,"  axis([0 %u 0 8]);\n", num_samples);

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

