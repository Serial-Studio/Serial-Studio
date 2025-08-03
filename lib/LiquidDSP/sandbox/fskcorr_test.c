// 
// fskcorr_test.c
//
// This example tests detection of FSK sequences.
//
// Preliminary statistics for detection are at the end of this file

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "fskcorr_test.m"

int main(int argc, char*argv[])
{
    unsigned int    M           = 1024;     // samples per symbol
    unsigned int    n           =  256;     // sequence length (number of symbols)
    unsigned int    p           =  4;       // over-sampling rate for output
    float           SNRdB       = -10.0f;   // SNR (so to speak)
    float           noise_floor = -60.0f;   // noise floor [dB-Hz]
    unsigned int    timer       =   0;      // correlator/FFT alignment
    float           fc          = 0.2f;     // nominal tone frequency

    unsigned int i;
    unsigned int j;

    // derived values
    float nstd = powf(10.0f, noise_floor/20.0f);
    float gain = powf(10.0f, (SNRdB + noise_floor)/20.0f);
    unsigned int i1  = (unsigned int)round(fc*M);
    unsigned int i0  = M - i1;
    printf("M = %u, fc = %.3f {%u,%u}\n", M, fc, i0, i1);

    // generate sequence
    uint8_t seq[n];
    for (i=0; i<n; i++)
        seq[i] = rand() & 1;

    // generate 'matched filter' receiver
    float triangle[2*p];
    for (i=0; i<p; i++) triangle[  i] = (float)(  i) / (float)p;
    for (i=0; i<p; i++) triangle[p+i] = (float)(p-i) / (float)p;
    //for (i=0; i<2*p; i++) printf("%8.3f\n", triangle[i]);
    firinterp_rrrf interp = firinterp_rrrf_create(p, triangle, 2*p);
    float buf_mf[n*p];
    for (i=0; i<n; i++)
        firinterp_rrrf_execute(interp, 2.0f*(float)seq[i] - 1.0f, buf_mf + i*p);
    firinterp_rrrf_destroy(interp);
    //for (i=0; i<n*p; i++) printf("mf(%3u) = %8.3f;\n", i+1, buf_mf[i]);
    // generate buffer and dot product objects for cross-correlation
    dotprod_rrrf xcorr     = dotprod_rrrf_create(buf_mf, n*p);
    windowf      buf_xcorr = windowf_create(p*n);
    float xcorr_norm = liquid_sumsqf(buf_mf, p*n);

    // allocate memory arrays
    float complex * buf_0 = (float complex*) fft_malloc(M*sizeof(float complex));
    float complex * buf_1 = (float complex*) fft_malloc(M*sizeof(float complex));
    float complex * buf_2 = (float complex*) fft_malloc(M*sizeof(float complex));
    float complex * buf_3 = (float complex*) fft_malloc(M*sizeof(float complex));
    windowcf buf_rx = windowcf_create(M);

    // create fft plans
    fftplan ifft = fft_create_plan(M, buf_0, buf_1, LIQUID_FFT_BACKWARD, 0);
    fftplan  fft = fft_create_plan(M, buf_2, buf_3, LIQUID_FFT_FORWARD,  0);

    // fill input with noise
    for (i=0; i<M; i++)
        windowcf_push(buf_rx, nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2);

    // create spectral periodogram
    unsigned int nfft = 2400;
    spgramcf periodogram = spgramcf_create_default(nfft);

    //
    FILE * fid = fopen("fskcorr_test.m","w");
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"n = %u;\n", n);
    fprintf(fid,"p = %u;\n", p);
    for (i=0; i<n; i++)
        fprintf(fid,"seq(%3u) = %1u;\n", i+1, seq[i]);
    fprintf(fid,"p = %u;\n", p);
    fprintf(fid,"%%s = reshape([(2*seq(:)-1) zeros(n,p-1)]',1,[]);\n");
    fprintf(fid,"s = []; for i=1:p, s=[s seq(:)]; end; s=reshape(s',1,[]);\n");
    fprintf(fid,"llr = [];\n");
    for (i=0; i<n+200; i++) {
        // clear input, generate symbol
        memset(buf_0, 0x0, M*sizeof(float complex));
        if (i < n)
            buf_0[ seq[i] ? i1 : i0 ] = gain;
        else
            buf_0[ rand() & 1 ? i1 : i0 ] = gain;
        fft_execute(ifft); // buf_0 -> buf_1

        // add noise
        for (j=0; j<M; j++)
            buf_1[j] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

        // update spectral periodogram
        spgramcf_write(periodogram, buf_1, M);

        // execute...
        for (j=0; j<M; j++) {
            if (!timer) {
                timer = M/p;
                float complex * r;
                windowcf_read(buf_rx, &r);
                memmove(buf_2, r, M*sizeof(float complex));
                fft_execute(fft);
                float r2 = liquid_sumsqcf(buf_3, M);
                float g  = 1.0f / (sqrtf(r2) + 1e-6f);
                float v0 = cabsf(buf_3[i0]) * g + 1e-6f;
                float v1 = cabsf(buf_3[i1]) * g + 1e-6f;
                float llr = log10f( v1 / v0 );
                fprintf(fid,"llr(end+1) = %12.4e;\n", llr);

                // run through cross-correlator
                float rxy = 0;
                windowf_push(buf_xcorr, llr);
                float * rf;
                windowf_read(buf_xcorr, &rf);
                dotprod_rrrf_execute(xcorr, rf, &rxy);
                float y2 = liquid_sumsqf(rf, p*n);
                rxy /= sqrtf(xcorr_norm*y2) + 1e-6f;
                fprintf(fid,"rxy(end+1) = %12.4e;\n", rxy);
               
                printf(" %12.4e { %12.6f, %12.6f } : llr:%12.6f, rxy:%10.6f\n", r2, v0, v1, llr, rxy);
            }
            windowcf_push(buf_rx,buf_1[j]);
            timer--;
        }
    }
    for (i=0; i<n*p; i++) {
        fprintf(fid,"buf_mf(%4u) = %12.4e;\n", i+1, buf_mf[i]);
    }
    // compute power spectral density output
    float psd[nfft];
    spgramcf_get_psd(periodogram, psd);
    for (i=0; i<nfft; i++) {
        fprintf(fid,"psd(%4u) = %12.4e;\n", i+1, psd[i]);
    }
    fprintf(fid,"num_samples = length(llr);\n");
    fprintf(fid,"txy         = ([0:(num_samples-1)] - p*n + 1)/p;\n");
    fprintf(fid,"S = fft(s,  num_samples);\n");
    fprintf(fid,"L = fft(llr,num_samples);\n");
    fprintf(fid,"figure('position',[1 1 800 800]);\n");
    fprintf(fid,"subplot(2,1,1);\n");
    fprintf(fid,"  plot(txy,rxy,'-x');\n");
    fprintf(fid,"  axis([-20 20 -0.2 1.2]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('lag [symbols]');\n");
    fprintf(fid,"  ylabel('cross-correlation output');\n");
    fprintf(fid,"subplot(2,1,2);\n");
    fprintf(fid,"  nfft=length(psd);\n");
    fprintf(fid,"  f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"  plot(f,psd);\n");
    fprintf(fid,"  axis([-0.5 0.5 %f %f]);\n", noise_floor-10, noise_floor+30);
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('normalized frequency [f/F_s]');\n");
    fprintf(fid,"  ylabel('power spectral density [dB]');\n");
    fclose(fid);
    printf("results written to fskcorr_test.m\n");

    fft_free(buf_0);
    fft_free(buf_1);
    fft_free(buf_2);
    fft_free(buf_3);
    fft_destroy_plan(fft);
    fft_destroy_plan(ifft);
    windowcf_destroy(buf_rx);
    dotprod_rrrf_destroy(xcorr);
    windowf_destroy(buf_xcorr);
    spgramcf_destroy(periodogram);

    printf("done.\n");
    return 0;
}

