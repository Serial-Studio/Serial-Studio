//
// ofdmframesync_example.c
//
// Example demonstrating the base OFDM frame synchronizer with different
// parameters and options.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ofdmframesync_example.m"

void usage()
{
    printf("Usage: ofdmframesync_example [OPTION]\n");
    printf("  h     : print help\n");
    printf("  M     : number of subcarriers (must be even),  default: 1200\n");
    printf("  C     : cyclic prefix length,                  default: 60\n");
    printf("  T     : taper length,                          default: 50\n");
    printf("  s     : signal-to-noise ratio [dB],            default: 30\n");
}

// forward declaration of callback function; this will be invoked for every
// OFDM symbol received by the parent ofdmframesync object. The object will
// reset when something other than a zero is returned.
//  _X          : array of received subcarrier samples [size: _M x 1]
//  _p          : subcarrier allocation array [size: _M x 1]
//  _M          : number of subcarriers
//  _userdata   : user-defined data pointer
static int callback(float complex * _X,
                    unsigned char * _p,
                    unsigned int    _M,
                    void *          _userdata);

// custom data type to pass to callback function
struct rx_symbols {
    float complex syms_bpsk[2000];  // even subcarrier symbols
    float complex syms_qpsk[2000];  // odd subcarrier symbols
    unsigned int  num_bpsk;         // counter
    unsigned int  num_qpsk;         // counter
};

