// 
// random_histogram_example.c
//
// This example tests the random number generators for different
// distributions.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "random_histogram_example.m"


// print usage/help message
void usage()
{
    printf("random_histogram_example [options]\n");
    printf("  -h    : print usage\n");
    printf("  -N     : number of trials\n");
    printf("  -n     : number of histogram bins\n");
    printf("  -d     : distribution: {uniform, normal, exp, weib, gamma, nak, rice}\n");
    printf("  -u     : u      UNIFORM: lower edge\n");
    printf("  -v     : v      UNIFORM: upper edge\n");
    printf("  -e     : eta    NORMAL: mean\n");
    printf("  -s     : sigma  NORMAL: standard deviation\n");
    printf("  -l     : lambda EXPONENTIAL: decay factor\n");
    printf("  -a     : alpha  WEIBULL: shape\n");
    printf("  -b     : beta   WEIBULL: spread\n");
    printf("  -g     : gamma  WEIBULL: threshold\n");
    printf("  -A     : alpha  GAMMA: shape\n");
    printf("  -B     : beta   GAMMA: spread\n");
    printf("  -m     : m      NAKAGAMI: shape\n");
    printf("  -o     : omega  NAKAGAMI: spread\n");
    printf("  -K     : K      RICE-K: spread\n");
    printf("  -O     : omega  RICE-K: spread\n");
}

