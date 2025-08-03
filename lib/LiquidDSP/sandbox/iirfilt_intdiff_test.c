// iirfilt_intdiff_test
//
// Tests infinite impulse response (IIR) integrator and differentiator
// filters. The filters are each order 8 and have a near-integer group
// delay of 7 samples. The magnitude response of the ideal is on the
// order of -140 dB error.
//
// References:
//  [Pintelon:1990] Rik Pintelon and Johan Schoukens, "Real-Time
//      Integration and Differentiation of Analog Signals by Means of
//      Digital Filtering," IEEE Transactions on Instrumentation and
//      Measurement, vol 39 no. 6, December 1990.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <math.h>
#include "liquid.h"

#define OUTPUT_FILENAME "iirfilt_intdiff_test.m"

int main(int argc, char*argv[]) {
    // options
    unsigned int num_samples = 801;

    // 
    // integrator digital zeros/poles/gain, [Pintelon:1990] Table II
    //
    // zeros, digital, integrator
    float complex zdi[8] = {
        1.175839 * -1.0f,
        3.371020 * cexpf(_Complex_I * M_PI / 180.0f * -125.1125f),
        3.371020 * cexpf(_Complex_I * M_PI / 180.0f *  125.1125f),
        4.549710 * cexpf(_Complex_I * M_PI / 180.0f *  -80.96404f),
        4.549710 * cexpf(_Complex_I * M_PI / 180.0f *   80.96404f),
        5.223966 * cexpf(_Complex_I * M_PI / 180.0f *  -40.09347f),
        5.223966 * cexpf(_Complex_I * M_PI / 180.0f *   40.09347f),
        5.443743,};
    // poles, digital, integrator
    float complex pdi[8] = {
        0.5805235f * -1.0f,
        0.2332021f * cexpf(_Complex_I * M_PI / 180.0f * -114.0968f),
        0.2332021f * cexpf(_Complex_I * M_PI / 180.0f *  114.0968f),
        0.1814755f * cexpf(_Complex_I * M_PI / 180.0f *  -66.33969f),
        0.1814755f * cexpf(_Complex_I * M_PI / 180.0f *   66.33969f),
        0.1641457f * cexpf(_Complex_I * M_PI / 180.0f *  -21.89539f),
        0.1641457f * cexpf(_Complex_I * M_PI / 180.0f *   21.89539f),
        1.0f,};
    // gain, digital, integrator
    float complex kdi = -1.89213380759321e-05f;

#if 1
    // second-order sections
    // allocate 12 values for 4 second-order sections each with
    // 2 roots (order 8), e.g. (1 + r0 z^-1)(1 + r1 z^-1)
    float Bi[12];
    float Ai[12];
    iirdes_dzpk2sosf(zdi, pdi, 8, kdi, Bi, Ai);
    // create integrator
    iirfilt_crcf integrator = iirfilt_crcf_create_sos(Bi,Ai,4);
#else
    // regular transfer function form
    // 9 = order + 1
    float bi[9];
    float ai[9];
    iirdes_dzpk2tff(zdi, pdi, 8, kdi, bi, ai);
    iirfilt_crcf integrator = iirfilt_crcf_create(bi,9,ai,9);
#endif


    // 
    // differentiator digital zeros/poles/gain, [Pintelon:1990] Table IV
    //
    // zeros, digital, differentiator
    float complex zdd[8] = {
        1.702575f * -1.0f,
        5.877385f * cexpf(_Complex_I * M_PI / 180.0f * -221.4063f),
        5.877385f * cexpf(_Complex_I * M_PI / 180.0f *  221.4063f),
        4.197421f * cexpf(_Complex_I * M_PI / 180.0f * -144.5972f),
        4.197421f * cexpf(_Complex_I * M_PI / 180.0f *  144.5972f),
        5.350284f * cexpf(_Complex_I * M_PI / 180.0f *  -66.88802f),
        5.350284f * cexpf(_Complex_I * M_PI / 180.0f *   66.88802f),
        1.0f,};
    // poles, digital, differentiator
    float complex pdd[8] = {
        0.8476936f * -1.0f,
        0.2990781f * cexpf(_Complex_I * M_PI / 180.0f * -125.5188f),
        0.2990781f * cexpf(_Complex_I * M_PI / 180.0f *  125.5188f),
        0.2232427f * cexpf(_Complex_I * M_PI / 180.0f *  -81.52326f),
        0.2232427f * cexpf(_Complex_I * M_PI / 180.0f *   81.52326f),
        0.1958670f * cexpf(_Complex_I * M_PI / 180.0f *  -40.51510f),
        0.1958670f * cexpf(_Complex_I * M_PI / 180.0f *   40.51510f),
        0.1886088f,};
    // gain, digital, differentiator
    float complex kdd = 2.09049284907492e-05f;

#if 1
    // second-order sections
    // allocate 12 values for 4 second-order sections each with
    // 2 roots (order 8), e.g. (1 + r0 z^-1)(1 + r1 z^-1)
    float Bd[12];
    float Ad[12];
    iirdes_dzpk2sosf(zdd, pdd, 8, kdd, Bd, Ad);
    // create differentiator
    iirfilt_crcf differentiator = iirfilt_crcf_create_sos(Bd,Ad,4);
#else
    // regular transfer function form
    // 9 = order + 1
    float bd[9];
    float ad[9];
    iirdes_dzpk2tff(zdd, pdd, 8, kdd, bd, ad);
    iirfilt_crcf differentiator = iirfilt_crcf_create(bd,9,ad,9);
#endif

    // allocate arrays
    float complex x[num_samples];
    float complex y[num_samples];
    float complex z[num_samples];

    float tmin = 0.0f;
    float tmax = 1.0f;
    float dt   = (tmax-tmin)/(float)(num_samples-1);
    unsigned int i;
    for (i=0; i<num_samples; i++) {
        // time vector
        float t = tmin + dt*i;

        // generate input signal
        x[i] = 0.2*cosf(2*M_PI*4.1*t + 0.0f) +
               0.4*cosf(2*M_PI*6.9*t + 0.9f) +
               0.7*cosf(2*M_PI*9.7*t + 0.2f) +
               _Complex_I*cosf(2*M_PI*4*t);

        // run integrator
        iirfilt_crcf_execute(integrator, x[i]*dt, &y[i]);

        // run differentitator
        iirfilt_crcf_execute(differentiator, y[i]/dt, &z[i]);
    }

    // open output file
    FILE*fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"delay = 7; %% fixed group delay\n");

    // save
    fprintf(fid,"n = %u;\n", num_samples);
    fprintf(fid,"x = zeros(1,n);\n");
    fprintf(fid,"y = zeros(1,n);\n");
    fprintf(fid,"z = zeros(1,n);\n");
    for (i=0; i<num_samples; i++) {
        fprintf(fid,"x(%5u)=%12.4e+%12.4ej; ", i+1, crealf(x[i]), cimagf(x[i]));
        fprintf(fid,"y(%5u)=%12.4e+%12.4ej; ", i+1, crealf(y[i]), cimagf(y[i]));
        fprintf(fid,"z(%5u)=%12.4e+%12.4ej; ", i+1, crealf(z[i]), cimagf(z[i]));
        fprintf(fid,"\n");
    }

    // plot results
    fprintf(fid,"tmin = %f;\n", tmin);
    fprintf(fid,"tmax = %f;\n", tmax);
    fprintf(fid,"dt = (tmax-tmin)/(n-1);\n");
    fprintf(fid,"tx = tmin + [[0:(n-1)]        ]*dt;\n");
    fprintf(fid,"ty = tmin + [[0:(n-1)]-delay  ]*dt;\n");
    fprintf(fid,"tz = tmin + [[0:(n-1)]-2*delay]*dt;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(3,1,1);\n");
    fprintf(fid,"  plot(tx, real(x), 'LineWidth', 1.2, tx, imag(x),'LineWidth',1.2);\n");
    fprintf(fid,"  axis([tmin tmax -2 2]);\n");
    fprintf(fid,"  ylabel('input');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,2);\n");
    fprintf(fid,"  plot(ty, real(y),'LineWidth',1.2, ty, imag(y),'LineWidth',1.2);\n");
    fprintf(fid,"  ymax = round(10000*max(abs(y)))/10000;\n");
    fprintf(fid,"  axis([tmin tmax -ymax ymax]);\n");
    fprintf(fid,"  ylabel('integrated');\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"subplot(3,1,3);\n");
    fprintf(fid,"  plot(tz, real(z),'LineWidth',1.2, tz, imag(z),'LineWidth',1.2);\n");
    fprintf(fid,"  axis([tmin tmax -2 2]);\n");
    fprintf(fid,"  ylabel('int/diff');\n");
    fprintf(fid,"  grid on;\n");

    // compute frequency response
    unsigned int nfft = 256;
    float complex Hi[nfft], Hd[nfft];
    float gdi[nfft], gdd[nfft];
    for (i=0; i<nfft; i++) {
        float f = 1e-4f + 0.5f * (float)i / (float)nfft;

        // compute magnitude/phase response
        iirfilt_crcf_freqresponse(integrator,     f, &Hi[i]);
        iirfilt_crcf_freqresponse(differentiator, f, &Hd[i]);

        // compute group delay
        gdi[i] = iirfilt_crcf_groupdelay(integrator, f);
        gdd[i] = iirfilt_crcf_groupdelay(differentiator, f);
    }

    // save frequency response
    fprintf(fid,"nfft = %u;\n", nfft);
    fprintf(fid,"Hi  = zeros(1,nfft);\n");
    fprintf(fid,"Hd  = zeros(1,nfft);\n");
    fprintf(fid,"gdi = zeros(1,nfft);\n");
    fprintf(fid,"gdd = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++) {
        fprintf(fid,"Hi(%5u)=%12.4e+%12.4ej; ", i+1, crealf(Hi[i]), cimagf(Hi[i]));
        fprintf(fid,"Hd(%5u)=%12.4e+%12.4ej; ", i+1, crealf(Hd[i]), cimagf(Hd[i]));
        fprintf(fid,"gdi(%5u)=%12.4e; ",        i+1, gdi[i]);
        fprintf(fid,"gdd(%5u)=%12.4e; ",        i+1, gdd[i]);
        fprintf(fid,"\n");
    }

    // plot magnitude response
    fprintf(fid,"f = 0.5*[0:(nfft-1)]/nfft;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"subplot(2,1,1),\n");
    fprintf(fid,"  plot(f,20*log10(Hi),    '-','Color',[0.0 0.5 0.0],'LineWidth',1.2,...\n");
    fprintf(fid,"       f,20*log10(Hd),    '-','Color',[0.0 0.0 0.5],'LineWidth',1.2,...\n");
    fprintf(fid,"       f,20*log10(Hi.*Hd),'-','Color',[0.5 0.3 0.0],'LineWidth',1.8);\n");
    fprintf(fid,"  axis([0.0 0.5 -20 20]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Integrator','Differentiator','Composite','location','northeast');\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Filter PSD [dB]');\n");

    // plot group delay
    fprintf(fid,"subplot(2,1,2),\n");
    fprintf(fid,"  plot(f,gdi,'-','Color',[0 0.5 0.0],'LineWidth',1.2,...\n");
    fprintf(fid,"       f,gdd,'-','Color',[0 0.0 0.5],'LineWidth',1.2);\n");
    fprintf(fid,"  axis([0.0 0.5 0 ceil(1.1*max([gdi gdd]))]);\n");
    fprintf(fid,"  grid on;\n");
    fprintf(fid,"  legend('Integrator','Differentiator','location','southeast');\n");
    fprintf(fid,"  xlabel('Normalized Frequency');\n");
    fprintf(fid,"  ylabel('Group delay [samples]');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);

    // destroy filter objects
    iirfilt_crcf_destroy(integrator);
    iirfilt_crcf_destroy(differentiator);

    printf("done.\n");
    return 0;
}

