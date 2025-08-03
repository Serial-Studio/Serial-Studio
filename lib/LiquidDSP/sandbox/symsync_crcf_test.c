//
// symsync_crcf_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symsync_crcf_test.m"

// print usage/help message
void usage()
{
    printf("symsync_crcf_test [options]\n");
    printf("  u/h   : print usage\n");
    printf("  T     : filter type: rrcos, rkaiser, [arkaiser], hM3, gmsk\n");
    printf("  m     : filter delay (symbols), default: 3\n");
    printf("  b     : filter excess bandwidth, default: 0.5\n");
    printf("  B     : filter polyphase banks, default: 32\n");
    printf("  s     : signal-to-noise ratio, default: 30dB\n");
    printf("  w     : timing pll bandwidth, default: 0.02\n");
    printf("  n     : number of symbols, default: 200\n");
    printf("  t     : timing phase offset [%% symbol], t in [-0.5,0.5], default: -0.2\n");
}

// filter coefficients
//  alpha   zeta    settling time (symbols)
//  0.90    1.5     70
//  0.92    1.4     100
//  0.94    1.2     110
//  0.96    0.5     140
//  0.98    0.2     210
//  0.985   0.08    410
//  0.990   0.05    500
//  0.995   0.01    1100
//  0.998   0.002   2500
//
// Let bandwidth = 1/settling time,