int main(int argc, char*argv[])
{
    srand(time(NULL));
    unsigned long int num_trials = 100000; // number of trials
    unsigned int num_bins = 30;
    enum {
        UNIFORM=0,
        NORMAL,
        EXPONENTIAL,
        WEIBULL,
        GAMMA,
        NAKAGAMIM,
        RICEK
    } distribution=NORMAL;

    // distribution parameters
    float u         = 0.0f; // UNIFORM: lower edge
    float v         = 1.0f; // UNIFORM: upper edge
    float eta       = 0.0f; // NORMAL: mean
    float sigma     = 1.0f; // NORMAL: standard deviation
    float lambda    = 3.0f; // EXPONENTIAL: decay factor
    float alphaw    = 1.0f; // WEIBULL: shape
    float betaw     = 1.0f; // WEIBULL: spread
    float gammaw    = 1.0f; // WEIBULL: threshold
    float alphag    = 4.5f; // GAMMA: shape
    float betag     = 1.0f; // GAMMA: spread
    float m         = 4.5f; // NAKAGAMI: shape factor
    float omeganak  = 1.0f; // NAKAGMAI: spread factor
    float K         = 4.0f; // RICE-K: K-factor (shape)
    float omegarice = 1.0f; // RICE-K: spread factor

    int dopt;
    while ((dopt = getopt(argc,argv,"hN:n:d:u:v:e:s:l:a:b:g:A:B:m:o:K:O:")) != EOF) {
        switch (dopt) {
        case 'h':
            usage();
            return 0;
        case 'N': num_trials = atoi(optarg); break;
        case 'n': num_bins = atoi(optarg); break;
        case 'd':
            if      (strcmp(optarg,"uniform")==0)   distribution = UNIFORM;
            else if (strcmp(optarg,"normal")==0)    distribution = NORMAL;
            else if (strcmp(optarg,"exp")==0)       distribution = EXPONENTIAL;
            else if (strcmp(optarg,"weib")==0)      distribution = WEIBULL;
            else if (strcmp(optarg,"gamma")==0)     distribution = GAMMA;
            else if (strcmp(optarg,"nak")==0)       distribution = NAKAGAMIM;
            else if (strcmp(optarg,"rice")==0)      distribution = RICEK;
            else {
                fprintf(stderr,"error: %s, unknown/unsupported distribution '%s'\n", argv[0], optarg);
                exit(1);
            }
        case 'u': u         = atof(optarg); break;
        case 'v': v         = atof(optarg); break;
        case 'e': eta       = atof(optarg); break;
        case 's': sigma     = atof(optarg); break;
        case 'l': lambda    = atof(optarg); break;
        case 'a': alphaw    = atof(optarg); break;
        case 'b': betaw     = atof(optarg); break;
        case 'g': gammaw    = atof(optarg); break;
        case 'A': alphag    = atof(optarg); break;
        case 'B': betag     = atof(optarg); break;
        case 'm': m         = atof(optarg); break;
        case 'o': omeganak  = atof(optarg); break;
        case 'K': K         = atof(optarg); break;
        case 'O': omegarice = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input
    if (num_bins == 0) {
        fprintf(stderr,"error: %s, number of bins must be greater than zero\n", argv[0]);
        exit(1);
    } else if (num_trials == 0) {
        fprintf(stderr,"error: %s, number of trials must be greater than zero\n", argv[0]);
        exit(1);
    }

    float xmin = 0.0f;
    float xmax = 1.0f;

    unsigned long int i;

    // make a guess at the histogram range so we don't need to
    // store all the generated random variables in a giant array.
    if (distribution == UNIFORM) {
        xmin = u - 0.08*(v-u); // lower edge less 8% range
        xmax = v + 0.08*(v-u); // upper edge plus 8% range
    } else if (distribution == NORMAL) {
        xmin = eta - 4.0f*sigma;
        xmax = eta + 4.0f*sigma;
    } else if (distribution == EXPONENTIAL) {
        xmin = 0.0f;
        xmax = 7.0f / lambda;
    } else if (distribution == WEIBULL) {
        xmin = gammaw;
        xmax = gammaw + betaw*powf( -logf(1e-3f), 1.0f/alphaw );
    } else if (distribution == GAMMA) {
        xmin = 0.0f;
        xmax = 6.5 * betag + 2.0*alphag;
    } else if (distribution == NAKAGAMIM) {
        xmin = 0.0f;
        xmax = 1.5f*( powf(omeganak, 0.8f) + 1.0f/m );
    } else if (distribution == RICEK) {
        xmin = 0.0f;
        xmax = 3.0f*logf(omegarice+1.0f) + 1.5f/(K+1.0f);
    } else {
        fprintf(stderr, "error: %s, unknown/unsupported distribution\n", argv[0]);
        exit(1);
    }

    //
    //float xspan = xmax - xmin;
    float bin_width = (xmax - xmin) / (num_bins);

    // initialize histogram
    unsigned int hist[num_bins];
    for (i=0; i<num_bins; i++)
        hist[i] = 0;

    // generate random variables
    float x = 0.0f;
    float m1 = 0.0f;    // first moment
    float m2 = 0.0f;    // second moment
    for (i=0; i<num_trials; i++) {
        switch (distribution) {
        case UNIFORM:     x = randuf(u,v);                      break;
        case NORMAL:      x = sigma*randnf() + eta;             break;
        case EXPONENTIAL: x = randexpf(lambda);                 break;
        case WEIBULL:     x = randweibf(alphaw,betaw,gammaw);   break;
        case GAMMA:       x = randgammaf(alphag,betag);         break;
        case NAKAGAMIM:   x = randnakmf(m,omeganak);            break;
        case RICEK:       x = randricekf(K,omegarice);          break;
        default:
            fprintf(stderr,"error: %s, unknown/unsupported distribution\n", argv[0]);
            exit(1);
        }

        // compute bin index
        unsigned int index;
        float ihat = num_bins * (x - xmin) / (xmax - xmin);
        if (ihat < 0.0f)
            index = 0;
        else
            index = (unsigned int)ihat;
        
        if (index >= num_bins)
            index = num_bins-1;

        hist[index]++;

        // update statistics
        m1 += x;    // first moment
        m2 += x*x;  // second moment
    }

    //
    m1 /= (float)num_trials;
    m2 /= (float)num_trials;

    // compute expected distribution
    unsigned int num_steps = 100;
    float xstep = (xmax - xmin) / (num_steps - 1);
    float f[num_steps];
    float F[num_steps];
    for (i=0; i<num_steps; i++) {
        x = xmin + i*xstep;
        switch (distribution) {
        case UNIFORM:
            f[i] = randuf_pdf(x,u,v);
            F[i] = randuf_cdf(x,u,v);
            break;
        case NORMAL:
            f[i] = randnf_pdf(x,eta,sigma);
            F[i] = randnf_cdf(x,eta,sigma);
            break;
        case EXPONENTIAL:
            f[i] = randexpf_pdf(x,lambda);
            F[i] = randexpf_cdf(x,lambda);
            break;
        case WEIBULL:
            f[i] = randweibf_pdf(x,alphaw,betaw,gammaw);
            F[i] = randweibf_cdf(x,alphaw,betaw,gammaw);
            break;
        case GAMMA:
            f[i] = randgammaf_pdf(x,alphag,betag);
            F[i] = randgammaf_cdf(x,alphag,betag);
            break;
        case NAKAGAMIM:
            f[i] = randnakmf_pdf(x,m,omeganak);
            F[i] = randnakmf_cdf(x,m,omeganak);
            break;
        case RICEK:
            f[i] = randricekf_pdf(x,K,omegarice);
            F[i] = randricekf_cdf(x,K,omegarice);
            break;
        default:
            fprintf(stderr,"error: %s, unknown/unsupported distribution\n", argv[0]);
            exit(1);
        }
    }

    // print results to screen
    // find max(hist)
    unsigned int hist_max = 0;
    for (i=0; i<num_bins; i++)
        hist_max = hist[i] > hist_max ? hist[i] : hist_max;

    printf("%8s : %6s [%6s]\n", "x", "count", "prob.");
    for (i=0; i<num_bins; i++) {
        printf("%8.2f : %6u [%6.4f]", xmin + i*bin_width, hist[i], (float)hist[i] / (float)num_trials);

        unsigned int k;
        unsigned int n = round(60 * (float)hist[i] / (float)hist_max);
        for (k=0; k<n; k++)
            printf("#");
        printf("\n");
    }

    // print distribution info, statistics
    printf("statistics:\n");
    switch (distribution) {
    case UNIFORM:
        printf("    distribution            :   %s\n", "uniform");
        printf("    u                       :   %f\n", u);
        printf("    v                       :   %f\n", v);
        break;
    case NORMAL:
        printf("    distribution            :   %s\n", "normal (Gauss)");
        printf("    eta                     :   %f\n", eta);
        printf("    sigma                   :   %f\n", sigma);
        break;
    case EXPONENTIAL:
        printf("    distribution            :   %s\n", "exponential");
        printf("    lambda                  :   %f\n", lambda);
        break;
    case WEIBULL:
        printf("    distribution            :   %s\n", "Weibull");
        printf("    alpha                   :   %f\n", alphaw);
        printf("    beta                    :   %f\n", betaw);
        printf("    gamma                   :   %f\n", gammaw);
        break;
    case GAMMA:
        printf("    distribution            :   %s\n", "gamma");
        printf("    alpha                   :   %f\n", alphag);
        printf("    beta                    :   %f\n", betag);
        break;
    case NAKAGAMIM:
        printf("    distribution            :   %s\n", "Nakagami-m");
        printf("    m                       :   %f\n", m);
        printf("    omega                   :   %f\n", omeganak);
        break;
    case RICEK:
        printf("    distribution            :   %s\n", "Rice-K");
        printf("    K                       :   %f\n", K);
        printf("    omega                   :   %f\n", omegarice);
        break;
    default:
        fprintf(stderr,"error: %s, unknown/unsupported distribution\n", argv[0]);
        exit(1);
    }
    printf("\n");
    printf("    samples                 :   %8lu\n", num_trials);
    printf("    first moment,  E( x }   :   %8.3f\n", m1);
    printf("    second moment, E{x^2}   :   %8.3f\n", m2);
    printf("    variance                :   %8.3f\n", m2 - m1*m1);
    printf("    standard deviation      :   %8.3f\n", sqrtf(m2 - m1*m1));

    // 
    // export results
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"xmin = %12.4e;\n", xmin);
    fprintf(fid,"xmax = %12.4e;\n", xmax);
    fprintf(fid,"num_bins = %u;\n", num_bins);
    fprintf(fid,"xspan = xmax - xmin;\n");

    float F_hat = 0.0f;
    for (i=0; i<num_bins; i++) {
        x = xmin + ((float)i + 0.5f)*bin_width;
        float h = (float)(hist[i]) / (num_trials * bin_width);
        fprintf(fid,"xh(%3lu) = %12.4e; h(%3lu) = %12.4e;\n", i+1, x, i+1, h);

        x = xmin + ((float)i + 1.0f)*bin_width;
        F_hat += h;
        fprintf(fid,"xH(%3lu) = %12.4e; H(%3lu) = %12.4e;\n", i+1, x, i+1, F_hat);
    }
    fprintf(fid,"H = H/H(end);\n");

    for (i=0; i<num_steps; i++) {
        x = xmin + i*xstep;
        fprintf(fid,"xf(%3lu) = %12.4e; f(%3lu) = %12.4e; F(%3lu) = %12.4e;\n", i+1, x, i+1, f[i], i+1, F[i]);
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(xh,h,'x', xf,f,'-');\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('f_x(x)');\n");
    fprintf(fid,"axis([(xmin-0.1*xspan) (xmax+0.1*xspan) 0 1.1*max([h f])]);\n");
    fprintf(fid,"legend('histogram','true PDF',1);\n");

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(xH,H,'x', xf,F,'-');\n");
    fprintf(fid,"xlabel('x');\n");
    fprintf(fid,"ylabel('f_x(x)');\n");
    //fprintf(fid,"axis([(xmin-0.1*xspan) (xmax+0.1*xspan) 0 1]);\n");
    fprintf(fid,"legend('histogram','true CDF',0);\n");

    fclose(fid);
    printf("results written to %s.\n",OUTPUT_FILENAME);


    printf("done.\n");
    return 0;
}

