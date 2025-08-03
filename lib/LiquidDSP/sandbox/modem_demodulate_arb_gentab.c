// Generate fast demodulation tables for arbitrary modems; use existing
// modulation scheme with fast decoding (e.g. 16-QAM) and associate
// these constellation points with the arbitrary constellation points.
// The goal is to allow the demodulator to first demodulate the
// received point using the fast demodulator and then perform a search
// over only the set of nearby points. This script computes the look-up
// table for these nearby points and stores them as an array of
// indices.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include "liquid.h"

#define OUTPUT_FILENAME "modem_demodulate_arb_gentab.m"
#define DEBUG 0

// print usage/help message
void usage()
{
    printf("sandbox/modem_demodulate_arb_gentab [options]\n");
    printf("  u/h   : print usage\n");
    printf("  m     : input modulation scheme (arb64vt default)\n");
    printf("  r     : reference modulation scheme (qam16 default)\n");
    liquid_print_modulation_schemes();
    printf("  a     : alpha (reference scheme gain), default: 1.0\n");
}

// search for nearest constellation points to reference points
void modemcf_arbref_search(float complex * _c,
                         unsigned int _M,
                         float complex * _cref,
                         unsigned int _p,
                         unsigned char * _index,
                         unsigned int _s);

// search for nearest constellation points to single reference point
void modemcf_arbref_search_point(float complex * _c,
                               unsigned int _M,
                               float complex _cref,
                               unsigned char * _index,
                               unsigned int _s);

// find unassigned constellation points
unsigned int modemcf_arbref_search_unassigned(unsigned char * _index,
                                            unsigned int _M,
                                            unsigned int _p,
                                            unsigned int _s,
                                            unsigned char * _assigned);

