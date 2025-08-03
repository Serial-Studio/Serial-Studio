//
// symsync_crcf_full_example.c
//
// This example extends that of `symsync_crcf_example.c` by including options
// for simulating a timing rate offset in addition to just a timing phase
// error. The resulting output file shows not just the constellation but the
// time domain sequence as well as the timing phase estimate over time.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symsync_crcf_full_example.m"

// print usage/help message
void usage()
{
    printf("symsync_crcf_full_example [options]\n");
    printf("  h     : print usage\n");
    printf("  T     : filter type: [rrcos], rkaiser, arkaiser, hM3, gmsk\n");
    printf("  k     : filter samples/symbol,   default: 2\n");
    printf("  K     : output samples/symbol,   default: 2\n");
    printf("  m     : filter delay (symbols),  default: 5\n");
    printf("  b     : filter excess bandwidth, default: 0.5\n");
    printf("  B     : filter polyphase banks,  default: 32\n");
    printf("  s     : signal-to-noise ratio,   default: 30dB\n");
    printf("  w     : timing pll bandwidth,    default: 0.02\n");
    printf("  n     : number of symbols,       default: 400\n");
    printf("  t     : timing phase offset [%% symbol], t in [-0.5,0.5], default: -0.2\n");
    printf("  r     : timing freq. offset [%% symbol], default: 1.000\n");
}


