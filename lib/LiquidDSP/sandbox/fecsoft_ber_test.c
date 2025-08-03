// 
// fecsoft_ber_test.c
//
// Simulate error rate using soft vs. hard decoding
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <time.h>
#include <getopt.h>

#include "liquid.h"

#define OUTPUT_FILENAME "fecsoft_ber_test.m"

void usage()
{
    printf("fec_ber\n");
    printf("  Simulates soft decoding\n");
    printf("options:\n");
    printf("  u/h   : print usage/help\n");
    printf("  s     : SNR start [dB], default: -2\n");
    printf("  x     : SNR max [dB], default: 13\n");
    printf("  n     : number of SNR steps, default: 16\n");
    printf("  t     : number of trials, default: 800\n");
    printf("  f     : frame size, default: 64\n");
    printf("  c     : coding scheme, (h74 default):\n");
    liquid_print_fec_schemes();
}

int main(int argc, char *argv[]) {
    // set initial seed to random
    srand(time(NULL));

    // options
    unsigned int n = 64;                    // frame size (bytes)
    float SNRdB_min = -2.0f;                // signal-to-noise ratio (minimum)
    float SNRdB_max = 13.0f;                // signal-to-noise ratio (maximum)
    unsigned int num_snr = 16;              // number of SNR steps
    unsigned int num_trials=800;            // number of trials
    fec_scheme fs = LIQUID_FEC_HAMMING74;   // error-correcting scheme

    // get command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"uhs:x:n:t:f:c:")) != EOF){
        switch (dopt) {
        case 'h':
        case 'u': usage(); return 0;
        case 's': SNRdB_min = atof(optarg);     break;
        case 'x': SNRdB_max = atof(optarg);     break;
        case 'n': num_snr = atoi(optarg);       break;
        case 't': num_trials = atoi(optarg);    break;
        case 'f': n = atoi(optarg);             break;
        case 'c':
            fs = liquid_getopt_str2fec(optarg);
            if (fs == LIQUID_FEC_UNKNOWN) {
                fprintf(stderr,"error: unknown/unsupported fec scheme \"%s\"\n\n",optarg);
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    unsigned int i;

    // create arrays
    unsigned int n_enc = fec_get_enc_msg_length(fs,n);
    printf("dec msg len : %u\n", n);
    printf("enc msg len : %u\n", n_enc);
    float rate = (float)n / (float)n_enc;
    unsigned char msg_org[n];            // original data message
    unsigned char msg_enc[n_enc];        // encoded data message
    float complex sym_rec[8*n_enc];      // received BPSK symbols
    unsigned char msg_cor_soft[8*n_enc]; // corrupted data message (soft bits)
    unsigned char msg_cor_hard[n_enc];   // corrupted data message (hard bits)
    unsigned char msg_dec_soft[n];       // decoded data message (soft bits)
    unsigned char msg_dec_hard[n];       // decoded data message (soft bits)

    // create object
    fec q = fec_create(fs,NULL);
    fec_print(q);

    unsigned int bit_errors_soft[num_snr];
    unsigned int bit_errors_hard[num_snr];

    //
    // set up parameters
    //
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_snr-1);

    // 
    // start trials
    //
    
    printf("  %8s %8s [%8s] %8s %12s %8s %12s %12s\n",
            "SNR [dB]", "Eb/N0", "trials", "soft", "(BER)", "hard", "(BER)", "uncoded");
    unsigned int s;
    for (s=0; s<num_snr; s++) {
        // compute SNR for this level
        float SNRdB = SNRdB_min + s*SNRdB_step; // SNR in dB for this round
        float nstd = powf(10.0f, -SNRdB/20.0f); // noise standard deviation

        // reset results
        bit_errors_soft[s] = 0;
        bit_errors_hard[s] = 0;

        unsigned int t;
        for (t=0; t<num_trials; t++) {
            // generate data
            for (i=0; i<n; i++)
                msg_org[i] = rand() & 0xff;

            // encode message
            fec_encode(q, n, msg_org, msg_enc);

            // modulate with BPSK
            for (i=0; i<n_enc; i++) {
                sym_rec[8*i+0] = (msg_enc[i] & 0x80) ? 1.0f : -1.0f;
                sym_rec[8*i+1] = (msg_enc[i] & 0x40) ? 1.0f : -1.0f;
                sym_rec[8*i+2] = (msg_enc[i] & 0x20) ? 1.0f : -1.0f;
                sym_rec[8*i+3] = (msg_enc[i] & 0x10) ? 1.0f : -1.0f;
                sym_rec[8*i+4] = (msg_enc[i] & 0x08) ? 1.0f : -1.0f;
                sym_rec[8*i+5] = (msg_enc[i] & 0x04) ? 1.0f : -1.0f;
                sym_rec[8*i+6] = (msg_enc[i] & 0x02) ? 1.0f : -1.0f;
                sym_rec[8*i+7] = (msg_enc[i] & 0x01) ? 1.0f : -1.0f;
            }

            // add noise
            for (i=0; i<8*n_enc; i++)
                sym_rec[i] += nstd*(randnf() + _Complex_I*randf())*M_SQRT1_2;

            // demodulate using LLR
            for (i=0; i<8*n_enc; i++) {
                float LLR = 2.0f * crealf(sym_rec[i]);
                int soft_bit = (int) (16*LLR + 127);
                if (soft_bit > 255) soft_bit = 255;
                if (soft_bit <   0) soft_bit = 0;
                msg_cor_soft[i] = (unsigned char)(soft_bit);
            }

            // convert to hard bits (hard decoding)
            for (i=0; i<n_enc; i++) {
                msg_cor_hard[i] = 0x00;

                msg_cor_hard[i] |= crealf(sym_rec[8*i+0]) > 0.0f ? 0x80 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+1]) > 0.0f ? 0x40 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+2]) > 0.0f ? 0x20 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+3]) > 0.0f ? 0x10 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+4]) > 0.0f ? 0x08 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+5]) > 0.0f ? 0x04 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+6]) > 0.0f ? 0x02 : 0x00;
                msg_cor_hard[i] |= crealf(sym_rec[8*i+7]) > 0.0f ? 0x01 : 0x00;
            }

            // decode
            fec_decode(     q, n, msg_cor_hard, msg_dec_hard);
            fec_decode_soft(q, n, msg_cor_soft, msg_dec_soft);
            
            // tabulate results
            bit_errors_soft[s] += count_bit_errors_array(msg_org, msg_dec_soft, n);
            bit_errors_hard[s] += count_bit_errors_array(msg_org, msg_dec_hard, n);
        }

        // print results for this SNR step
        printf("  %8.3f %8.3f [%8u] %8u %12.4e %8u %12.4e %12.4e\n",
                SNRdB,
                SNRdB - 10*log10f(rate),
                8*n*num_trials,
                bit_errors_soft[s], (float)(bit_errors_soft[s]) / (float)(num_trials*n*8),
                bit_errors_hard[s], (float)(bit_errors_hard[s]) / (float)(num_trials*n*8),
                0.5f*erfcf(1.0f/nstd));
    }

    // clean up objects
    fec_destroy(q);

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME, "w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"\n\n");
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");
    fprintf(fid,"n = %u;    %% frame size [bytes]\n", n);
    fprintf(fid,"k = %u;    %% encoded frame size [bytes]\n", n_enc);
    fprintf(fid,"r = n / k; %% true rate\n");
    fprintf(fid,"num_snr = %u;\n", num_snr);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    fprintf(fid,"num_bit_trials = num_trials*n*8;\n");
    for (i=0; i<num_snr; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"bit_errors_soft(%6u) = %u;\n", i+1, bit_errors_soft[i]);
        fprintf(fid,"bit_errors_hard(%6u) = %u;\n", i+1, bit_errors_hard[i]);
    }
    fprintf(fid,"EbN0dB = SNRdB - 10*log10(r);\n");
    fprintf(fid,"EbN0dB_bpsk = -15:0.5:40;\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(EbN0dB_bpsk, 0.5*erfc(sqrt(10.^[EbN0dB_bpsk/10]))+1e-12,'-x',\n");
    fprintf(fid,"         EbN0dB,      bit_errors_soft / num_bit_trials + 1e-12,  '-x',\n");
    fprintf(fid,"         EbN0dB,      bit_errors_hard / num_bit_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f (%f-10*log10(r)) 1e-6 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('uncoded','soft','hard',1);\n");
    fprintf(fid,"xlabel('E_b/N_0 [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('BER vs. E_b/N_0 for %s');\n", fec_scheme_str[fs][1]);
    fprintf(fid,"grid on;\n");

    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(EbN0dB_bpsk, 0.5*erfc(sqrt(10.^[EbN0dB_bpsk/10]))+1e-12,'-x',\n");
    fprintf(fid,"         SNRdB,       bit_errors_soft / num_bit_trials + 1e-12,  '-x',\n");
    fprintf(fid,"         SNRdB,       bit_errors_hard / num_bit_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f %f 1e-6 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('uncoded','soft','hard',1);\n");
    fprintf(fid,"xlabel('SNR [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('BER vs. SNR for %s');\n", fec_scheme_str[fs][1]);
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

