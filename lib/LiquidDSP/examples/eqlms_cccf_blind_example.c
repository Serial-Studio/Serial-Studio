// This example tests the least mean-squares (LMS) equalizer (EQ) on a
// signal with an unknown modulation and carrier frequency offset. That
// is, the equalization is done completely blind of the modulation
// scheme or its underlying data set. The error estimate assumes a
// constant modulus linear modulation scheme. This works surprisingly
// well even more amplitude-modulated signals, e.g. 'qam16'.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include <getopt.h>
#include <time.h>
#include "liquid.h"

#define OUTPUT_FILENAME "eqlms_cccf_blind_example.m"

// print usage/help message
void usage()
{
    printf("Usage: eqlms_cccf_blind_example [OPTION]\n");
    printf("  h     : print help\n");
    printf("  n     : number of symbols, default: 500\n");
    printf("  s     : SNR [dB], default: 30\n");
    printf("  c     : number of channel filter taps (minimum: 1), default: 5\n");
    printf("  k     : samples/symbol, default: 2\n");
    printf("  m     : filter semi-length (symbols), default: 4\n");
    printf("  b     : filter excess bandwidth factor, default: 0.3\n");
    printf("  p     : equalizer semi-length (symbols), default: 3\n");
    printf("  u     : equalizer learning rate, default; 0.05\n");
    printf("  M     : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
}

int main(int argc, char*argv[])
{
    //srand(time(NULL));

    // options
    unsigned int num_symbols=800; // number of symbols to observe
    float SNRdB = 30.0f;          // signal-to-noise ratio [dB]
    float fc    = 0.002f;         // carrier offset
    unsigned int hc_len=5;        // channel filter length
    unsigned int k=2;             // matched filter samples/symbol
    unsigned int m=3;             // matched filter delay (symbols)
    float beta=0.3f;              // matched filter excess bandwidth factor
    unsigned int p=3;             // equalizer length (symbols, hp_len = 2*k*p+1)
    float mu = 0.08f;             // equalizer learning rate

    // modulation type/depth
    modulation_scheme ms = LIQUID_MODEM_QPSK;

    int dopt;
    while ((dopt = getopt(argc,argv,"hn:s:c:k:m:b:p:u:M:")) != EOF) {
        switch (dopt) {
        case 'h': usage();                      return 0;
        case 'n': num_symbols   = atoi(optarg); break;
        case 's': SNRdB         = atof(optarg); break;
        case 'c': hc_len        = atoi(optarg); break;
        case 'k': k             = atoi(optarg); break;
        case 'm': m             = atoi(optarg); break;
        case 'b': beta          = atof(optarg); break;
        case 'p': p             = atoi(optarg); break;
        case 'u': mu            = atof(optarg); break;
        case 'M':
            ms = liquid_getopt_str2mod(optarg);
            if (ms == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown/unsupported modulation scheme '%s'\n", argv[0], optarg);
                return 1;
            }
            break;
        default:
            exit(1);
        }
    }

    // validate input
    if (num_symbols == 0) {
        fprintf(stderr,"error: %s, number of symbols must be greater than zero\n", argv[0]);
        exit(1);
    } else if (hc_len == 0) {
        fprintf(stderr,"error: %s, channel must have at least 1 tap\n", argv[0]);
        exit(1);
    } else if (k < 2) {
        fprintf(stderr,"error: %s, samples/symbol must be at least 2\n", argv[0]);
        exit(1);
    } else if (m == 0) {
        fprintf(stderr,"error: %s, filter semi-length must be at least 1 symbol\n", argv[0]);
        exit(1);
    } else if (beta < 0.0f || beta > 1.0f) {
        fprintf(stderr,"error: %s, filter excess bandwidth must be in [0,1]\n", argv[0]);
        exit(1);
    } else if (p == 0) {
        fprintf(stderr,"error: %s, equalizer semi-length must be at least 1 symbol\n", argv[0]);
        exit(1);
    } else if (mu < 0.0f || mu > 1.0f) {
        fprintf(stderr,"error: %s, equalizer learning rate must be in [0,1]\n", argv[0]);
        exit(1);
    }

    // derived values
    unsigned int hm_len = 2*k*m+1;   // matched filter length
    unsigned int hp_len = 2*k*p+1;   // equalizer filter length
    unsigned int num_samples = k*num_symbols;

    // bookkeeping variables
    float complex syms_tx[num_symbols]; // transmitted data symbols
    float complex x[num_samples];       // interpolated time series
    float complex y[num_samples];       // channel output
    float complex z[num_samples];       // equalized output
    float complex syms_rx[num_symbols]; // received data symbols

    float hm[hm_len];                   // matched filter response
    float complex hc[hc_len];           // channel filter coefficients
    float complex hp[hp_len];           // equalizer filter coefficients

    unsigned int i;

    // generate matched filter response
    liquid_firdes_prototype(LIQUID_FIRFILT_RRC, k, m, beta, 0.0f, hm);
    firinterp_crcf interp = firinterp_crcf_create(k, hm, hm_len);

    // create the modem objects
    modemcf mod   = modemcf_create(ms);
    modemcf demod = modemcf_create(ms);
    unsigned int M = 1 << modemcf_get_bps(mod);

    // generate channel impulse response, filter
    hc[0] = 1.0f;
    for (i=1; i<hc_len; i++)
        hc[i] = 0.09f*(randnf() + randnf()*_Complex_I);
    firfilt_cccf fchannel = firfilt_cccf_create(hc, hc_len);

    // generate random symbols
    for (i=0; i<num_symbols; i++)
        modemcf_modulate(mod, rand()%M, &syms_tx[i]);

    // interpolate
    for (i=0; i<num_symbols; i++)
        firinterp_crcf_execute(interp, syms_tx[i], &x[i*k]);
    
    // push through channel
    float nstd = powf(10.0f, -SNRdB/20.0f);
    for (i=0; i<num_samples; i++) {
        firfilt_cccf_push(fchannel, x[i]);
        firfilt_cccf_execute(fchannel, &y[i]);

        // add carrier offset and noise
        y[i] *= cexpf(_Complex_I*2*M_PI*fc*i);
        y[i] += nstd*(randnf() + randnf()*_Complex_I)*M_SQRT1_2;
    }

    // push through equalizer
    // create equalizer, initialized with square-root Nyquist filter
    eqlms_cccf eq = eqlms_cccf_create_rnyquist(LIQUID_FIRFILT_RRC, k, p, beta, 0.0f);
    eqlms_cccf_set_bw(eq, mu);

    // get initialized weights
    eqlms_cccf_copy_coefficients(eq, hp);

    // filtered error vector magnitude (empirical RMS error)
    float evm_hat = 0.03f;

    // nco/pll for phase recovery
    nco_crcf nco = nco_crcf_create(LIQUID_VCO);
    nco_crcf_pll_set_bandwidth(nco, 0.02f);

    float complex d_hat = 0.0f;
    unsigned int num_symbols_rx = 0;
    for (i=0; i<num_samples; i++) {
        // print filtered evm (empirical rms error)
        if ( ((i+1)%50)==0 )
            printf("%4u : rms error = %12.8f dB\n", i+1, 10*log10(evm_hat));

        eqlms_cccf_push(eq, y[i]);
        eqlms_cccf_execute(eq, &d_hat);

        // store output
        z[i] = d_hat;

        // decimate by k
        if ( (i%k) != 0 ) continue;

        // update equalizer independent of the signal: estimate error
        // assuming constant modulus signal
        eqlms_cccf_step_blind(eq, d_hat);

        // apply carrier recovery
        float complex v;
        nco_crcf_mix_down(nco, d_hat, &v);

        // save resulting data symbol
        if (num_symbols_rx < num_symbols)
            syms_rx[num_symbols_rx++] = v;

        // demodulate
        unsigned int sym_out;   // output symbol
        float complex d_prime;  // estimated input sample
        modemcf_demodulate(demod, v, &sym_out);
        modemcf_get_demodulator_sample(demod, &d_prime);
        float phase_error = modemcf_get_demodulator_phase_error(demod);

        // update pll
        nco_crcf_pll_step(nco, phase_error);

        // update rx nco object
        nco_crcf_step(nco);

        // update filtered evm estimate
        float evm = crealf( (d_prime-v)*conjf(d_prime-v) );
        evm_hat = 0.98f*evm_hat + 0.02f*evm;
    }

    // get equalizer weights
    eqlms_cccf_copy_coefficients(eq, hp);

    // destroy objects
    eqlms_cccf_destroy(eq);
    nco_crcf_destroy(nco);
    firinterp_crcf_destroy(interp);
    firfilt_cccf_destroy(fchannel);
    modemcf_destroy(mod);
    modemcf_destroy(demod);

    // 
    // export output
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = num_symbols*k;\n");

    // save transmit matched-filter response
    fprintf(fid,"hm_len = 2*k*m+1;\n");
    fprintf(fid,"hm = zeros(1,hm_len);\n");
    for (i=0; i<hm_len; i++)
        fprintf(fid,"hm(%4u) = %12.4e;\n", i+1, hm[i]);

    // save channel impulse response
    fprintf(fid,"hc_len = %u;\n", hc_len);
    fprintf(fid,"hc = zeros(1,hc_len);\n");
    for (i=0; i<hc_len; i++)
        fprintf(fid,"hc(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(hc[i]), cimagf(hc[i]));

    // save equalizer response
    fprintf(fid,"hp_len = %u;\n", hp_len);
    fprintf(fid,"hp = zeros(1,hp_len);\n");
    for (i=0; i<hp_len; i++)
        fprintf(fid,"hp(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(hp[i]), cimagf(hp[i]));

    // save sample sets
    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    fprintf(fid,"z = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));
    }
    fprintf(fid,"syms_rx = zeros(1,num_symbols);\n");
    for (i=0; i<num_symbols; i++) {
        fprintf(fid,"syms_rx(%4u) = %12.4e + j*%12.4e;\n", i+1, crealf(syms_rx[i]), cimagf(syms_rx[i]));
    }

    // plot time response
    fprintf(fid,"t = 0:(num_samples-1);\n");
    fprintf(fid,"tsym = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(t,real(z),...\n");
    fprintf(fid,"     t(tsym),real(z(tsym)),'x');\n");

    // plot constellation
    fprintf(fid,"syms_rx_0 = syms_rx(1:(length(syms_rx)/2));\n");
    fprintf(fid,"syms_rx_1 = syms_rx((length(syms_rx)/2):end);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(syms_rx_0),imag(syms_rx_0),'x','Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"     real(syms_rx_1),imag(syms_rx_1),'x','Color',[1 1 1]*0.0);\n");
    fprintf(fid,"xlabel('In-Phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");

    // compute composite response
    fprintf(fid,"g  = real(conv(conv(hm,hc),hp));\n");

    // plot responses
    fprintf(fid,"nfft = 1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"Hm = 20*log10(abs(fftshift(fft(hm/k,nfft))));\n");
    fprintf(fid,"Hc = 20*log10(abs(fftshift(fft(hc,  nfft))));\n");
    fprintf(fid,"Hp = 20*log10(abs(fftshift(fft(hp,  nfft))));\n");
    fprintf(fid,"G  = 20*log10(abs(fftshift(fft(g/k, nfft))));\n");

    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,Hm, f,Hc, f,Hp, f,G,'-k','LineWidth',2, [-0.5/k 0.5/k],[-6.026 -6.026],'or');\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('Power Spectral Density');\n");
    fprintf(fid,"legend('transmit','channel','equalizer','composite','half-power points','location','northeast');\n");
    fprintf(fid,"axis([-0.5 0.5 -12 8]);\n");
    fprintf(fid,"grid on;\n");
    
    fclose(fid);
    printf("results written to '%s'\n", OUTPUT_FILENAME);

    return 0;
}
