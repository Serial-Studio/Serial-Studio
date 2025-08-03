// 
// iirdes_example.c
//
// Tests infinite impulse response (IIR) digital filter design.
// SEE ALSO: iirdes_analog_example.c
//           iir_filter_crcf_example.c
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "iirdes_example.m"

// print usage/help message
void usage()
{
    printf("iirdes_example -- infinite impulse response filter design\n");
    printf("options (default values in []):\n");
    printf("  u/h   : print usage/help\n");
    printf("  t     : filter type: [butter], cheby1, cheby2, ellip, bessel\n");
    printf("  b     : filter transformation: [LP], HP, BP, BS\n");
    printf("  n     : filter order, n > 0 [5]\n");
    printf("  r     : passband ripple in dB (cheby1, ellip), r > 0 [1.0]\n");
    printf("  s     : stopband attenuation in dB (cheby2, ellip), s > 0 [60.0]\n");
    printf("  f     : passband cut-off, 0 < f < 0.5 [0.2]\n");
    printf("  c     : center frequency (BP, BS cases), 0 < c < 0.5 [0.25]\n");
    printf("  o     : format [sos], tf\n");
    printf("          sos   : second-order sections form\n");
    printf("          tf    : regular transfer function form (potentially\n");
    printf("                  unstable for large orders\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int order=5;   // filter order
    float fc = 0.20f;       // cutoff frequency (low-pass prototype)
    float f0 = 0.25f;       // center frequency (band-pass, band-stop)
    float As = 60.0f;       // stopband attenuation [dB]
    float Ap = 1.0f;        // passband ripple [dB]

    // filter type
    liquid_iirdes_filtertype ftype = LIQUID_IIRDES_BUTTER;

    // band type
    liquid_iirdes_bandtype btype = LIQUID_IIRDES_LOWPASS;

    // output format: second-order sections or transfer function
    liquid_iirdes_format format = LIQUID_IIRDES_SOS;

    int dopt;
    while ((dopt = getopt(argc,argv,"uht:b:n:r:s:f:c:o:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h':
            usage();
            return 0;
        case 't':
            if (strcmp(optarg,"butter")==0) {
                ftype = LIQUID_IIRDES_BUTTER;
            } else if (strcmp(optarg,"cheby1")==0) {
                ftype = LIQUID_IIRDES_CHEBY1;
            } else if (strcmp(optarg,"cheby2")==0) {
                ftype = LIQUID_IIRDES_CHEBY2;
            } else if (strcmp(optarg,"ellip")==0) {
                ftype = LIQUID_IIRDES_ELLIP;
            } else if (strcmp(optarg,"bessel")==0) {
                ftype = LIQUID_IIRDES_BESSEL;
            } else {
                fprintf(stderr,"error: iirdes_example, unknown filter type \"%s\"\n", optarg);
                usage();
                exit(1);
            }
            break;
        case 'b':
            if (strcmp(optarg,"LP")==0) {
                btype = LIQUID_IIRDES_LOWPASS;
            } else if (strcmp(optarg,"HP")==0) {
                btype = LIQUID_IIRDES_HIGHPASS;
            } else if (strcmp(optarg,"BP")==0) {
                btype = LIQUID_IIRDES_BANDPASS;
            } else if (strcmp(optarg,"BS")==0) {
                btype = LIQUID_IIRDES_BANDSTOP;
            } else {
                fprintf(stderr,"error: iirdes_example, unknown band type \"%s\"\n", optarg);
                usage();
                exit(1);
            }
            break;
        case 'n': order = atoi(optarg); break;
        case 'r': Ap = atof(optarg);    break;
        case 's': As = atof(optarg);    break;
        case 'f': fc = atof(optarg);    break;
        case 'c': f0 = atof(optarg);    break;
        case 'o':
            if (strcmp(optarg,"sos")==0) {
                format = LIQUID_IIRDES_SOS;
            } else if (strcmp(optarg,"tf")==0) {
                format = LIQUID_IIRDES_TF;
            } else {
                fprintf(stderr,"error: iirdes_example, unknown output format \"%s\"\n", optarg);
                usage();
                exit(1);
            }
            break;
        default:
            exit(1);
        }
    }

    // validate input
    if (fc <= 0 || fc >= 0.5) {
        fprintf(stderr,"error: %s, cutoff frequency out of range\n", argv[0]);
        usage();
        exit(1);
    } else if (f0 < 0 || f0 > 0.5) {
        fprintf(stderr,"error: %s, center frequency out of range\n", argv[0]);
        usage();
        exit(1);
    } else if (Ap <= 0) {
        fprintf(stderr,"error: %s, pass-band ripple out of range\n", argv[0]);
        usage();
        exit(1);
    } else if (As <= 0) {
        fprintf(stderr,"error: %s, stop-band ripple out of range\n", argv[0]);
        usage();
        exit(1);
    }

    // derived values : compute filter length
    unsigned int N = order; // effective order
    // filter order effectively doubles for band-pass, band-stop
    // filters due to doubling the number of poles and zeros as
    // a result of filter transformation
    if (btype == LIQUID_IIRDES_BANDPASS ||
        btype == LIQUID_IIRDES_BANDSTOP)
    {
        N *= 2;
    }
    unsigned int r = N % 2;     // odd/even order
    unsigned int L = (N-r)/2;   // filter semi-length

    // allocate memory for filter coefficients
    unsigned int h_len = (format == LIQUID_IIRDES_SOS) ? 3*(L+r) : N+1;
    float b[h_len];
    float a[h_len];

    // design filter
    liquid_iirdes(ftype, btype, format, order, fc, f0, Ap, As, b, a);

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    fprintf(fid,"n=%u;\n", order);
    fprintf(fid,"r=%u;\n", r);
    fprintf(fid,"L=%u;\n", L);
    fprintf(fid,"nfft=1024;\n");

    unsigned int i;
    if (format == LIQUID_IIRDES_TF) {
        // print coefficients
        for (i=0; i<=N; i++) printf("a[%3u] = %12.8f;\n", i, a[i]);
        for (i=0; i<=N; i++) printf("b[%3u] = %12.8f;\n", i, b[i]);

        fprintf(fid,"a = zeros(1,n+1);\n");
        fprintf(fid,"b = zeros(1,n+1);\n");
        for (i=0; i<=N; i++) {
            fprintf(fid,"a(%3u) = %12.4e;\n", i+1, a[i]);
            fprintf(fid,"b(%3u) = %12.4e;\n", i+1, b[i]);
        }
        fprintf(fid,"\n");
        fprintf(fid,"H = fft(b,nfft)./fft(a,nfft);\n");
        fprintf(fid,"H = fftshift(H);\n");
        fprintf(fid,"%% group delay\n");
        fprintf(fid,"c = conv(b,fliplr(conj(a)));\n");
        fprintf(fid,"cr = c.*[0:(length(c)-1)];\n");
        fprintf(fid,"t0 = fftshift(fft(cr,nfft));\n");
        fprintf(fid,"t1 = fftshift(fft(c, nfft));\n");
        fprintf(fid,"polebins = find(abs(t1)<1e-6);\n");
        fprintf(fid,"t0(polebins)=0;\n");
        fprintf(fid,"t1(polebins)=1;\n");
        fprintf(fid,"gd = real(t0./t1) - length(a) + 1;\n");

    } else {
        float * B = b;
        float * A = a;
        // print coefficients
        printf("B [%u x 3] :\n", L+r);
        for (i=0; i<L+r; i++)
            printf("  %12.8f %12.8f %12.8f\n", B[3*i+0], B[3*i+1], B[3*i+2]);
        printf("A [%u x 3] :\n", L+r);
        for (i=0; i<L+r; i++)
            printf("  %12.8f %12.8f %12.8f\n", A[3*i+0], A[3*i+1], A[3*i+2]);

        unsigned int j;
        for (i=0; i<L+r; i++) {
            for (j=0; j<3; j++) {
                fprintf(fid,"B(%3u,%3u) = %16.8e;\n", i+1, j+1, B[3*i+j]);
                fprintf(fid,"A(%3u,%3u) = %16.8e;\n", i+1, j+1, A[3*i+j]);
            }
        }
        fprintf(fid,"\n");
        fprintf(fid,"H = ones(1,nfft);\n");
        fprintf(fid,"gd = zeros(1,nfft);\n");
        fprintf(fid,"t0 = zeros(1,nfft);\n");
        fprintf(fid,"t1 = zeros(1,nfft);\n");
        fprintf(fid,"for i=1:(L+r),\n");
        fprintf(fid,"    H = H .* fft(B(i,:),nfft)./fft(A(i,:),nfft);\n");
        fprintf(fid,"    %% group delay\n");
        fprintf(fid,"    c = conv(B(i,:),fliplr(conj(A(i,:))));\n");
        fprintf(fid,"    cr = c.*[0:4];\n");
        fprintf(fid,"    t0 = fftshift(fft(cr,nfft));\n");
        fprintf(fid,"    t1 = fftshift(fft(c, nfft));\n");
        fprintf(fid,"    polebins = find(abs(t1)<1e-6);\n");
        fprintf(fid,"    t0(polebins)=0;\n");
        fprintf(fid,"    t1(polebins)=1;\n");
        fprintf(fid,"    gd = gd + real(t0./t1) - 2;\n");
        fprintf(fid,"end;\n");
        fprintf(fid,"H = fftshift(H);\n");
    }

    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure;\n");

    // plot magnitude response (detail)
    fprintf(fid,"subplot(3,1,1),\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'-','Color',[0.5 0 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 -4 1]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Filter PSD [dB]');\n");

    // plot magnitude response (full range)
    fprintf(fid,"subplot(3,1,2),\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'-','Color',[0.5 0 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 -100 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Filter PSD [dB]');\n");

    // plot group delay
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(f,gd,'-','Color',[0 0.5 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 0 ceil(1.1*max(gd))]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Group delay [samples]');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