int main(int argc, char*argv[]) {
    srand(time(NULL));

    // options
    unsigned int k=2;               // samples/symbol (input)
    unsigned int m=3;               // filter delay (symbols)
    float beta=0.5f;                // filter excess bandwidth factor
    unsigned int npfb=32;           // number of filters in the bank
    unsigned int num_symbols=512;   // number of data symbols
    float SNRdB = 30.0f;            // signal-to-noise ratio
    liquid_firfilt_type ftype_tx = LIQUID_FIRFILT_ARKAISER;
    liquid_firfilt_type ftype_rx = LIQUID_FIRFILT_ARKAISER;

    float bt=0.02f;     // loop filter bandwidth
    float tau=-0.2f;    // fractional symbol offset

    int dopt;
    while ((dopt = getopt(argc,argv,"uhT:m:b:B:s:w:n:t:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();                        return 0;
        case 'T':
            if (strcmp(optarg,"gmsk")==0) {
                ftype_tx = LIQUID_FIRFILT_GMSKTX;
                ftype_rx = LIQUID_FIRFILT_GMSKRX;
            } else {
                ftype_tx = liquid_getopt_str2firfilt(optarg);
                ftype_rx = liquid_getopt_str2firfilt(optarg);
            }
            if (ftype_tx == LIQUID_FIRFILT_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown filter type '%s'\n", argv[0], optarg);
                exit(1);
            }
            break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'B':   npfb = atoi(optarg);     break;
        case 's':   SNRdB       = atof(optarg);     break;
        case 'w':   bt          = atof(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        case 't':   tau         = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (m < 1) {
        fprintf(stderr,"error: m (filter delay) must be greater than 0\n");
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: beta (excess bandwidth factor) must be in (0,1]\n");
        exit(1);
    } else if (npfb == 0) {
        fprintf(stderr,"error: number of polyphase filters must be greater than 0\n");
        exit(1);
    } else if (bt <= 0.0f) {
        fprintf(stderr,"error: timing PLL bandwidth must be greater than 0\n");
        exit(1);
    } else if (num_symbols == 0) {
        fprintf(stderr,"error: number of symbols must be greater than 0\n");
        exit(1);
    } else if (tau < -1.0f || tau > 1.0f) {
        fprintf(stderr,"error: timing phase offset must be in [-1,1]\n");
        exit(1);
    }

    unsigned int i;

    unsigned int num_samples = k*num_symbols;
    float complex sym_in[num_symbols];          // data symbols
    float complex x[num_samples];               // interpolated samples
    float complex y[num_samples];               // noisy samples
    float complex sym_out[num_symbols + 64];    // synchronized symbols

    // generate random QPSK symbols
    for (i=0; i<num_symbols; i++) {
        sym_in[i] = ( rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2 ) +
                    ( rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2 ) * _Complex_I;
    }

    // interpolate symbols
    firinterp_crcf q = firinterp_crcf_create_prototype(ftype_tx, k, m, beta, tau);
    for (i=0; i<num_symbols; i++) {
        firinterp_crcf_execute(q, sym_in[i], &x[i*k]);
    }
    firinterp_crcf_destroy(q);

    // add noise
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples; i++)
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;


    // create matched and derivative-matched filters
    firpfb_crcf  mf = firpfb_crcf_create_rnyquist( ftype_rx, npfb, k, m, beta);
    firpfb_crcf dmf = firpfb_crcf_create_drnyquist(ftype_rx, npfb, k, m, beta);

    // run symbol timing recovery algorithm
    unsigned int n = 0;
    //float tau_hat[num_samples];
    float alpha     = 0.96;
    float zeta      = 0.5f;
    float pfb_error = 0.0f;
    float pfb_q     = 0.0f;
    float pfb_soft  = 0.0f;
    int   pfb_index = 0;
    int   pfb_timer = 0;   // output flag
    // deubgging...
    float debug_pfb_error[num_symbols + 64];
    float debug_pfb_q[num_symbols + 64];
    float debug_pfb_soft[num_symbols + 64];
    for (i=0; i<num_samples; i++) {
        // push sample into filters
        firpfb_crcf_push(mf,  y[i]);
        firpfb_crcf_push(dmf, y[i]);

        //
        if (pfb_timer <= 0) {
            // reset timer
            pfb_timer = 2;  // k samples/symbol

            // compute filterbank outputs
            float complex v  = 0.0f;
            float complex dv = 0.0f;
            firpfb_crcf_execute(mf,  pfb_index, &v);
            firpfb_crcf_execute(dmf, pfb_index, &dv);

            // scale output by samples/symbol
            v /= (float)k;

            // save output
            sym_out[n] = v;

            // compute error
            pfb_error = crealf(conjf(v)*dv);
            pfb_q     = alpha*pfb_q + zeta*pfb_error;
            pfb_soft  += pfb_q;

            // update index
            pfb_index = (int) roundf(pfb_soft);
            while (pfb_index < 0) {
                pfb_index += npfb;
                pfb_soft  += npfb;
                pfb_timer--;
            }
            while (pfb_index > npfb-1) {
                pfb_index -= npfb;
                pfb_soft  -= npfb;
                pfb_timer++;
            }
    
            // save debugging outputs
            debug_pfb_error[n]  = pfb_error;
            debug_pfb_q[n]      = pfb_q;
            debug_pfb_soft[n]   = pfb_soft;
            
            // increment output counter
            n++;
        }

        // decrement timer
        pfb_timer--;
    }
    
    // destroy filterbanks
    firpfb_crcf_destroy(mf);
    firpfb_crcf_destroy(dmf);

    // print last several symbols to screen
    printf("output symbols:\n");
    for (i=n-10; i<n; i++)
        printf("  sym_out(%2u) = %8.4f + j*%8.4f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));

    //
    // export output file
    //

    FILE* fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s, auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"close all;\nclear all;\n\n");

    fprintf(fid,"k=%u;\n",k);
    fprintf(fid,"m=%u;\n",m);
    fprintf(fid,"beta=%12.8f;\n",beta);
    fprintf(fid,"npfb=%u;\n",npfb);
    fprintf(fid,"num_symbols=%u;\n",num_symbols);
    fprintf(fid,"n=%u;\n", n);

    for (i=0; i<num_symbols; i++)
        fprintf(fid,"sym_in(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_in[i]), cimagf(sym_in[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        
    for (i=0; i<n; i++) {
        fprintf(fid,"sym_out(%5u)   = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));
        fprintf(fid,"pfb_error(%5u) = %12.4e;\n", i+1, debug_pfb_error[i]);
        fprintf(fid,"pfb_q(%5u)     = %12.4e;\n", i+1, debug_pfb_q[i]);
        fprintf(fid,"pfb_soft(%5u)  = %12.4e;\n", i+1, debug_pfb_soft[i]);
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(sym_out),imag(sym_out),'x');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.3);\n");
    fprintf(fid,"axis square\n");
    fprintf(fid,"grid on;\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(1:n, pfb_error, 'Color',[1 1 1]*0.8, ...\n");
    fprintf(fid,"       1:n, pfb_q, '-b', 'LineWidth',1.2);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  ylabel('timing error');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  plot(1:n, pfb_soft);\n");
    //fprintf(fid,"  axis([0 n -1 npfb]);\n");
    fprintf(fid,"  xlabel('output symbol');\n");
    fprintf(fid,"  ylabel('filterbank index');\n");
    fprintf(fid,"  grid on;\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