// statistics for detection (no signal)
//
//  M       n       p       rxy<0.999   rxy-max
//  1024      8     4       0.6770      0.8297
//  1024     16     4       0.5038      0.5986
//  1024     32     4       0.3766      0.4831
//  1024     64     4       0.2723      0.3518
//  1024    128     4       0.194       0.2515
//  1024    256     4       0.135       0.1812
//
//  M       n       p       rxy<0.999   rxy-max
//  512       8     4       0.685680    0.805020
//  512      16     4       0.512720    0.617780
//  512      32     4       0.380050    0.522730
//  512      64     4       0.268200    0.350750
//  512     128     4       0.195950    0.264390
//  512     256     4       0.137270    0.172950
//
//  M       n       p       rxy<0.999   rxy-max
//  256       8     4       0.682740   0.798770
//  256      16     4       0.511650   0.606020
//  256      32     4       0.378360   0.456830
//  256      64     4       0.268110   0.366800
//  256     128     4       0.194210   0.265030
//  256     256     4       0.137570   0.167970
//
//  M       n       p       rxy<0.999   rxy-max
//  128       8     4       0.681850   0.800160
//  128      16     4       0.516140   0.611770
//  128      32     4       0.370560   0.473900
//  128      64     4       0.261640   0.332120
//  128     128     4       0.192710   0.238660
//  128     256     4       0.138710   0.175530
//
//  M       n       p       rxy<0.999   rxy-max
//  64        8     4       0.679760   0.817040
//  64       16     4       0.513050   0.622740
//  64       32     4       0.369610   0.501350
//  64       64     4       0.270710   0.384900
//  64      128     4       0.187790   0.280190
//  64      256     4       0.137610   0.184820
//
//  M       n       p       rxy<0.999   rxy-max
//  16        8     4       0.683680   0.763970
//  16       16     4       0.512160   0.613260
//  16       32     4       0.376040   0.504720
//  16       64     4       0.264740   0.335860
//  16      128     4       0.188710   0.260880
//  16      256     4       0.144660   0.190590
