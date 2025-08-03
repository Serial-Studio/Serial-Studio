// 
// sandbox/fecsoft_conv_test.c
//
// This script simulates soft vs. hard decoding of convolutional codes.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <getopt.h>

#include "liquid.internal.h"

#define OUTPUT_FILENAME "fecsoft_conv_test.m"

void usage()
{
    printf("fecsoft_conv_test\n");
    printf("  Simulates soft decoding of convoluational codes\n");
    printf("options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  s     : SNR start [dB], default: -5\n");
    printf("  x     : SNR max [dB], default: 5\n");
    printf("  n     : number of SNR steps, default: 21\n");
    printf("  t     : number of trials, default: 1000\n");
    printf("  c     : convoluational coding scheme: v27, v29, v39, v615, punctured\n");
}

int main(int argc, char *argv[]) {
    // set initial seed to random
    srand(time(NULL));

    // options
    fec_scheme fs = LIQUID_FEC_CONV_V27;    // convolutional FEC scheme
    unsigned int n = 64;                    // original message length
    float SNRdB_min = -5.0f;                // signal-to-noise ratio (minimum)
    float SNRdB_max =  5.0f;                // signal-to-noise ratio (maximum)
    unsigned int num_snr = 21;              // number of SNR steps
    unsigned int num_trials=100;            // number of trials

    // get command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"uhs:x:n:t:c:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 's': SNRdB_min = atof(optarg);     break;
        case 'x': SNRdB_max = atof(optarg);     break;
        case 'n': num_snr = atoi(optarg);       break;
        case 't': num_trials = atoi(optarg);    break;
        case 'c':
            fs = liquid_getopt_str2fec(optarg);
            if (fs == LIQUID_FEC_UNKNOWN ) {
                fprintf(stderr,"error: %s, unknown/unsupported fec scheme \"%s\"\n\n",argv[0], optarg);
                exit(1);
            } else if ( !fec_scheme_is_convolutional(fs) ) {
                fprintf(stderr,"error: %s, input fec scheme '%s' is not convolutional\n\n",argv[0], optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // derived values
    unsigned int k = fec_get_enc_msg_length(fs, n);   // encoded message length

    // create forward error-correction object
    fec q = fec_create(fs, NULL);
    fec_print(q);

    // 
    // data arrays
    //
#if 0
    unsigned int sym_org;       // original symbol
    float complex sym_enc[7];   // encoded symbol
    float complex sym_rec[7];   // received symbol
#endif
    unsigned char msg_org[n];           // original message
    unsigned char msg_enc[k];           // encoded message
    float complex mod_sym[8*k];         // modulated symbols
    unsigned char msg_rec_soft[8*k];    // received 'soft' bits
    unsigned char msg_rec_hard[8*k];    // received 'hard' bits
    unsigned char msg_dec_soft[n];      // soft-decision decoding
    unsigned char msg_dec_hard[n];      // hard-decision decoding

    unsigned int bit_errors_soft[num_snr];
    unsigned int bit_errors_hard[num_snr];

    //
    // set up parameters
    //
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_snr-1);

    // 
    // start trials
    //
#if 0
    // test
    sym_org = 13;
    fecsoft_hamming74_encode(sym_org, sym_enc);
    fecsoft_hamming74_decode(sym_enc, &sym_dec_soft);
    exit(1);
#endif
    
    printf("  %8s [%6s] %6s %6s\n", "SNR [dB]", "trials", "soft", "hard");
    unsigned int s;
    for (s=0; s<num_snr; s++) {
        // compute SNR for this level
        float SNRdB = SNRdB_min + s*SNRdB_step;
        float nstd = powf(10.0f, -SNRdB/20.0f);

        // reset results
        bit_errors_soft[s] = 0;
        bit_errors_hard[s] = 0;

        unsigned int t;
        for (t=0; t<num_trials; t++) {

            // generate data
            for (i=0; i<n; i++)
                msg_org[i] = rand() & 0xff;

            // encode
            if ( fec_scheme_is_punctured(fs) ) {
                fec_conv_punctured_encode(q, n, msg_org, msg_enc);
            } else {
                fec_conv_encode(q, n, msg_org, msg_enc);
            }

            // expand and modulate (BPSK)
            for (i=0; i<k; i++) {
                mod_sym[8*i+0] = ((msg_enc[i] >> 7) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+1] = ((msg_enc[i] >> 6) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+2] = ((msg_enc[i] >> 5) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+3] = ((msg_enc[i] >> 4) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+4] = ((msg_enc[i] >> 3) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+5] = ((msg_enc[i] >> 2) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+6] = ((msg_enc[i] >> 1) & 0x01) ? 1.0f : -1.0f;
                mod_sym[8*i+7] = ((msg_enc[i]     ) & 0x01) ? 1.0f : -1.0f;
            }

            // add noise
            for (i=0; i<8*k; i++)
                mod_sym[i] += nstd*randnf()*cexpf(_Complex_I*2*M_PI*randf());

            // 'demodulate' hard/soft
            for (i=0; i<8*k; i++) {
                // hard decision
                msg_rec_hard[i] = ( crealf(mod_sym[i]) > 0.0f ) ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;

                // soft decision
#if 0
                float soft_bit = (255*( 0.5f + 0.5f*tanhf(crealf(mod_sym[i])) ));
                if (soft_bit < 0)        msg_rec_soft[i] = 0;
                else if (soft_bit > 255) msg_rec_soft[i] = 255;
                else                     msg_rec_soft[i] = (unsigned char) soft_bit;
#else
                msg_rec_soft[i] = (unsigned char)(255*( 0.5f + 0.5f*tanhf(crealf(mod_sym[i])) ));
#endif

                //printf("  hard : %3u, soft : %3u\n", msg_rec_hard[i], msg_rec_soft[i]);
            }

            // decode using 'soft' algorithm
            if ( fec_scheme_is_punctured(fs) ) {
                fec_conv_punctured_decode_soft(q, n, msg_rec_hard, msg_dec_hard);
                fec_conv_punctured_decode_soft(q, n, msg_rec_soft, msg_dec_soft);
            } else {
                fec_conv_decode_soft(q, n, msg_rec_hard, msg_dec_hard);
                fec_conv_decode_soft(q, n, msg_rec_soft, msg_dec_soft);
            }
            
            // count bit errors and tabulate results
            bit_errors_hard[s] += count_bit_errors_array(msg_org, msg_dec_hard, n);
            bit_errors_soft[s] += count_bit_errors_array(msg_org, msg_dec_soft, n);
        }

        // print results for this SNR step
        printf("  %8.3f [%6u] %6u %6u\n",
                SNRdB,
                8*n*num_trials,
                bit_errors_soft[s],
                bit_errors_hard[s]);
    }

    // destroy forward error-correction object
    fec_destroy(q);


    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    fprintf(fid,"total_bits = %u;\n", 8*n*num_trials);
    fprintf(fid,"num_snr = %u;\n", num_snr);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    for (i=0; i<num_snr; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"bit_errors_soft(%6u) = %u;\n", i+1, bit_errors_soft[i]);
        fprintf(fid,"bit_errors_hard(%6u) = %u;\n", i+1, bit_errors_hard[i]);
    }
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(SNRdB, bit_errors_soft / total_bits + 1e-12,\n");
    fprintf(fid,"         SNRdB, bit_errors_hard / total_bits + 1e-12);\n");
    fprintf(fid,"axis([%f %f 1e-4 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('soft','hard',1);\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('Bit error rate for %s');\n", fec_scheme_str[fs][1]);
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

#if !LIBFEC_ENABLED
//
// libfec is not installed; need to define internal methods used in this
// program, even though they don't do anything
//
int fec_conv_encode(fec _q,
                    unsigned int _dec_msg_len,
                    unsigned char * _msg_dec,
                    unsigned char * _msg_enc)
{
    return liquid_error(LIQUID_EUMODE,"fec_conv_encode(): libfec not installed; this sandbox program won't run");
}

int fec_conv_decode_soft(fec _q,
                         unsigned int _dec_msg_len,
                         unsigned char * _msg_enc,
                         unsigned char * _msg_dec)
{
    return liquid_error(LIQUID_EUMODE,"fec_conv_decode_soft(): libfec not installed; this sandbox program won't run");
}

int fec_conv_punctured_decode_soft(fec _q,
                                   unsigned int _dec_msg_len,
                                   unsigned char * _msg_enc,
                                   unsigned char * _msg_dec)
{
    return liquid_error(LIQUID_EUMODE,"fec_conv_punctured_decode_soft(): libfec not installed; this sandbox program won't run");
}
#endif

