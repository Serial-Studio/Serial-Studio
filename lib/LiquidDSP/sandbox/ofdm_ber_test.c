// test OFDM bit error rate
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include "liquid.h"

void usage()
{
    printf("ofdm_ber_test [options]\n");
    printf("  h     : print usage\n");
    printf("  s     : signal-to-noise ratio [dB], default: 6.5\n");
    printf("  M     : number of subcarriers (must be even), default: 64\n");
    printf("  C     : cyclic prefix length, default: 16\n");
    printf("  m     : modulation scheme (bpsk default)\n");
    liquid_print_modulation_schemes();
    printf("  n     : number of OFDM symbols, default: 40\n");
    printf("  c     : number of channel taps, default: 1\n");
}

int main(int argc, char*argv[])
{
    // set random number generator seed
    srand(time(NULL));

    // options
    unsigned int M = 64;                // number of subcarriers
    unsigned int cp_len = 16;           // cyclic prefix length
    modulation_scheme ms = LIQUID_MODEM_BPSK;
    float SNRdB = 6.5f;                 // signal-to-noise ratio [dB]
    unsigned int hc_len = 1;            // channel impulse response length
    unsigned int num_symbols = 40;      // number of OFDM symbols

    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"hs:M:C:m:n:c:")) != EOF){
        switch (dopt) {
        case 'h': usage(); return 0;
        case 's': SNRdB  = atof(optarg); break;
        case 'M': M      = atoi(optarg); break;
        case 'C': cp_len = atoi(optarg); break;
        case 'm':
            ms = liquid_getopt_str2mod(optarg);
            if (ms == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown/unsupported mod. scheme: %s\n", argv[0], optarg);
                exit(-1);
            }
            break;
        case 'n': num_symbols = atoi(optarg); break;
        case 'c': hc_len      = atoi(optarg); break;
        default:
            exit(-1);
        }
    }

    unsigned int i;

    // validate options
    if (M < 4) {
        fprintf(stderr,"error: %s, must have at least 4 subcarriers\n", argv[0]);
        exit(1);
    } else if (hc_len == 0) {
        fprintf(stderr,"error: %s, must have at least 1 channel tap\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int symbol_len = M + cp_len;
    float nstd = powf(10.0f, -SNRdB/20.0f);
    float fft_gain = 1.0f / sqrtf(M);   // 'gain' due to taking FFT
    
    // buffers
    unsigned int sym_in[M];             // input data symbols
    unsigned int sym_out[M];            // output data symbols
    float complex x[M];                 // time-domain buffer
    float complex X[M];                 // freq-domain buffer
    float complex buffer[symbol_len];   // 

    // create modulator/demodulator objects
    modemcf mod   = modemcf_create(ms);
    modemcf demod = modemcf_create(ms);
    unsigned int bps = modemcf_get_bps(mod);  // modem bits/symbol

    // create channel filter (random taps)
    float complex hc[hc_len];
    hc[0] = 1.0f;
    for (i=1; i<hc_len; i++)
        hc[i] = 0.1f * (randnf() + _Complex_I*randnf());
    firfilt_cccf fchannel = firfilt_cccf_create(hc, hc_len);

    //
    unsigned int n;
    unsigned int num_bit_errors = 0;
    for (n=0; n<num_symbols; n++) {
        // generate random data symbols and modulate onto subcarriers
        for (i=0; i<M; i++) {
            sym_in[i] = rand() % (1<<bps);

            modemcf_modulate(mod, sym_in[i], &X[i]);
        }

        // run inverse transform
        fft_run(M, X, x, LIQUID_FFT_BACKWARD, 0);

        // scale by FFT gain so E{|x|^2} = 1
        for (i=0; i<M; i++)
            x[i] *= fft_gain;

        // apply channel impairments
        for (i=0; i<M + cp_len; i++) {
            // push samples through channel filter, starting with cyclic prefix
            firfilt_cccf_push(fchannel, x[(M-cp_len+i)%M]);

            // compute output
            firfilt_cccf_execute(fchannel, &buffer[i]);

            // add noise
            buffer[i] += nstd*( randnf() + _Complex_I*randnf() ) * M_SQRT1_2;
        }

        // run forward transform
        fft_run(M, &buffer[cp_len], X, LIQUID_FFT_FORWARD, 0);

        // TODO : apply equalizer to 'X' here

        // demodulate and compute bit errors
        for (i=0; i<M; i++) {
            // scale by fft size
            X[i] *= fft_gain;

            modemcf_demodulate(demod, X[i], &sym_out[i]);

            num_bit_errors += liquid_count_ones(sym_in[i] ^ sym_out[i]);
        }
    }

    // destroy objects
    modemcf_destroy(mod);
    modemcf_destroy(demod);
    firfilt_cccf_destroy(fchannel);

    // print results
    unsigned int total_bits = M*bps*num_symbols;
    float ber = (float)num_bit_errors / (float)total_bits;
    printf("  bit errors : %6u / %6u (%12.4e)\n", num_bit_errors, total_bits, ber);

    printf("done.\n");
    return 0;
}

