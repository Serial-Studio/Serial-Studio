/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// punctured SEC-DED test
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <getopt.h>

#include "liquid.internal.h"

#define OUTPUT_FILENAME "fec_secded_punctured_test.m"

void usage()
{
    printf("Usage: fec_secded_punctured_test\n");
    printf("  h     : print usage/help\n");
    printf("  p     : input rate, [1:8], default: 4\n");
    printf("  s     : SNR start [dB], default: -5\n");
    printf("  x     : SNR max [dB], default: 5\n");
    printf("  n     : number of SNR steps, default: 21\n");
    printf("  t     : number of trials, default: 1000\n");
}

// encode symbol
void secded7264_encode_symbol_punctured(unsigned char * _sym_dec,
                                        unsigned char * _sym_enc,
                                        unsigned int    _p);

// decode symbol
void secded7264_decode_symbol_punctured(unsigned char * _sym_enc,
                                        unsigned char * _sym_dec,
                                        unsigned int    _p);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    unsigned int p = 4;             // input size: p in [1,8]
    float SNRdB_min =  0.0f;        // minimum SNR
    float SNRdB_max = 10.0f;        // maximum SNR
    unsigned int num_steps = 21;    // number of steps
    unsigned int num_trials = 80000;

    // get command-line options
    int dopt;
    while((dopt = getopt(argc,argv,"hp:s:x:n:t:")) != EOF){
        switch (dopt) {
        case 'h': usage(); return 0;
        case 'p': p          = atoi(optarg);    break;
        case 's': SNRdB_min  = atof(optarg);    break;
        case 'x': SNRdB_max  = atof(optarg);    break;
        case 'n': num_steps  = atoi(optarg);    break;
        case 't': num_trials = atoi(optarg);    break;
        default:
            printf("error: %s, unknown option\n", argv[0]);
            exit(-1);
        }
    }

    // validate input
    if (p < 1 || p >8) {
        fprintf(stderr,"error: %s, p must be in [1,8]\n", argv[0]);
        exit(1);
    } else if (SNRdB_max <= SNRdB_min) {
        fprintf(stderr,"error: %s, invalid SNR range\n", argv[0]);
        exit(1);
    } else if (num_steps < 1) {
        fprintf(stderr,"error: %s, must have at least 1 SNR step\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int n = p*8;           // decoded message length (bits)
    unsigned int k = (p+1)*8;       // encoded message length (bits)
    float rate = (float)n / (float)k;
    float SNRdB_step = (SNRdB_max - SNRdB_min) / (num_steps-1);

    unsigned int i;
    
    // arrays
    unsigned char m[p];     // original message [64 x 1]
    unsigned char v[p+1];   // encoded/transmitted message
    float complex y[k];     // received message with noise
    unsigned char r[p+1];   // received vector (hard decision)
    unsigned char m_hat[p]; // estimated original message

#if 0
    // test it out
    for (i=0; i<p; i++)
        m[i] = rand() & 0xff;

    secded7264_encode_symbol_punctured(m,v,p);

    // add error
    v[0] ^= 0x01;

    // decode
    secded7264_decode_symbol_punctured(v,m_hat,p);

    // print
    printf("m:      ");
    for (i=0; i<p; i++) { liquid_print_bitstring(m[i], 8); printf(" "); }
    printf("\n");

    printf("m-hat:  ");
    for (i=0; i<p; i++) { liquid_print_bitstring(m_hat[i], 8); printf(" "); }
    printf("\n");

    // count errors
    printf("decoded errors : %3u / %3u\n", count_bit_errors_array(m,m_hat,p), p*8);
#else
    // data arrays
    unsigned int num_bit_errors[num_steps];

    unsigned int t;
    unsigned int s;
    printf("  %8s %8s [%8s] %8s %12s %12s\n", "SNR [dB]", "Eb/N0", "trials", "# errs", "(BER)", "uncoded");
    for (s=0; s<num_steps; s++) {
        // compute SNR
        float SNRdB = SNRdB_min + s*SNRdB_step;

        // noise standard deviation
        float sigma = powf(10.0f, -SNRdB/20.0f);

        // reset error counter
        num_bit_errors[s] = 0;

        for (t=0; t<num_trials; t++) {
            // generate original message signal
            for (i=0; i<p; i++)
                m[i] = rand() & 0xff;

            // compute encoded message
            secded7264_encode_symbol_punctured(m,v,p);

            // expand to bits
            for (i=0; i<p+1; i++) {
                y[8*i+0] = (v[i] & 0x80) ? 1.0f : -1.0f;
                y[8*i+1] = (v[i] & 0x40) ? 1.0f : -1.0f;
                y[8*i+2] = (v[i] & 0x20) ? 1.0f : -1.0f;
                y[8*i+3] = (v[i] & 0x10) ? 1.0f : -1.0f;
                y[8*i+4] = (v[i] & 0x08) ? 1.0f : -1.0f;
                y[8*i+5] = (v[i] & 0x04) ? 1.0f : -1.0f;
                y[8*i+6] = (v[i] & 0x02) ? 1.0f : -1.0f;
                y[8*i+7] = (v[i] & 0x01) ? 1.0f : -1.0f;
            }

            // add noise
            for (i=0; i<k; i++)
                y[i] += sigma*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

            // demodulate (hard decision)
            for (i=0; i<p+1; i++) {
                r[i] = 0x00;

                r[i] |= ( crealf(y[8*i+0]) > 0 ) ? 0x80 : 0x00;
                r[i] |= ( crealf(y[8*i+1]) > 0 ) ? 0x40 : 0x00;
                r[i] |= ( crealf(y[8*i+2]) > 0 ) ? 0x20 : 0x00;
                r[i] |= ( crealf(y[8*i+3]) > 0 ) ? 0x10 : 0x00;
                r[i] |= ( crealf(y[8*i+4]) > 0 ) ? 0x08 : 0x00;
                r[i] |= ( crealf(y[8*i+5]) > 0 ) ? 0x04 : 0x00;
                r[i] |= ( crealf(y[8*i+6]) > 0 ) ? 0x02 : 0x00;
                r[i] |= ( crealf(y[8*i+7]) > 0 ) ? 0x01 : 0x00;
            }

            // decode
            secded7264_decode_symbol_punctured(r,m_hat,p);

            // compute errors in decoded message signal
            num_bit_errors[s] += count_bit_errors_array(m,m_hat,p);
        }

        // print results for this SNR step
        printf("  %8.3f %8.3f [%8u] %8u %12.4e %12.4e\n",
                SNRdB,
                SNRdB - 10*log10f(rate),
                n*num_trials,
                num_bit_errors[s], (float)(num_bit_errors[s]) / (float)(num_trials*n),
                0.5f*erfcf(1.0f/sigma));
    }

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
    fprintf(fid,"r = n / k;\n");
    fprintf(fid,"num_steps = %u;\n", num_steps);
    fprintf(fid,"num_trials = %u;\n", num_trials);
    fprintf(fid,"num_bit_trials = num_trials*n;\n");
    for (i=0; i<num_steps; i++) {
        fprintf(fid,"SNRdB(%4u) = %12.8f;\n",i+1, SNRdB_min + i*SNRdB_step);
        fprintf(fid,"num_bit_errors(%6u) = %u;\n", i+1, num_bit_errors[i]);
    }
    fprintf(fid,"EbN0dB = SNRdB - 10*log10(r);\n");
    fprintf(fid,"EbN0dB_bpsk = -15:0.5:40;\n");
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"semilogy(EbN0dB_bpsk, 0.5*erfc(sqrt(10.^[EbN0dB_bpsk/10]))+1e-12,'-x',\n");
    fprintf(fid,"         EbN0dB,      num_bit_errors / num_bit_trials + 1e-12,  '-x');\n");
    fprintf(fid,"axis([%f (%f-10*log10(r)) 1e-6 1]);\n", SNRdB_min, SNRdB_max);
    fprintf(fid,"legend('uncoded','coded',1);\n");
    fprintf(fid,"xlabel('E_b/N_0 [dB]');\n");
    fprintf(fid,"ylabel('Bit Error Rate');\n");
    fprintf(fid,"title('BER vs. E_b/N_0 for (71,64,%u) r=%8.4f code');\n", 8*p, rate);
    fprintf(fid,"grid on;\n");

    fclose(fid);
    printf("results written to %s\n", OUTPUT_FILENAME);
#endif

    return 0;
}


// encode symbol
void secded7264_encode_symbol_punctured(unsigned char * _sym_dec,
                                        unsigned char * _sym_enc,
                                        unsigned int    _p)
{
    unsigned int i;
    
    // compute parity
    unsigned char m[8];
    for (i=0; i<8; i++)
        m[i] = (i < _p) ? _sym_dec[i] : 0x00;
    unsigned char parity = fec_secded7264_compute_parity(m);

    // copy input message
    _sym_enc[0] = parity;
    for (i=0; i<_p; i++)
        _sym_enc[i+1] = _sym_dec[i];
}

// decode symbol
void secded7264_decode_symbol_punctured(unsigned char * _sym_enc,
                                        unsigned char * _sym_dec,
                                        unsigned int    _p)
{
    unsigned int i;

    // strip input, padding with zeros
    unsigned char v[9];
    for (i=0; i<9; i++)
        v[i] = (i <= _p) ? _sym_enc[i] : 0x00;

    // decode
    unsigned char m_hat[8];
    fec_secded7264_decode_symbol(v, m_hat);

    // copy to output
    for (i=0; i<_p; i++)
        _sym_dec[i] = m_hat[i];
}