int main(int argc, char*argv[]) {
    srand(time(NULL));

    // options
    unsigned int k           =   2;     // samples/symbol (input)
    unsigned int k_out       =   2;     // samples/symbol (output)
    unsigned int m           =   5;     // filter delay (symbols)
    float        beta        =   0.5f;  // filter excess bandwidth factor
    unsigned int num_filters =  32;     // number of filters in the bank
    unsigned int num_symbols = 400;     // number of data symbols
    float        SNRdB       =  30.0f;  // signal-to-noise ratio

    // transmit/receive filter types
    liquid_firfilt_type ftype_tx = LIQUID_FIRFILT_RRC;
    liquid_firfilt_type ftype_rx = LIQUID_FIRFILT_RRC;

    float bt    =  0.02f;       // loop filter bandwidth
    float tau   = -0.2f;        // fractional symbol offset
    float r     =    1.00f;     // resampled rate
    
    // use random data or 101010 phasing pattern
    int random_data=1;

    int dopt;
    while ((dopt = getopt(argc,argv,"hT:k:K:m:b:B:s:w:n:t:r:")) != EOF) {
        switch (dopt) {
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
        case 'k':   k           = atoi(optarg);     break;
        case 'K':   k_out       = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'B':   num_filters = atoi(optarg);     break;
        case 's':   SNRdB       = atof(optarg);     break;
        case 'w':   bt          = atof(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        case 't':   tau         = atof(optarg);     break;
        case 'r':   r           = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (k < 2) {
        fprintf(stderr,"error: k (samples/symbol) must be at least 2\n");
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: m (filter delay) must be greater than 0\n");
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: beta (excess bandwidth factor) must be in (0,1]\n");
        exit(1);
    } else if (num_filters == 0) {
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
    } else if (r < 0.5f || r > 2.0f) {
        fprintf(stderr,"error: timing frequency offset must be in [0.5,2]\n");
        exit(1);
    }

    // compute delay
    while (tau < 0) tau += 1.0f;    // ensure positive tau
    float g = k*tau;                // number of samples offset
    int ds=floorf(g);               // additional symbol delay
    float dt = (g - (float)ds);     // fractional sample offset
    if (dt > 0.5f) {                // force dt to be in [0.5,0.5]
        dt -= 1.0f;
        ds++;
    }

    unsigned int i, n=0;

    unsigned int num_samples = k*num_symbols;
    unsigned int num_samples_resamp = (unsigned int) ceilf(num_samples*r*1.1f) + 4;
    float complex s[num_symbols];           // data symbols
    float complex x[num_samples];           // interpolated samples
    float complex y[num_samples_resamp];    // resampled data (resamp_crcf)
    float complex z[k_out*num_symbols + 64];// synchronized samples
    float complex sym_out[num_symbols + 64];// synchronized symbols

    for (i=0; i<num_symbols; i++) {
        if (random_data) {
            // random signal (QPSK)
            s[i]  = cexpf(_Complex_I*0.5f*M_PI*((rand() % 4) + 0.5f));
        } else {
            s[i] = (i%2) ? 1.0f : -1.0f;  // 101010 phasing pattern
        }
    }

    // 
    // create and run interpolator
    //

    // design interpolating filter
    unsigned int h_len = 2*k*m+1;
    float h[h_len];
    liquid_firdes_prototype(ftype_tx,k,m,beta,dt,h);
    firinterp_crcf q = firinterp_crcf_create(k,h,h_len);
    for (i=0; i<num_symbols; i++) {
        firinterp_crcf_execute(q, s[i], &x[n]);
        n+=k;
    }
    assert(n == num_samples);
    firinterp_crcf_destroy(q);

    // 
    // run resampler
    //
    unsigned int resamp_len = 10*k; // resampling filter semi-length (filter delay)
    float resamp_bw = 0.45f;        // resampling filter bandwidth
    float resamp_as = 60.0f;        // resampling filter stop-band attenuation
    unsigned int resamp_npfb = 64;  // number of filters in bank
    resamp_crcf f = resamp_crcf_create(r, resamp_len, resamp_bw, resamp_as, resamp_npfb);
    unsigned int num_samples_resampled = 0;
    unsigned int num_written;
    for (i=0; i<num_samples; i++) {
#if 0
        // bypass arbitrary resampler
        y[i] = x[i];
        num_samples_resampled = num_samples;
#else
        // TODO : compensate for resampler filter delay
        resamp_crcf_execute(f, x[i], &y[num_samples_resampled], &num_written);
        num_samples_resampled += num_written;
#endif
    }
    resamp_crcf_destroy(f);

    // 
    // add noise
    //
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples_resampled; i++)
        y[i] += nstd*(randnf() + _Complex_I*randnf());


    // 
    // create and run symbol synchronizer
    //

    symsync_crcf d = symsync_crcf_create_rnyquist(ftype_rx, k, m, beta, num_filters);
    symsync_crcf_set_lf_bw(d,bt);
    symsync_crcf_set_output_rate(d,k_out);

    unsigned int num_samples_sync=0;
    unsigned int nn;
    unsigned int num_symbols_sync = 0;
    float tau_hat[num_samples];
    for (i=ds; i<num_samples_resampled; i++) {
        tau_hat[num_samples_sync] = symsync_crcf_get_tau(d);
        symsync_crcf_execute(d, &y[i], 1, &z[num_samples_sync], &nn);

        // decimate
        unsigned int j;
        for (j=0; j<nn; j++) {
            if ( (num_samples_sync%k_out)==0 )
                sym_out[num_symbols_sync++] = z[num_samples_sync];
            num_samples_sync++;
        }
    }
    symsync_crcf_destroy(d);

    // print last several symbols to screen
    printf("output symbols:\n");
    printf("  ...\n");
    for (i=num_symbols_sync-10; i<num_symbols_sync; i++)
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
    fprintf(fid,"k_out=%u;\n",k_out);
    fprintf(fid,"num_filters=%u;\n",num_filters);
    fprintf(fid,"num_symbols=%u;\n",num_symbols);

    for (i=0; i<h_len; i++)
        fprintf(fid,"h(%3u) = %12.5f;\n", i+1, h[i]);

    for (i=0; i<num_symbols; i++)
        fprintf(fid,"s(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(s[i]), cimagf(s[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        
    for (i=0; i<num_samples_resampled; i++)
        fprintf(fid,"y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        
    for (i=0; i<num_samples_sync; i++)
        fprintf(fid,"z(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(z[i]), cimagf(z[i]));
        
    for (i=0; i<num_symbols_sync; i++)
        fprintf(fid,"sym_out(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));
        
    for (i=0; i<num_samples_sync; i++)
        fprintf(fid,"tau_hat(%3u) = %12.8f;\n", i+1, tau_hat[i]);


    fprintf(fid,"\n\n");
    fprintf(fid,"%% scale QPSK in-phase by sqrt(2)\n");
    fprintf(fid,"z = z*sqrt(2);\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"tz = [0:length(z)-1]/k_out;\n");
    fprintf(fid,"iz = 1:k_out:length(z);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(tz,     real(z),    '-',...\n");
    fprintf(fid,"     tz(iz), real(z(iz)),'or');\n");
    fprintf(fid,"xlabel('Time');\n");
    fprintf(fid,"ylabel('Output Signal (real)');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"legend('output time series','optimum timing','location','northeast');\n");

    fprintf(fid,"iz0 = iz( 1:round(length(iz)*0.5) );\n");
    fprintf(fid,"iz1 = iz( round(length(iz)*0.5):length(iz) );\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(real(z(iz0)),imag(z(iz0)),'x','MarkerSize',4,'Color',[0.6 0.6 0.6]);\n");
    fprintf(fid,"plot(real(z(iz1)),imag(z(iz1)),'o','MarkerSize',4,'Color',[0 0.25 0.5]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*2.0);\n");
    fprintf(fid,"xlabel('In-phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"legend(['first 50%%'],['last 50%%'],'location','northeast');\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"tt = 0:(length(tau_hat)-1);\n");
    fprintf(fid,"b = floor(num_filters*tau_hat + 0.5);\n");
    fprintf(fid,"stairs(tt,tau_hat*num_filters);\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(tt,b,'-k','Color',[0 0 0]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('filterbank index');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([0 length(tau_hat) -1 num_filters]);\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
