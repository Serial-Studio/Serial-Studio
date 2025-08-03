//
// symsync_eqlms_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <assert.h>

#include "liquid.h"

#define OUTPUT_FILENAME "symsync_eqlms_test.m"

// print usage/help message
void usage()
{
    printf("symsync_eqlms_test [options]\n");
    printf("  u/h   : print usage\n");
    // transmit filter properties
    printf("  k     : filter samples/symbol, default: 2\n");
    printf("  m     : filter delay (symbols), default: 3\n");
    printf("  b     : filter excess bandwidth, default: 0.5\n");
    printf("  n     : number of symbols, default: 400\n");
    // symsync properties
    printf("  B     : filter polyphase banks, default: 32\n");
    printf("  w     : timing pll bandwidth, default: 0.02\n");
    // equalizer properties
    printf("  p     : equalizer order [symbols], default: 3\n");
    printf("  W     : equalizer bandwidth, default: 0.05\n");
    // channel
    printf("  s     : signal-to-noise ratio, default: 30dB\n");
    printf("  c     : channel impulse response length, default: 5\n");
    printf("  t     : timing phase offset [%% symbol], -0.5 < t <= 0.5, default: 0\n");
}


int main(int argc, char*argv[]) {
    srand(time(NULL));

    // options
    unsigned int k=2;               // samples/symbol (input)
    unsigned int m=3;               // filter delay (symbols)
    float beta=0.5f;                // filter excess bandwidth factor
    unsigned int npfb=32;    // number of filters in the bank
    unsigned int p=3;               // equalizer length (symbols, hp_len = 2*k*p+1)
    float mu = 0.05f;               // equalizer learning rate
    unsigned int num_symbols=500;   // number of data symbols
    unsigned int hc_len=5;          // channel filter length
    float SNRdB = 30.0f;            // signal-to-noise ratio
    liquid_firfilt_type ftype = LIQUID_FIRFILT_ARKAISER;

    float bt=0.05f;                 // symbol synchronizer loop filter bandwidth
    float tau=-0.1f;                // fractional symbol offset
    
    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:b:n:B:w:p:W:s:c:t:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':   usage();                        return 0;
        // transmit filter properties
        case 'k':   k           = atoi(optarg);     break;
        case 'm':   m           = atoi(optarg);     break;
        case 'b':   beta        = atof(optarg);     break;
        case 'n':   num_symbols = atoi(optarg);     break;
        // symsync properties
        case 'B':   npfb        = atoi(optarg);     break;
        case 'w':   bt          = atof(optarg);     break;
        // equalizer properties
        case 'p':   p           = atoi(optarg);     break;
        case 'W':   mu          = atof(optarg);     break;
        // equalizer properties
        case 's':   SNRdB       = atof(optarg);     break;
        case 'c':   hc_len      = atoi(optarg);     break;
        case 't':   tau         = atof(optarg);     break;
        default:
            exit(1);
        }
    }

    // validate input
    if (k < 2) {
        fprintf(stderr,"error: %s,k (samples/symbol) must be at least 2\n", argv[0]);
        exit(1);
    } else if (m < 1) {
        fprintf(stderr,"error: %s,m (filter delay) must be greater than 0\n", argv[0]);
        exit(1);
    } else if (beta <= 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: %s,beta (excess bandwidth factor) must be in (0,1]\n", argv[0]);
        exit(1);
    } else if (num_symbols == 0) {
        fprintf(stderr,"error: %s,number of symbols must be greater than 0\n", argv[0]);
        exit(1);
    } else if (npfb == 0) {
        fprintf(stderr,"error: %s,number of polyphase filters must be greater than 0\n", argv[0]);
        exit(1);
    } else if (bt < 0.0f) {
        fprintf(stderr,"error: %s,timing PLL bandwidth cannot be negative\n", argv[0]);
        exit(1);
    } else if (p == 0) {
        fprintf(stderr,"error: %s, equalizer order must be at least 1\n", argv[0]);
        exit(1);
    } else if (mu < 0.0f || mu > 1.0f) {
        fprintf(stderr,"error: %s, equalizer learning rate must be in [0,1]\n", argv[0]);
        exit(1);
    } else if (hc_len < 1) {
        fprintf(stderr,"error: %s, channel response must have at least 1 tap\n", argv[0]);
        exit(1);
    } else if (tau < -1.0f || tau > 1.0f) {
        fprintf(stderr,"error: %s,timing phase offset must be in [-1,1]\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int ht_len = 2*k*m+1;  // transmit filter order
    unsigned int hp_len = 2*k*p+1;  // equalizer order
    float nstd = powf(10.0f, -SNRdB/20.0f);

    float dt = tau;                 // fractional sample offset
    unsigned int ds = 0;            // full sample delay

    unsigned int i;

    unsigned int num_samples = k*num_symbols;
    float complex s[num_symbols];               // data symbols
    float complex x[num_samples];               // interpolated samples
    float complex y[num_samples];               // channel output
    float complex z[k*num_symbols + 64];        // synchronized samples
    float complex sym_out[num_symbols + 64];    // synchronized symbols

    for (i=0; i<num_symbols; i++) {
        s[i] = (rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2) +
               (rand() % 2 ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;
    }

    // 
    // create and run interpolator
    //

    // design interpolating filter
    float ht[ht_len];
    liquid_firdes_prototype(ftype,k,m,beta,dt,ht);
    firinterp_crcf q = firinterp_crcf_create(k, ht, ht_len);
    for (i=0; i<num_symbols; i++)
        firinterp_crcf_execute(q, s[i], &x[i*k]);
    firinterp_crcf_destroy(q);


    // 
    // channel
    //

    // generate channel impulse response, filter
    float complex hc[hc_len];
    hc[0] = 1.0f;
    for (i=1; i<hc_len; i++)
        hc[i] = 0.07f*(randnf() + randnf()*_Complex_I);
    firfilt_cccf fchannel = firfilt_cccf_create(hc, hc_len);
    // push through channel
    for (i=0; i<num_samples; i++) {
        firfilt_cccf_push(fchannel, x[i]);
        firfilt_cccf_execute(fchannel, &y[i]);

        // add noise
        y[i] += nstd*(randnf() + randnf()*_Complex_I)*M_SQRT1_2;
    }
    firfilt_cccf_destroy(fchannel);


    // 
    // symbol timing recovery
    //

    // create symbol synchronizer
    symsync_crcf d = symsync_crcf_create_rnyquist(ftype, k, m, beta, npfb);
    symsync_crcf_set_lf_bw(d,bt);
    symsync_crcf_set_output_rate(d,k);

    unsigned int num_samples_sync=0;
    unsigned int nw;
    for (i=ds; i<num_samples; i++) {
        // push through symbol synchronizer
        symsync_crcf_execute(d, &y[i], 1, &z[num_samples_sync], &nw);
        num_samples_sync += nw;
    }
    printf("num samples : %6u (%6u synchronized)\n", num_samples, num_samples_sync);
    symsync_crcf_destroy(d);


    // 
    // equalizer/decimator
    //

    // create equalizer as low-pass filter
    float complex hp[hp_len];
    eqlms_cccf eq = eqlms_cccf_create_lowpass(hp_len, 0.4f);
    eqlms_cccf_set_bw(eq, mu);

    // push through equalizer and decimate
    unsigned int num_symbols_sync = 0;
    float complex d_hat = 0.0f;
    for (i=0; i<num_samples_sync; i++) {
        // push sample into equalizer
        eqlms_cccf_push(eq, z[i]);

        // decimate by k
        if ( (i%k) != 0) continue;

        // compute output
        eqlms_cccf_execute(eq, &d_hat);
        sym_out[num_symbols_sync++] = d_hat;

        // check if buffer is full
        if ( i < hp_len ) continue;

        // estimate transmitted signal
        float complex d_prime = (crealf(d_hat) > 0.0f ? M_SQRT1_2 : -M_SQRT1_2) +
                                (cimagf(d_hat) > 0.0f ? M_SQRT1_2 : -M_SQRT1_2) * _Complex_I;

        // update equalizer
        eqlms_cccf_step(eq, d_prime, d_hat);
    }

    // get equalizer weights
    eqlms_cccf_copy_coefficients(eq, hp);

    // destroy equalizer object
    eqlms_cccf_destroy(eq);

    // print last several symbols to screen
    printf("output symbols:\n");
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
    fprintf(fid,"npfb=%u;\n",npfb);
    fprintf(fid,"num_symbols=%u;\n",num_symbols);

    for (i=0; i<ht_len; i++)
        fprintf(fid,"ht(%3u) = %12.5f;\n", i+1, ht[i]);

    for (i=0; i<hc_len; i++)
        fprintf(fid,"hc(%3u) = %12.5f + j*%12.8f;\n", i+1, crealf(hc[i]), cimagf(hc[i]));

    for (i=0; i<hp_len; i++)
        fprintf(fid,"hp(%3u) = %12.5f + j*%12.8f;\n", i+1, crealf(hp[i]), cimagf(hp[i]));

    for (i=0; i<num_symbols; i++)
        fprintf(fid,"s(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(s[i]), cimagf(s[i]));

    for (i=0; i<num_samples; i++)
        fprintf(fid,"x(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        
    for (i=0; i<num_samples; i++)
        fprintf(fid,"y(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        
    for (i=0; i<num_samples_sync; i++)
        fprintf(fid,"z(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(z[i]), cimagf(z[i]));
        
    for (i=0; i<num_symbols_sync; i++)
        fprintf(fid,"sym_out(%3u) = %12.8f + j*%12.8f;\n", i+1, crealf(sym_out[i]), cimagf(sym_out[i]));
        
#if 0
    fprintf(fid,"\n\n");
    fprintf(fid,"%% scale QPSK in-phase by sqrt(2)\n");
    fprintf(fid,"z = z*sqrt(2);\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"tz = [0:length(z)-1]/k;\n");
    fprintf(fid,"iz = 1:k:length(z);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(tz,     real(z),    '-',...\n");
    fprintf(fid,"     tz(iz), real(z(iz)),'or');\n");
    fprintf(fid,"xlabel('Time');\n");
    fprintf(fid,"ylabel('Output Signal (real)');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"legend('output time series','optimim timing',1);\n");
#endif

    // compute composite response
    fprintf(fid,"hd = real(conv(ht/k,conv(hc,hp)));\n");

    // plot frequency response
    fprintf(fid,"nfft = 1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"Ht = 20*log10(abs(fftshift(fft(ht/k,nfft))));\n");
    fprintf(fid,"Hc = 20*log10(abs(fftshift(fft(hc,  nfft))));\n");
    fprintf(fid,"Hp = 20*log10(abs(fftshift(fft(hp,  nfft))));\n");
    fprintf(fid,"Hd = 20*log10(abs(fftshift(fft(hd,  nfft))));\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,Ht, f,Hc, f,Hp, f,Hd,'-k','LineWidth',2);\n");
    fprintf(fid,"axis([-0.5 0.5 -20 10]);\n");
    fprintf(fid,"axis([-0.5 0.5 -6  6 ]);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"legend('transmit','channel','equalizer','composite','location','northeast');\n");

    fprintf(fid,"i0 = [1:round(length(sym_out)/2)];\n");
    fprintf(fid,"i1 = [round(length(sym_out)/2):length(sym_out)];\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(sym_out(i0)),imag(sym_out(i0)),'x','MarkerSize',4,'Color',[0.60 0.60 0.60],...\n");
    fprintf(fid,"     real(sym_out(i1)),imag(sym_out(i1)),'x','MarkerSize',4,'Color',[0.00 0.25 0.50]);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"xlabel('In-phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"legend(['first 50%%'],['last 50%%'],'location','northeast');\n");

    fclose(fid);

    printf("results written to %s.\n", OUTPUT_FILENAME);

    // clean it up
    printf("done.\n");
    return 0;
}
