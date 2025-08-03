//
// ofdmframesync_cfo_test : test carrier frequency offset estimation
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid.h"

#define OUTPUT_FILENAME "ofdmframesync_cfo_test.dat"

void usage()
{
    printf("Usage: ofdmframesync_cfo_test [OPTION]\n");
    printf("  h     : print help\n");
    printf("  M     : number of subcarriers (must be even), default: 64\n");
    printf("  C     : cyclic prefix length, default: 16\n");
    printf("  T     : taper length, default: 4\n");
    printf("  S     : signal-to-noise ratio [dB], default: 20\n");
    printf("  t     : number of trials, default: 500\n");
    printf("  s     : number of steps, default: 21\n");
}

static int callback(float complex * _X,
                    unsigned char * _p,
                    unsigned int    _M,
                    void *          _userdata);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned int M           = 64;      // number of subcarriers
    unsigned int cp_len      = 16;      // cyclic prefix length
    unsigned int taper_len   = 4;       // taper length
    float noise_floor        = -30.0f;  // noise floor [dB]
    float SNRdB              = 20.0f;   // signal-to-noise ratio [dB]
    unsigned int num_trials  = 500;     // number of trials to simulate
    unsigned int num_steps   = 21;      // number of steps

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hM:C:T:S:t:s:")) != EOF){
        switch (dopt) {
        case 'h': usage();                  return 0;
        case 'M': M         = atoi(optarg); break;
        case 'C': cp_len    = atoi(optarg); break;
        case 'T': taper_len = atoi(optarg); break;
        case 'S': SNRdB     = atof(optarg); break;
        case 't': num_trials= atoi(optarg); break;
        case 's': num_steps = atoi(optarg); break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int num_symbols = 1;             // number of data symbols
    unsigned int symbol_len = M + cp_len;
    unsigned int num_samples = (3+num_symbols)*symbol_len;  // data symbols
    float nstd = powf(10.0f, noise_floor/20.0f);
    float gamma = powf(10.0f, (SNRdB + noise_floor)/20.0f);
    printf("gamma : %f\n", gamma);

    // initialize subcarrier allocation
    unsigned char p[M];
    ofdmframe_init_default_sctype(M, p);

    // create frame generator
    ofdmframegen fg = ofdmframegen_create(M, cp_len, taper_len, p);
    ofdmframegen_print(fg);

    // create frame synchronizer
    int frame_detected;
    ofdmframesync fs = ofdmframesync_create(M, cp_len, taper_len, p, callback, (int*)&frame_detected);
    ofdmframesync_print(fs);

    float complex X[M];                 // channelized symbols
    float complex frame[num_samples];   // initial frame
    float complex y[num_samples];       // output time series

    unsigned int n=0;

    // generate frame
    ofdmframegen_write_S0a(fg, &frame[n]);  n += symbol_len;
    ofdmframegen_write_S0b(fg, &frame[n]);  n += symbol_len;
    ofdmframegen_write_S1( fg, &frame[n]);  n += symbol_len;
    // generate single data symbol (random BPSK)
    for (i=0; i<M; i++)
        X[i] = rand() % 2 ? 1.0f : -1.0f;
    ofdmframegen_writesymbol(fg, X, &frame[n]);
    
    // carrier frequency offset
    float nu_min = 0.0f;            // minimum
    float nu_max = 0.9f*M_PI / (float)M; // maximum
    float nu_step = (nu_max - nu_min) / (float)(num_steps-1);

    // count subcarrier types
    unsigned int M_data  = 0;
    unsigned int M_pilot = 0;
    unsigned int M_null  = 0;
    ofdmframe_validate_sctype(p, M, &M_null, &M_pilot, &M_data);

    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"# %s: auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"#\n");
    fprintf(fid,"#  M           = %u\n", M);
    fprintf(fid,"#  M_data      = %u\n", M_data);
    fprintf(fid,"#  M_pilot     = %u\n", M_pilot);
    fprintf(fid,"#  M_null      = %u\n", M_null);
    fprintf(fid,"#  cp_len      = %u\n", cp_len);
    fprintf(fid,"#  num_samples = %u\n", num_samples);
    fprintf(fid,"#  noise_floor = %f\n", noise_floor);
    fprintf(fid,"#  SNRdB = %f\n", SNRdB);
    fprintf(fid,"#\n");
    fprintf(fid,"# %12s %12s %12s %12s %12s %12s\n",
            "id", "nu", "detected", "total", "bias", "rmse");

    // run trials
    for (n=0; n<num_steps; n++) {
        float nu = nu_min + nu_step*n;
        unsigned int t;
        unsigned int num_frames_detected=0;
        float nu_hat_bias = 0.0f;
        float nu_hat_rmse = 0.0f;
        for (t=0; t<num_trials; t++) {

            // reset frame synchronizer
            ofdmframesync_reset(fs);

            // set initial estimate to zero
            frame_detected = 0;

            // add noise, carrier offset
            float phi = randf()*2*M_PI; // random initial phase offset
            for (i=0; i<num_samples; i++) {
                // add carrier offset
                y[i] = gamma * frame[i] * cexpf(_Complex_I*(phi + nu*i));

                // add noise
                y[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;
            }

            // execute synchronizer
            ofdmframesync_execute(fs,y,num_samples);

            // increment number of frames detected
            num_frames_detected += frame_detected;

            // assume frame was detected; save carrier offset estimate
            float nu_hat = ofdmframesync_get_cfo(fs);

            // accumulate statistics
            float nu_hat_error = nu - nu_hat;
            nu_hat_bias += nu_hat_error;
            nu_hat_rmse += nu_hat_error*nu_hat_error;

        }
        nu_hat_bias /= (float)num_trials;
        nu_hat_rmse /= (float)num_trials;
        nu_hat_rmse  = sqrtf(nu_hat_rmse);

        // print results
        printf("%6u, nu=%12.8f", n, nu);
        printf(", frames: %6u / %6u", num_frames_detected, num_trials);
        printf(", bias: %12.8f", nu_hat_bias);
        printf(", rmse: %12.8f", nu_hat_rmse);
        printf("\n");
        fprintf(fid,"  %12u %12f %12u %12u %12.4e %12.4e\n",
                n, nu, num_frames_detected, num_trials,
                nu_hat_bias, nu_hat_rmse);
    }

    // destroy objects
    ofdmframegen_destroy(fg);
    ofdmframesync_destroy(fs);


    // 
    // export output file
    //

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

static int callback(float complex * _X,
                    unsigned char * _p,
                    unsigned int    _M,
                    void *          _userdata)
{
    //printf("**** callback invoked\n");

    // set flag that frame was detected
    int * frame_detected = (int*)_userdata;
    *frame_detected = 1;

    // return zero as not to reset synchronizer (need to retain
    // internal carrier frequency offset estimate)
    return 0;
}

