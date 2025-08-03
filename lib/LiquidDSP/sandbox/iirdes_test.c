// iirdes_example.c
//
// Tests infinite impulse response (IIR) filter design.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "iirdes_test.m"

// print usage/help message
void usage()
{
    printf("iirdes_example -- infinite impulse response filter design\n");
    printf("options (default values in []):\n");
    printf("  -h          : print usage/help\n");
    printf("  -t <type>   : filter type: [butter], cheby1, cheby2, ellip, bessel\n");
    printf("  -b <type>   : filter transformation: [LP], HP, BP, BS\n");
    printf("  -n <order>  : filter order, n > 0 [5]\n");
    printf("  -r <ripple> : passband ripple in dB (cheby1, ellip), r > 0 [1.0]\n");
    printf("  -s <atten>  : stopband attenuation in dB (cheby2, ellip), s > 0 [60.0]\n");
    printf("  -f <freq>   : passband cut-off, 0 < f < 0.5 [0.2]\n");
    printf("  -c <freq>   : center frequency (BP, BS cases), 0 < c < 0.5 [0.25]\n");
    printf("  -o <format> : format [sos], tf\n");
    printf("      sos - second-order sections form\n");
    printf("      tf  - regular transfer function form (potentially\n");
    printf("            unstable for large orders\n");
}