int main(int argc, char*argv[])
{
    // create mod/demod objects
    modulation_scheme ms = LIQUID_MODEM_ARB64VT;
    modulation_scheme mref = LIQUID_MODEM_QAM64;
    float alpha = 1.0f;

    int dopt;
    while ((dopt = getopt(argc,argv,"uhp:m:r:a:")) != EOF) {
        switch (dopt) {
        case 'u':
        case 'h': usage(); return 0;
        case 'm':
            ms = liquid_getopt_str2mod(optarg);
            if (ms == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown/unsupported modulation scheme '%s'\n", argv[0], optarg);
                return 1;
            }
            break;
        case 'r':
            mref = liquid_getopt_str2mod(optarg);
            if (mref == LIQUID_MODEM_UNKNOWN) {
                fprintf(stderr,"error: %s, unknown/unsupported modulation scheme '%s'\n", argv[0], optarg);
                return 1;
            }
            break;
        case 'a': alpha = atof(optarg); break;
        default:
            exit(1);
        }
    }

    // validate input

    unsigned int i;
    unsigned int j;

    // initialize reference points
    modemcf qref = modemcf_create(mref);
    unsigned int kref = modemcf_get_bps(qref);
    unsigned int p = 1 << kref;
    float complex cref[p];
    for (i=0; i<p; i++) {
        modemcf_modulate(qref, i, &cref[i]);
        cref[i] *= alpha;
    }
    modemcf_destroy(qref);

    // generate the constellation
    modemcf q = modemcf_create(ms);
    unsigned int bps = modemcf_get_bps(q);
    unsigned int M = 1 << bps;
    float complex constellation[M];
    for (i=0; i<M; i++)
        modemcf_modulate(q, i, &constellation[i]);
    modemcf_destroy(q);

    // perform search
    unsigned char * link = NULL;
    unsigned int num_unassigned=M;
    unsigned char unassigned[M];
    unsigned int s=0;  // number of points per reference

    // run search until all points are found
    do {
        // increment number of points per reference
        s++;

        // reallocte memory for links
        link = (unsigned char*) realloc(link, p*s);

        // search for nearest constellation points to reference points
        modemcf_arbref_search(constellation, M, cref, p, link, s);

        // find unassigned constellation points
        num_unassigned = modemcf_arbref_search_unassigned(link,M,p,s,unassigned);
        printf("%3u : number of unassigned points: %3u / %3u\n", s, num_unassigned, M);
    } while (num_unassigned > 0);

    // print table
    printf("\n");
    printf("unsigned char modemcf_demodulate_gentab[%u][%u] = {\n", p, s);
    for (i=0; i<p; i++) {
        printf("    {");
        for (j=0; j<s; j++) {
            printf("%3u%s", link[i*s+j], j==(s-1) ? "" : ",");
        }
        printf("}%s", i==(p-1) ? "};\n" : ",\n");
    }

    // 
    // export output file
    //
    FILE * fid = fopen(OUTPUT_FILENAME,"w");
    fprintf(fid,"%% %s : auto-generated file\n", OUTPUT_FILENAME);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"bps = %u;\n", bps);
    fprintf(fid,"M = %u;\n", M);
    fprintf(fid,"p = %u;\n", p);
    fprintf(fid,"s = %u;\n", s);

    // save constellation points
    for (i=0; i<M; i++) {
        fprintf(fid,"c(%3u) = %12.4e + j*%12.4e;\n", i+1,
                                                     crealf(constellation[i]),
                                                     cimagf(constellation[i]));
    }

    // save reference points
    for (i=0; i<p; i++)
        fprintf(fid,"cref(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(cref[i]), cimagf(cref[i]));

    // save reference matrix
    fprintf(fid,"link = zeros(p,s);\n");
    for (i=0; i<p; i++) {
        for (j=0; j<s; j++) {
            fprintf(fid,"link(%3u,%3u) = %u;\n", i+1, j+1, link[i*s+j]+1);
        }
    }

    // plot results
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(real(c),    imag(c),   'bx',\n");
    fprintf(fid,"     real(cref), imag(cref),'or');\n");

    // draw lines between reference points and associated constellation points
    fprintf(fid,"hold on;\n");
    fprintf(fid,"  for i=1:p,\n");
    fprintf(fid,"    for j=1:s,\n");
    fprintf(fid,"      plot([real(cref(i)) real(c(link(i,j)))], [imag(cref(i)) imag(c(link(i,j)))], '-', 'Color', 0.8*[1 1 1]);\n");
    fprintf(fid,"    end;\n");
    fprintf(fid,"  end;\n");
    fprintf(fid,"hold off;\n");
    fprintf(fid,"xlabel('in-phase');\n");
    fprintf(fid,"ylabel('quadrature phase');\n");
    fprintf(fid,"%%legend('constellation','reference','links',0);\n");
    fprintf(fid,"title(['Arbitrary ' num2str(M) '-QAM']);\n");
    fprintf(fid,"axis([-1 1 -1 1]*1.9);\n");
    fprintf(fid,"axis square;\n");
    fprintf(fid,"grid on;\n");
    fclose(fid);

    printf("results written to '%s'\n", OUTPUT_FILENAME);
    printf("done.\n");

    // free allocated memory
    free(link);

    return 0;
}

// search for nearest constellation points to reference points
//  _c      :   input constellation [size: _M x 1]
//  _M      :   input constellation size
//  _cref   :   reference points [size: _p x 1]
//  _p      :   reference points size
//  _index  :   indices of nearest constellation points [size: _p x _s]
//  _s      :   number of nearest constellation points
void modemcf_arbref_search(float complex * _c,
                         unsigned int _M,
                         float complex * _cref,
                         unsigned int _p,
                         unsigned char * _index,
                         unsigned int _s)
{
    // validate input
    if (_M < 2) {
        fprintf(stderr,"error: modemcf_arbref_search(), input constellation size too small\n");
        exit(1);
    } else if (_s > _M) {
        fprintf(stderr,"error: modemcf_arbref_search(), index size exceeds constellation size\n");
        exit(1);
    }

    //
    unsigned int i;
    for (i=0; i<_p; i++)
        modemcf_arbref_search_point(_c, _M, _cref[i], &_index[_s*i], _s);
}

// search for nearest constellation points to single reference point
void modemcf_arbref_search_point(float complex * _c,
                               unsigned int _M,
                               float complex _cref,
                               unsigned char * _index,
                               unsigned int _s)
{
#if DEBUG
    printf("searching neighbors to (%8.3f,%8.3f)\n", crealf(_cref), cimagf(_cref));
#endif
    // initialize array of selected element flags
    unsigned char selected[_M];
    memset(selected, 0x00, _M);

    unsigned int i;
    unsigned int n;
    
    for (n=0; n<_s; n++) {
        int min_found = 0;
        float d;
        float dmin = 0.0f;
        unsigned int index_min = 0;
        for (i=0; i<_M; i++) {
            // ignore constellation point if it has already been
            // selected
            if (selected[i]==1)
                continue;

            // compute distance
            d = crealf( (_c[i]-_cref)*conjf(_c[i]-_cref) );
            if ( d < dmin || !min_found ) {
                dmin = d;
                index_min = i;
                min_found = 1;
            }
        }

        // save minimum index
        _index[n] = index_min;

        // flag point as 'selected'
        selected[index_min] = 1;

#if DEBUG
        printf("%6u (%8.3f,%8.3f)\n", index_min, crealf(_c[index_min]), cimagf(_c[index_min]));
#endif
    }
}

// find unassigned constellation points
unsigned int modemcf_arbref_search_unassigned(unsigned char * _index,
                                            unsigned int _M,
                                            unsigned int _p,
                                            unsigned int _s,
                                            unsigned char * _assigned)
{
    unsigned int num_unassigned = 0;

    //
    unsigned int i;
    for (i=0; i<_M; i++) {
        // initialized 'assigned' flag to false
        _assigned[i] = 0;

        unsigned int j;
        for (j=0; j<_p; j++) {

            // see if this constellation point has been assigned
            // to this reference
            unsigned int k;
            for (k=0; k<_s; k++) {
                if (_index[j*_s+k] == i)
                    _assigned[i] = 1;
            }
        }

        if (!_assigned[i])
            num_unassigned++;
    }

    return num_unassigned;
}

