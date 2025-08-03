// 
// eqlms_cccf_test.c
//
// Tests least mean-squares (LMS) equalizer (EQ).
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>
#include "liquid.h"

#define OUTPUT_FILENAME "eqlms_cccf_test.m"

int main() {
    // options
    unsigned int k=2;       // samples/symbol
    unsigned int m=3;
    float beta = 0.3f;
    //float dt = 0.4f;        // timing offset
    unsigned int num_symbols = 256;
    unsigned int hc_len=1;  // channel impulse response length
    unsigned int p=13;      // equalizer filter length
    float mu = 0.05f;

    // TODO : validate input

    // derived values
    unsigned int num_samples = k*num_symbols;

    // data arrays
    float complex s[num_symbols];   // original QPSK symbols
    float complex x[num_samples];   // interpolated samples
    float complex y[num_samples];   // received samples
    float complex z[num_samples];   // recovered samples

    // generate data sequence
    unsigned int i;
    for (i=0; i<num_symbols; i++) {
        s[i] = (rand() % 2 ? 1.0f : -1.0f) +
               (rand() % 2 ? 1.0f : -1.0f) * _Complex_I;
    }

    // interpolate 
    unsigned int h_len = 2*k*m+1;
    float hm[h_len];
#if 1
    // design GMSK
    float c0 = 1.0f / sqrtf(logf(2.0f));
    for (i=0; i<h_len; i++) {
        float t = (float)(i) / (float)(k) - (float)(m);
        //float sig = 0.7f;
        //hm[i] = expf(-t*t / (2*sig*sig) );

        hm[i] = liquid_Qf(2*M_PI*beta*(t-0.5f)*c0) -
                liquid_Qf(2*M_PI*beta*(t+0.5f)*c0);
        printf("h(%3u) = %12.8f\n", i+1, hm[i]);
    }
#else
    design_rrc_filter(k,m,beta,dt,hm);
#endif
    firinterp_crcf interp = firinterp_crcf_create(k,hm,h_len);
    for (i=0; i<num_symbols; i++)
        firinterp_crcf_execute(interp, s[i], &x[i*k]);
    firinterp_crcf_destroy(interp);

    // generate channel filter
    float complex hc[hc_len];
    for (i=0; i<hc_len; i++)
        hc[i] = (i==0) ? 1.0f : 0.1f*randnf()*cexpf(_Complex_I*2*M_PI*randf());

    // push signal through channel...
    firfilt_cccf fchannel = firfilt_cccf_create(hc,hc_len);
    for (i=0; i<num_samples; i++) {
        firfilt_cccf_push(fchannel, x[i]);
        firfilt_cccf_execute(fchannel, &y[i]);
    }
    firfilt_cccf_destroy(fchannel);
    
    // initialize equalizer
#if 1
    // initialize with delta at center
    float complex h[p]; // coefficients
    unsigned int p0 = (p-1)/2;  // center index
    for (i=0; i<p; i++)
        h[i] = (i==p0) ? 1.0f : 0.0f;
#else
    // initialize with square-root raised cosine
    p = 2*k*m+1;
    float h_tmp[p];
    float complex h[p];
    design_rrc_filter(k,m,beta,0.0f,h_tmp);
    for (i=0; i<p; i++)
        h[i] = h_tmp[i] / k;
#endif
    
    // run equalizer
    float complex w0[p];
    float complex w1[p];
    memmove(w0, h, p*sizeof(float complex));
    windowcf buffer = windowcf_create(p);
    for (i=0; i<num_samples; i++) {
        // push value into buffer
        windowcf_push(buffer, y[i]);

        // compute d_hat
        float complex d_hat = 0.0f;
        float complex * r;
        windowcf_read(buffer, &r);
        unsigned int j;
        for (j=0; j<p; j++)
            d_hat += conjf(w0[j]) * r[j];

        // store in output
        z[i] = d_hat;

        // check to see if buffer is full, return if not
        if (i <= p) continue;

        // decimate by k
        if ( (i%k) != 0 ) continue;

        // estimate transmitted QPSK symbol
        float complex d_prime = (crealf(d_hat) > 0.0f ? 1.0f : -1.0f) +
                                (cimagf(d_hat) > 0.0f ? 1.0f : -1.0f) * _Complex_I;

        // compute error
        float complex alpha = d_prime - d_hat;

        // compute signal energy
        float ex2 = 1.414f;

        // update weighting vector
        for (j=0; j<p; j++)
            w1[j] = w0[j] + mu*conjf(alpha)*r[j]/ex2;

        // copy new filter values
        memmove(w0, w1, p*sizeof(complex float));
    }

    // destroy additional objects
    windowcf_destroy(buffer);

    // print results to file
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all\n");
    fprintf(fid,"close all\n");

    fprintf(fid,"k = %u;\n", k);
    fprintf(fid,"m = %u;\n", m);
    fprintf(fid,"num_symbols = %u;\n", num_symbols);
    fprintf(fid,"num_samples = num_symbols*k;\n");

    // save samples
    fprintf(fid,"x = zeros(1,num_samples);\n");
    fprintf(fid,"y = zeros(1,num_samples);\n");
    fprintf(fid,"z = zeros(1,num_samples);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(z[i]), cimagf(z[i]));
    }
    fprintf(fid,"t = 1:num_samples;\n");
    fprintf(fid,"tsym = 1:k:num_samples;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1), plot(t, real(z), tsym, real(z(tsym)),'x');\n");
    fprintf(fid,"subplot(2,1,2), plot(t, imag(z), tsym, imag(z(tsym)),'x');\n");

    // plot symbols
    fprintf(fid,"tsym0 = tsym([1:length(tsym)/2]);\n");
    fprintf(fid,"tsym1 = tsym((length(tsym)/2):end);\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(z(tsym0)),imag(z(tsym0)),'x','Color',[0.7 0.7 0.7],...\n");
    fprintf(fid,"     real(z(tsym1)),imag(z(tsym1)),'x','Color',[0.0 0.0 0.0]);\n");
    fprintf(fid,"xlabel('In-phase');\n");
    fprintf(fid,"ylabel('Quadrature');\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.5);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");

    // save filter coefficients
    for (i=0; i<h_len; i++) fprintf(fid,"hm(%3u) = %12.4e;\n", i+1, hm[i]);
    for (i=0; i<p; i++)     fprintf(fid,"hp(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(w0[i]), cimagf(w0[i]));
    for (i=0; i<hc_len; i++)fprintf(fid,"hc(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(hc[i]), cimagf(hc[i]));

    fprintf(fid,"nfft = 1024;\n");
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"Hm = 20*log10(abs(fftshift(fft(hm/k,nfft))));\n");
    fprintf(fid,"Hc = 20*log10(abs(fftshift(fft(hc,nfft))));\n");
    fprintf(fid,"Hp = 20*log10(abs(fftshift(fft(hp,nfft))));\n");
    fprintf(fid,"G = Hm + Hp + Hc;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,Hm, f,Hc, f,Hp, f,G,'-k','LineWidth',2);\n");
    fprintf(fid,"xlabel('Normalized Frequency');\n");
    fprintf(fid,"ylabel('Power Spectral Density');\n");
    fprintf(fid,"legend('transmit','channel','equalizer','total');\n");
    fprintf(fid,"axis([-0.5 0.5 -10 10]);\n");
    fprintf(fid,"grid on;\n");

    fclose(fid);

    printf("results written to '%s'\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}
