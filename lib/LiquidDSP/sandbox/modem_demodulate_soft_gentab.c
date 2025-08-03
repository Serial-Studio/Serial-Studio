// Generate table of nearest symbols and tests soft demodulation
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "liquid.h"

#define OUTPUT_FILENAME "modem_demodulate_soft_gentab.m"
#define DEBUG           0

// print a string of bits to the standard output
void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%1u", (_x >> (_n-i-1)) & 1);
}

// soft demodulation
//  _q          :   demodulator object
//  _c          :   constellation table
//  _cp         :   nearest neighbor table [size: _p x 1]
//  _p          :   size of table
//  _r          :   received sample
//  _s          :   hard demodulator output
//  _soft_bits  :   soft bit output (approximate log-likelihood ratio)
void modemcf_demodulate_soft_tab(modemcf _q,
                               float complex * _c,
                               unsigned int * _cp,
                               unsigned int _p,
                               float complex _r,
                               unsigned int * _s,
                               float * _soft_bits);

int main(int argc, char*argv[])
{
    srand(time(NULL));

    // options
    modulation_scheme ms = LIQUID_MODEM_QAM16;  // modulation scheme
    unsigned int p = 4;                         // number of 'nearest symbols'
    float complex e = 0.1f + _Complex_I*0.15f;  // error

    // validate input
    if (ms == LIQUID_MODEM_UNKNOWN || ms >= LIQUID_MODEM_NUM_SCHEMES) {
        fprintf(stderr,"error: %s, invalid modulation scheme\n", argv[0]);
        exit(1);
    }

    unsigned int i;
    unsigned int j;
    unsigned int k;

    // derived values
    unsigned int bps = modulation_types[ms].bps;
    unsigned int M = 1 << bps;  // constellation size
    // ensure number of nearest symbols is not too large
    if (p > (M-1)) {
        fprintf(stderr,"error: requesting too many neighbors\n");
        exit(1);
    }
    float sig = 0.2f;           // noise standard deviation

    // generate constellation
    modemcf q = modemcf_create(ms);
    float complex c[M];         // constellation
    for (i=0; i<M; i++)
        modemcf_modulate(q, i, &c[i]);
    modemcf_destroy(q);

    // 
    // find nearest symbols
    //
    unsigned int cp[M*p];

    // initialize table (with 'M' as invalid symbol)
    for (i=0; i<M; i++) {
        for (k=0; k<p; k++)
            cp[i*p + k] = M;
    }

    int symbol_valid;
    for (i=0; i<M; i++) {
#if DEBUG
        printf("\n******** %3u ********\n", i);
#endif
        for (k=0; k<p; k++) {
#if DEBUG
            printf("iteration %u\n", k);
#endif
            float dmin=1e9f;
            for (j=0; j<M; j++) {
                symbol_valid = 1;
                // ignore this symbol
                if (i==j) symbol_valid = 0;

                // also ignore symbol if it is already in the index
                unsigned int l;
                for (l=0; l<p; l++) {
                    if ( cp[i*p + l] == j )
                        symbol_valid = 0;
                }

                // compute distance
                float d = cabsf( c[i] - c[j] );
#if DEBUG
                printf("  testing %3u... ", j);
                if (!symbol_valid)
                    printf("invalid\n");
                else
                    printf(" d = %8.6f ", d);
#endif

                if ( d < dmin && symbol_valid ) {
                    dmin = d;
                    cp[i*p + k] = j;
#if DEBUG
                    printf("*\n");
                } else {
                    if (symbol_valid)
                        printf("\n");
#endif
                }

                // ...
                //cp[i*p + k] = rand() % (M+4);
            }
#if DEBUG
            printf("new symbol : %3u, dmin=%8.6f\n", cp[i*p + k], dmin);
#endif
        }
    }

    // 
    // print results
    //
    for (i=0; i<M; i++) {
        printf(" %4u [", i);
        print_bitstring(i,bps);
        printf("] : ");
        for (k=0; k<p; k++) {
            printf(" ");
            print_bitstring(cp[i*p + k],bps);
            if (cp[i*p + k] < M)
                printf("(%6.4f)", cabsf( c[i]-c[cp[i*p+k]] ));
            else
                printf("        ");
        }
        printf("\n");
    }

    // print c-type array
    printf("\n");
    printf("// %s soft demodulation nearest neighbors (p=%u)\n", modulation_types[ms].name, p);
    printf("const unsigned char %s_demod_soft_neighbors[%u] = {\n", modulation_types[ms].name, p*M);
    printf("    ");
    for (i=0; i<M; i++) {
        for (k=0; k<p; k++) {
            printf("%4u", cp[i*p+k]);
            if (k != p-1) printf(",");
        }
        if (i != M-1) {
            printf(",   // ");
            print_bitstring(i,bps);
            printf("\n    ");
        } else {
            printf("};  // ");
            print_bitstring(i,bps);
            printf("\n\n");
        }
    }

    // select input symbol and compute received symbol
    unsigned int sym_in = rand() % M;
    if (ms == LIQUID_MODEM_QAM16)
        sym_in = 13;
    float complex r = c[sym_in] + e;

    // run soft demodulation for each bit
    float soft_bits[bps];
    for (k=0; k<bps; k++) {
#if DEBUG
        printf("\n");
        printf("********* bit index %u ************\n", k);
#endif
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

#if DEBUG
            printf("  symbol : ");
            print_bitstring(i,bps);
            printf(" [%c]", bit ? '1' : '0');
            printf(" { %7.4f %7.4f}, d=%7.4f, t=%12.8f\n", crealf(c[i]), cimagf(c[i]), d, t);
#endif
        }

        soft_bits[k] = logf(bit_1) - logf(bit_0);
#if DEBUG
        printf(" {0 : %12.8f, 1 : %12.8f}\n", bit_0, bit_1);
#endif
    }

    // 
    // demodulate using internal method
    //
    q = modemcf_create(ms);
    unsigned int sym_out_tab;
    float soft_bits_tab[bps];
    modemcf_demodulate_soft_tab(q,c,cp,p,r,&sym_out_tab,soft_bits_tab);
    modemcf_destroy(q);

    // print results
    printf("\n");
    printf("  input symbol : ");
    print_bitstring(sym_in,bps);
    printf(" {%12.8f, %12.8f}\n", crealf(c[sym_in]), cimagf(c[sym_in]));

    printf("  soft bits :\n");
    printf("   /----------- LLR -----------\\       /------- min dist --------\\\n");
    for (k=0; k<bps; k++) {
        printf("    %1u : ", (sym_in >> (bps-k-1)) & 0x01);
        printf("%12.8f > ", soft_bits[k]);
        int soft_bit = (soft_bits[k]*16 + 127);
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit =   0;
        printf("%5d > %1u", soft_bit, soft_bit & 0x80 ? 1 : 0);

        //
        // print tabular method
        //
        printf("        ");
        printf("%12.8f > ", soft_bits_tab[k]);
        soft_bit = (soft_bits_tab[k]*16 + 127);
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit =   0;
        printf("%5d > %1u", soft_bit, soft_bit & 0x80 ? 1 : 0);

        printf("\n");
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
    // print lines to 'nearest neighbor' points
    fprintf(fid,"plot(");
    for (i=0; i<p; i++) {
        float complex x_hat = c[ cp[sym_out_tab*p+i] ];
        fprintf(fid,"[%12.8f %12.8f],[%12.8f %12.8f],'Color',[1 1 1]*0.8",
                crealf(r), crealf(x_hat),
                cimagf(r), cimagf(x_hat));
        if (i == p-1) fprintf(fid,");\n");
        else          fprintf(fid,",...\n     ");
    }
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

// soft demodulation
//  _q          :   demodulator object
//  _c          :   constellation table
//  _cp         :   nearest neighbor table [size: M*_p x 1]
//  _p          :   size of table
//  _r          :   received sample
//  _s          :   hard demodulator output
//  _soft_bits  :   soft bit output (approximate log-likelihood ratio)
void modemcf_demodulate_soft_tab(modemcf _q,
                               float complex * _c,
                               unsigned int * _cp,
                               unsigned int _p,
                               float complex _r,
                               unsigned int * _s,
                               float * _soft_bits)
{
#if DEBUG
    printf("\nmodem_demodulate_soft_tab() invoked\n");
#endif
    // run hard demodulation
    modemcf_demodulate(_q, _r, _s);
#if DEBUG
    printf("  hard demod    :   %3u\n", *_s);
#endif

    unsigned int bps = modemcf_get_bps(_q);

    float sig = 0.2f;

    unsigned int k;
    unsigned int i;
    unsigned int bit;
    float d;
    for (k=0; k<bps; k++) {
        // initialize soft bit value
        _soft_bits[k] = 0.0f;

        // find nearest 0 and nearest 1
        float dmin_0 = 1.0f;
        float dmin_1 = 1.0f;

        // check bit of hard demodulation
        d = crealf( (_r-_c[*_s])*conjf(_r-_c[*_s]) );
        bit = (*_s >> (bps-k-1)) & 0x01;
        if (bit) dmin_1 = d;
        else     dmin_0 = d;

        // check symbols in table
#if DEBUG
        printf("  index %2u : ", k);
#endif
        for (i=0; i<_p; i++) {
            bit = (_cp[(*_s)*_p+i] >> (bps-k-1)) & 0x01;

#if DEBUG
            print_bitstring(_cp[(*_s)*_p+i],bps);
            printf("[%1u]", bit);
#endif

            // compute distance
            float complex x_hat = _c[ _cp[(*_s)*_p + i] ];
            d = crealf( (_r-x_hat)*conjf(_r-x_hat) );
#if DEBUG
            printf("(%8.6f) ", d);
#endif
            if (bit) {
                if (d < dmin_1) dmin_1 = d;
            } else {
                if (d < dmin_0) dmin_0 = d;
            }
        }
#if DEBUG
        printf("\n");
        printf("  dmin_0 : %12.8f\n", dmin_0);
        printf("  dmin_1 : %12.8f\n", dmin_1);
#endif

        // make assignments
#if 0
        _soft_bits[k] =   logf(expf(-dmin_1/(2.0f*sig*sig)))
                        - logf(expf(-dmin_0/(2.0f*sig*sig)));
#else
        _soft_bits[k] =   (-dmin_1/(2.0f*sig*sig))
                        - (-dmin_0/(2.0f*sig*sig));
#endif
    }
}

