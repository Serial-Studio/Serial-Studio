// Test soft demoulation using log-likelihood ratio
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "liquid.h"

#define OUTPUT_FILENAME "modem_demodulate_soft_test.m"

int main() {
    srand(time(NULL));

    // options
    modulation_scheme ms = LIQUID_MODEM_QAM16;  // modulation scheme
    float complex e = 0.1f + _Complex_I*0.2f;   // error

    unsigned int i;

    // derived values
    float sig = 0.2f;           // noise standard deviation

    // generate constellation
    modemcf q = modemcf_create(ms);
    unsigned int bps = modemcf_get_bps(q);
    unsigned int M = 1 << bps;  // constellation size
    float complex c[M];         // constellation
    for (i=0; i<M; i++)
        modemcf_modulate(q, i, &c[i]);
    modemcf_destroy(q);

    // select input symbol and compute received symbol
    unsigned int sym_in = rand() % M;
    float complex r = c[sym_in] + e;

    // run soft demodulation for each bit
    float soft_bits[bps];
    unsigned int k;
    for (k=0; k<bps; k++) {
        printf("\n");
        printf("********* bit index %u ************\n", k);
        // reset soft bit value
        soft_bits[k] = 0.0f;
        float bit_0 = 0.0f;
        float bit_1 = 0.0f;

        // compute LLR for this bit
        for (i=0; i<M; i++) {
            // compute distance between received point and symbol
            float d = crealf( (r-c[i])*conjf(r-c[i]) );
            float t = expf( -d / (2.0f*sig*sig) );

            // check if this symbol has a '0' or '1' at this bit index
            unsigned int bit = (i >> (bps-k-1)) & 0x01;
            //printf("%c", bit ? '1' : '0');

            if (bit) bit_1 += t;
            else     bit_0 += t;

            printf("  symbol : ");
            unsigned int j;
            for (j=0; j<bps; j++)
                printf("%c", (i >> (bps-j-1)) & 0x01 ? '1' : '0');
            printf(" [%c]", bit ? '1' : '0');
            printf(" { %7.4f %7.4f}, d=%7.4f, t=%12.8f\n", crealf(c[i]), cimagf(c[i]), d, t);

        }

        soft_bits[k] = logf(bit_1) - logf(bit_0);
        printf(" {0 : %12.8f, 1 : %12.8f}\n", bit_0, bit_1);
    }

    // print results
    printf("\n");
    printf("  input symbol : ");
    for (k=0; k<bps; k++)
        printf("%c", (sym_in >> (bps-k-1)) & 0x01 ? '1' : '0');
    printf(" {%12.8f, %12.8f}\n", crealf(c[sym_in]), cimagf(c[sym_in]));

    printf("  soft bits :\n");
    for (k=0; k<bps; k++) {
        printf("    %1u : ", (sym_in >> (bps-k-1)) & 0x01);
        printf("%12.8f > ", soft_bits[k]);
        int soft_bit = (soft_bits[k]*16 + 127);
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit =   0;
        printf("%5d > %1u\n", soft_bit, soft_bit & 0x80 ? 1 : 0);
    }

    // 
    // export results to file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n\n");
    fprintf(fid,"m = %u;\n", bps);
    fprintf(fid,"M = %u;\n", 1<<bps);
    fprintf(fid,"c = zeros(1,M);\n");
    fprintf(fid,"i_str = cell(1,M);\n");

    for (i=0; i<M; i++) {
        // write symbol to output file
        fprintf(fid,"c(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(c[i]), cimagf(c[i]));
        fprintf(fid,"i_str{%3u} = '", i+1);
        unsigned int j;
        for (j=0; j<bps; j++)
            fprintf(fid,"%c", (i >> (bps-j-1)) & 0x01 ? '1' : '0');
        fprintf(fid,"';\n");
    }
    fprintf(fid,"x = %12.8f + j*%12.8f;\n", crealf(c[sym_in]), cimagf(c[sym_in]));
    fprintf(fid,"r = %12.8f + j*%12.8f;\n", crealf(r), cimagf(r));

    // plot results
    fprintf(fid,"\n\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(c,'o','MarkerSize',4,r,'rx',[x r]);\n");
    fprintf(fid,"hold on;\n");
    fprintf(fid,"text(real(c)+0.02, imag(c)+0.02, i_str);\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.6);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('in phase');\n");
    fprintf(fid,"ylabel('quadrature phase');\n");

    fclose(fid);
    printf("results written to %s.\n", OUTPUT_FILENAME);


    printf("done.\n");
    return 0;
}