// main function
int main(int argc, char*argv[])
{
    // set the random seed differently for each run
    srand(time(NULL));

    // options
    unsigned int M           = 1200;    // number of subcarriers
    unsigned int cp_len      = 60;      // cyclic prefix length
    unsigned int taper_len   = 50;      // taper length
    unsigned int num_symbols = 20;      // number of data symbols
    float noise_floor        = -120.0f; // noise floor [dB]
    float SNRdB              = 30.0f;   // signal-to-noise ratio [dB]

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hdM:C:T:s:")) != EOF){
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'M': M         = atoi(optarg);     break;
        case 'C': cp_len    = atoi(optarg);     break;
        case 'T': taper_len = atoi(optarg);     break;
        case 's': SNRdB     = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int frame_len   = M + cp_len;
    unsigned int num_samples = (3+num_symbols)*frame_len;
    float        nstd        = powf(10.0f, noise_floor/20.0f);
    float        gamma       = powf(10.0f, (SNRdB + noise_floor)/20.0f);

    unsigned char p[M];
    float complex X[M];             // channelized symbols
    float complex y[num_samples];   // output time series

    // initialize subcarrier allocation
    ofdmframe_init_default_sctype(M, p);

    // create subcarrier notch in upper half of band
    unsigned int n0 = (unsigned int) (0.13 * M);    // lower edge of notch
    unsigned int n1 = (unsigned int) (0.21 * M);    // upper edge of notch
    for (i=n0; i<n1; i++)
        p[i] = OFDMFRAME_SCTYPE_NULL;

    // create struct for holding data symbols
    struct rx_symbols data;
    data.num_bpsk = 0;
    data.num_qpsk = 0;

    // create frame generator
    ofdmframegen fg = ofdmframegen_create(M, cp_len, taper_len, p);
    ofdmframegen_print(fg);

    // create frame synchronizer
    ofdmframesync fs = ofdmframesync_create(M, cp_len, taper_len, p, callback, (void*)&data);
    ofdmframesync_print(fs);

    unsigned int n=0;

    // write first S0 symbol
    ofdmframegen_write_S0a(fg, &y[n]);
    n += frame_len;

    // write second S0 symbol
    ofdmframegen_write_S0b(fg, &y[n]);
    n += frame_len;

    // write S1 symbol
    ofdmframegen_write_S1( fg, &y[n]);
    n += frame_len;

    // modulate data subcarriers
    for (i=0; i<num_symbols; i++) {

        // load different subcarriers with different data
        unsigned int j;
        for (j=0; j<M; j++) {
            // ignore 'null' and 'pilot' subcarriers
            if (p[j] != OFDMFRAME_SCTYPE_DATA)
                continue;

            // use BPSK for even frequencies, QPSK for odd
            if ( (j % 2) == 0 ) {
                // BPSK
                X[j] = rand() % 2 ? -1.0f : 1.0f;
            } else {
                // QPSK
                X[j] = (rand() % 2 ? -0.707f : 0.707f) +
                       (rand() % 2 ? -0.707f : 0.707f) * _Complex_I;
            }
        }

        // generate OFDM symbol in the time domain
        ofdmframegen_writesymbol(fg, X, &y[n]);
        n += frame_len;
    }

    // add channel effects
    for (i=0; i<num_samples; i++) {

        // channel gain
        y[i] *= gamma;

        // add noise
        y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    }

    // execute synchronizer on entire frame
    ofdmframesync_execute(fs,y,num_samples);

    // destroy objects
    ofdmframegen_destroy(fg);
    ofdmframesync_destroy(fs);

    // estimate power spectral density of received signal
    unsigned int nfft = 1024;   // FFT size
    float        psd[nfft];     // PSD estimate output array
    spgramcf_estimate_psd(nfft, y, num_samples, psd);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"M           = %u;\n", M);
    fprintf(fid,"noise_floor = %f;\n", noise_floor);
    fprintf(fid,"SNRdB       = %f;\n", SNRdB);
    fprintf(fid,"num_samples = %u;\n", num_samples);
    fprintf(fid,"nfft        = %u;\n", nfft);

    // save received time-domain signal
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));

    // save power spectral density estimate
    fprintf(fid,"psd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"psd(%3u) = %12.4e;\n", i+1, psd[i]);
    fprintf(fid,"psd = 10*log10( fftshift(psd) );\n");

    // save received symbols
    fprintf(fid,"num_bpsk = %u;\n", data.num_bpsk);
    fprintf(fid,"num_qpsk = %u;\n", data.num_qpsk);
    fprintf(fid,"syms_bpsk = zeros(1,num_bpsk);\n");
    fprintf(fid,"syms_qpsk = zeros(1,num_qpsk);\n");
    for (i=0; i<data.num_bpsk; i++)
        fprintf(fid,"syms_bpsk(%3u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(data.syms_bpsk[i]), cimagf(data.syms_bpsk[i]));
    for (i=0; i<data.num_qpsk; i++)
        fprintf(fid,"syms_qpsk(%3u) = %12.4e + 1i*%12.4e;\n", i+1, crealf(data.syms_qpsk[i]), cimagf(data.syms_qpsk[i]));

    // plot power spectral density
    fprintf(fid,"\n\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,psd,'LineWidth',1.5,'Color',[0 0.3 0.5]);\n");
    fprintf(fid,"psd_min = noise_floor - 10;\n");
    fprintf(fid,"psd_max = noise_floor + 10 + max(SNRdB, 0);\n");
    fprintf(fid,"axis([-0.5 0.5 psd_min psd_max]);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('Power Spectral Density [dB]');\n");

    // plot received constellation
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"plot(real(syms_bpsk),imag(syms_bpsk),'x','MarkerSize',4,'Color',[0 0.3 0.5]);\n");
    fprintf(fid,"plot(real(syms_qpsk),imag(syms_qpsk),'x','MarkerSize',4,'Color',[0 0.5 0.3]);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('I');\n");
    fprintf(fid,"ylabel('Q');\n");
    fprintf(fid,"legend('even subcarriers (BPSK)','odd subcarriers (QPSK)','location','northeast');\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

// callback function
//  _X          : array of received subcarrier samples [size: _M x 1]
//  _p          : subcarrier allocation array [size: _M x 1]
//  _M          : number of subcarriers
//  _userdata   : user-defined data pointer
static int callback(float complex * _X,
                    unsigned char * _p,
                    unsigned int    _M,
                    void *          _userdata)
{
    // print status to the screen
    printf("**** callback invoked\n");

    // save received data subcarriers
    struct rx_symbols * data = (struct rx_symbols*) _userdata;
    unsigned int i;
    for (i=0; i<_M; i++) {
        // ignore 'null' and 'pilot' subcarriers
        if (_p[i] != OFDMFRAME_SCTYPE_DATA)
            continue;

        // extract BPSK (even frequencies) and QPSK (odd frequencies) symbols
        if ( (i % 2) == 0 && data->num_bpsk < 2000) {
            // save at most 2000 BPSK symbols
            data->syms_bpsk[data->num_bpsk] = _X[i];
            data->num_bpsk++;
        } else if ( (i % 2) == 1 && data->num_qpsk < 2000) {
            // save at most 2000 QPSK symbols
            data->syms_qpsk[data->num_qpsk] = _X[i];
            data->num_qpsk++;
        }
    }

    // return
    return 0;
}

