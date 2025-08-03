// 
// gmskmodem_test.c
//

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "gmskmodem_test.m"

// print usage/help message
void usage()
{
    printf("gmskmodem_test -- Gaussian minimum-shift keying modem example\n");
    printf("options (default values in <>):\n");
    printf("  u/h   : print usage/help\n");
    printf("  k     : samples/symbol <4>\n");
    printf("  m     : filter delay [symbols], <3>\n");
    printf("  n     : number of data symbols <32>\n");
    printf("  b     : bandwidth-time product, 0 <= b <= 1, <0.3>\n");
    printf("  s     : SNR [dB] <30>\n");
}

int main(int argc, char*argv[]) {
    // options
    unsigned int k=4;                   // filter samples/symbol
    unsigned int m=3;                   // filter delay (symbols)
    float BT=0.5f;                      // bandwidth-time product
    unsigned int num_data_symbols = 32; // number of data symbols
    float SNRdB = 30.0f;                // signal-to-noise ratio [dB]

    int dopt;
    while ((dopt = getopt(argc,argv,"uhk:m:n:b:s:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage();              return 0;
        case 'k': k = atoi(optarg); break;
        case 'm': m = atoi(optarg); break;
        case 'n': num_data_symbols = atoi(optarg); break;
        case 'b': BT = atof(optarg); break;
        case 's': SNRdB = atof(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // validate input
    if (BT <= 0.0f || BT >= 1.0f) {
        fprintf(stderr,"error: %s, bandwidth-time product must be in (0,1)\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int num_symbols = num_data_symbols + 2*m;
    unsigned int num_samples = k*num_symbols;
    unsigned int h_len = 2*k*m+1;
    float nstd = powf(10.0f, -SNRdB/20.0f);

    // arrays
    unsigned char sym_in[num_symbols];      // input symbols
    float phi[num_samples];                 // transmitted phase
    float complex x[num_samples];           // transmitted signal
    float complex y[num_samples];           // received signal
    float phi_hat[num_samples];             // received phase
    float phi_prime[num_samples];           // matched-filter output
    unsigned char sym_out[num_symbols];     // output symbols

    // create transmit/receive interpolator/decimator
#if 0
    float ht[h_len];    // transmit filter coefficients
    liquid_firdes_gmsktx(k,m,BT,0.0f,ht);
    firfilt_rrrf ft = firfilt_rrrf_create(ht,h_len);
#else
    firinterp_rrrf interp_tx = firinterp_rrrf_create_prototype(LIQUID_FIRFILT_GMSKTX,k,m,BT,0);
#endif
    float hr[h_len];    // receive filter coefficients
    liquid_firdes_gmskrx(k,m,BT,0.0f,hr);
    firfilt_rrrf decim_rx = firfilt_rrrf_create(hr,h_len);

    // generate symbols and interpolate
    float theta = 0.0f;
    float k_inv = 1.0f / (float)k;
    for (i=0; i<num_symbols; i++) {
        sym_in[i] = rand() % 2;
        firinterp_rrrf_execute(interp_tx, sym_in[i] ? k_inv : -k_inv, &phi[k*i]);
        //firinterp_rrrf_execute(interp_tx, i==0 ? 1.0f : 0.0f, &phi[k*i]);

        // accumulate phase
        unsigned int j;
        for (j=0; j<k; j++) {
            theta += phi[i*k + j];
            x[i*k+j] = cexpf(_Complex_I*theta);
        }
    }

    // push through channel
    for (i=0; i<num_samples; i++)
        y[i] = x[i] + nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
    
    // run receiver
    float complex x_prime = 0.0f;
    unsigned int n=0;
    for (i=0; i<num_samples; i++) {
        phi_hat[i] = cargf( conjf(x_prime)*y[i] );
        x_prime = y[i];

        // push through filter
        firfilt_rrrf_push(decim_rx, phi_hat[i]);
        firfilt_rrrf_execute(decim_rx, &phi_prime[i]);

        // decimate output
        if ( (i%k)==0 ) {
            sym_out[n] = phi_prime[i] > 0.0f ? 1 : 0;
            printf("%3u : %12.8f (%1u)", n, phi_prime[i], sym_out[n]);
            if (n >= 2*m) printf(" (%1u)\n", sym_in[n-2*m]);
            else          printf("\n");
            n++;
        }
    }

    // compute number of errors
    unsigned int num_errors = 0;
    for (i=0; i<num_data_symbols; i++)
        num_errors += sym_in[i] == sym_out[i+2*m] ? 0 : 1;

    printf("errors : %3u / %3u\n", num_errors, num_data_symbols);

    // destroy objects
    firinterp_rrrf_destroy(interp_tx);
    firfilt_rrrf_destroy(decim_rx);

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"BT = %f;\n", BT);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = %u;\n", num_samples);

    // save filters
    for (i=0; i<h_len; i++)
        fprintf(fid,"hr(%3u) = %12.8f;\n", i+1, hr[i]);

    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u)       = %12.8f + j*%12.8f;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u)       = %12.8f + j*%12.8f;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"phi(%4u) = %12.8f;\n", i+1, phi[i]);
        fprintf(fid,"phi_hat(%4u) = %12.8f;\n", i+1, phi_hat[i]);
        fprintf(fid,"phi_prime(%4u) = %12.8f;\n", i+1, phi_prime[i]);
    }
    fprintf(fid,"t=[0:(num_samples-1)]/k;\n");
    fprintf(fid,"i = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,phi_prime,'-', t(i),phi_prime(i),'or');\n");
    fprintf(fid,"xlabel('time');\n");
    fprintf(fid,"ylabel('matched filter output');\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