int main(int argc, char*argv[]) {
    // options
    unsigned int n=5;       // filter order
    float fc = 0.20f;       // cutoff frequency (low-pass prototype)
    float f0 = 0.25f;       // center frequency (band-pass, band-stop)
    float As = 60.0f;       // stopband attenuation [dB]
    float ripple = 1.0f;    // passband ripple [dB]

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
        case 'n': n = atoi(optarg);         break;
        case 'r': ripple = atof(optarg);    break;
        case 's': As = atof(optarg);        break;
        case 'f': fc = atof(optarg);        break;
        case 'c': f0 = atof(optarg);        break;
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
    } else if (ripple <= 0) {
        fprintf(stderr,"error: %s, pass-band ripple out of range\n", argv[0]);
        usage();
        exit(1);
    } else if (As <= 0) {
        fprintf(stderr,"error: %s, stop-band ripple out of range\n", argv[0]);
        usage();
        exit(1);
    }


    // number of analog poles/zeros
    unsigned int npa = n;
    unsigned int nza;

    // analog poles/zeros/gain
    float complex pa[n];
    float complex za[n];
    float complex ka;
    float complex k0;

    unsigned int r = n%2;
    unsigned int L = (n-r)/2;

    unsigned int i;
    // specific filter variables
    float epsilon, Gp, Gs, ep, es;

    switch (ftype) {
    case LIQUID_IIRDES_BUTTER:
        printf("Butterworth filter design:\n");
        nza = 0;
        k0 = 1.0f;
        butter_azpkf(n,za,pa,&ka);
        break;
    case LIQUID_IIRDES_CHEBY1:
        printf("Cheby-I filter design:\n");
        nza = 0;
        epsilon = sqrtf( powf(10.0f, ripple / 10.0f) - 1.0f );
        k0 = r ? 1.0f : 1.0f / sqrtf(1.0f + epsilon*epsilon);
        cheby1_azpkf(n,epsilon,za,pa,&ka);
        break;
    case LIQUID_IIRDES_CHEBY2:
        printf("Cheby-II filter design:\n");
        nza = 2*L;
        epsilon = powf(10.0f, -As/20.0f);
        k0 = 1.0f;
        cheby2_azpkf(n,epsilon,za,pa,&ka);
        break;
    case LIQUID_IIRDES_ELLIP:
        printf("elliptic filter design:\n");
        nza = 2*L;
        Gp = powf(10.0f, -ripple  / 20.0f);
        Gs = powf(10.0f, -As      / 20.0f);
        printf("  Gp = %12.8f\n", Gp);
        printf("  Gs = %12.8f\n", Gs);

        // epsilon values
        ep = sqrtf(1.0f/(Gp*Gp) - 1.0f);
        es = sqrtf(1.0f/(Gs*Gs) - 1.0f);
        k0 = r ? 1.0f : 1.0f / sqrtf(1.0f + ep*ep);

        ellip_azpkf(n,ep,es,za,pa,&ka);
        break;
    case LIQUID_IIRDES_BESSEL:
        printf("Bessel filter design:\n");
        bessel_azpkf(n,za,pa,&ka);
        k0 = 1.0f;
        nza = 0;
        break;
    default:
        fprintf(stderr,"error: iirdes_example: unknown filter type\n");
        exit(1);
    }

    printf("poles (analog):\n");
    for (i=0; i<npa; i++)
        printf("  pa[%3u] = %12.8f + j*%12.8f\n", i, crealf(pa[i]), cimagf(pa[i]));
    printf("zeros (analog):\n");
    for (i=0; i<nza; i++)
        printf("  za[%3u] = %12.8f + j*%12.8f\n", i, crealf(za[i]), cimagf(za[i]));
    printf("gain (analog):\n");
    printf("  ka : %12.8f + j*%12.8f\n", crealf(ka), cimagf(ka));

    // complex digital poles/zeros/gain
    // NOTE: allocated double the filter order to cover band-pass, band-stop cases
    float complex zd[2*n];
    float complex pd[2*n];
    float complex kd;
    float m = iirdes_freqprewarp(btype,fc,f0);
    printf("m : %12.8f\n", m);
    bilinear_zpkf(za,    nza,
                  pa,    npa,
                  k0,    m,
                  zd, pd, &kd);

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");

    // print analog z/p/k
    fprintf(fid,"nza = %u;\n", nza);
    fprintf(fid,"npa = %u;\n", npa);
    fprintf(fid,"za = zeros(1,nza);\n");
    for (i=0; i<nza; i++)
        fprintf(fid,"  za(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(za[i]), cimagf(za[i]));
    fprintf(fid,"pa = zeros(1,npa);\n");
    for (i=0; i<npa; i++)
        fprintf(fid,"  pa(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(pa[i]), cimagf(pa[i]));

    // print digital z/p/k
    printf("zeros (digital, low-pass prototype):\n");
    for (i=0; i<n; i++)
        printf("  zd[%3u] = %12.4e + j*%12.4e;\n", i, crealf(zd[i]), cimagf(zd[i]));
    printf("poles (digital, low-pass prototype):\n");
    for (i=0; i<n; i++)
        printf("  pd[%3u] = %12.4e + j*%12.4e;\n", i, crealf(pd[i]), cimagf(pd[i]));
    printf("gain (digital):\n");
    printf("  kd : %12.8f + j*%12.8f\n", crealf(kd), cimagf(kd));

    // negate zeros, poles for high-pass and band-stop cases
    if (btype == LIQUID_IIRDES_HIGHPASS ||
        btype == LIQUID_IIRDES_BANDSTOP)
    {
        for (i=0; i<n; i++) {
            zd[i] = -zd[i];
            pd[i] = -pd[i];
        }
    }

    // transform zeros, poles in band-pass, band-stop cases
    // NOTE: this also doubles the filter order
    if (btype == LIQUID_IIRDES_BANDPASS ||
        btype == LIQUID_IIRDES_BANDSTOP)
    {
        // allocate memory for transformed zeros, poles
        float complex zd1[2*n];
        float complex pd1[2*n];

        // run zeros, poles transform
        iirdes_dzpk_lp2bp(zd, pd,   // low-pass prototype zeros, poles
                          n,        // filter order
                          f0,       // center frequency
                          zd1,pd1); // transformed zeros, poles (length: 2*n)

        // copy transformed zeros, poles
        memmove(zd, zd1, 2*n*sizeof(float complex));
        memmove(pd, pd1, 2*n*sizeof(float complex));

        // update parameters : n -> 2*n
        r = 0;
        L = n;
        n = 2*n;
    }

    fprintf(fid,"n=%u;\n", n);
    fprintf(fid,"r=%u;\n", r);
    fprintf(fid,"L=%u;\n", L);
    fprintf(fid,"nfft=1024;\n");

    // print digital z/p/k
    fprintf(fid,"zd = zeros(1,n);\n");
    fprintf(fid,"pd = zeros(1,n);\n");
    for (i=0; i<n; i++) {
        fprintf(fid,"  zd(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(zd[i]), cimagf(zd[i]));
        fprintf(fid,"  pd(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(pd[i]), cimagf(pd[i]));
    }


    if (format == LIQUID_IIRDES_TF) {
        float b[n+1];       // numerator
        float a[n+1];       // denominator

        // convert complex digital poles/zeros/gain into transfer function
        iirdes_dzpk2tff(zd,pd,n,kd,b,a);

        // print coefficients
        for (i=0; i<=n; i++) printf("a[%3u] = %12.8f;\n", i, a[i]);
        for (i=0; i<=n; i++) printf("b[%3u] = %12.8f;\n", i, b[i]);

        fprintf(fid,"a = zeros(1,n+1);\n");
        fprintf(fid,"b = zeros(1,n+1);\n");
        for (i=0; i<=n; i++) {
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
        // second-order sections
        float A[3*(L+r)];
        float B[3*(L+r)];

        // convert complex digital poles/zeros/gain into second-
        // order sections form
        iirdes_dzpk2sosf(zd,pd,n,kd,B,A);

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

    // plot zeros, poles
    fprintf(fid,"\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"k=0:0.01:1;\n");
    fprintf(fid,"ti = cos(2*pi*k);\n");
    fprintf(fid,"tq = sin(2*pi*k);\n");
    fprintf(fid,"plot(ti,tq,'-','LineWidth',1,'Color',[1 1 1]*0.7,...\n");
    fprintf(fid,"     real(zd),imag(zd),'o','LineWidth',2,'Color',[0.5 0   0],'MarkerSize',2,...\n");
    fprintf(fid,"     real(pd),imag(pd),'x','LineWidth',2,'Color',[0   0.5 0],'MarkerSize',2);\n");
    fprintf(fid,"xlabel('real');\n");
    fprintf(fid,"ylabel('imag');\n");
    fprintf(fid,"title('z-plane');\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.2);\n");
    fprintf(fid,"axis square;\n");

    // plot group delay
    fprintf(fid,"f = [0:(nfft-1)]/nfft - 0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"  plot(f,gd,'-','Color',[0 0.5 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 0 ceil(1.1*max(gd))]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Group delay [samples]');\n");

    // plot magnitude response
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1),\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'-','Color',[0.5 0 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 -4 1]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Filter PSD [dB]');\n");
    fprintf(fid,"subplot(2,1,2),\n");
    fprintf(fid,"  plot(f,20*log10(abs(H)),'-','Color',[0.5 0 0],'LineWidth',2);\n");
    fprintf(fid,"  axis([0.0 0.5 -100 10]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Filter PSD [dB]');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    printf("done.\n");
    return 0;
}

