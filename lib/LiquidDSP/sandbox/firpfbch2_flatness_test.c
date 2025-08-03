// test spectral flatness of reconstruction
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "liquid.h"

int main(int argc, char*argv[])
{
    // options
    unsigned int M=24;              // number of channels (must be even)
    unsigned int m=2;               // filter delay
    float        As = 60.0f;        // stop-band suppression
    unsigned int num_blocks=5000000;// number of symbols
    unsigned int nfft=1200;         //

    // create filterbank objects from prototype
    firpfbch2_crcf qa = firpfbch2_crcf_create_kaiser(LIQUID_ANALYZER,    M, m, As);
    firpfbch2_crcf qs = firpfbch2_crcf_create_kaiser(LIQUID_SYNTHESIZER, M, m, As);
    spgramcf q = spgramcf_create_default(nfft);
    //spwaterfallcf psd = spwaterfallcf_create(1200,LIQUID_WINDOW_HAMMING,800,10,1200);

    // derived values
    unsigned int M2 = M/2;
    float complex buf_0[M2];
    float complex buf_1[M ];
    float complex buf_2[M2];

    unsigned int i,j;
    float phi = 0.0f, dphi = 0.1*2*M_PI;
    for (i=0; i<num_blocks; i++) {
        // fill input with noise
        for (j=0; j<M2; j++) {
            buf_0[j] = ( randnf() + _Complex_I*randnf() ) * M_SQRT1_2;

            buf_0[j] += 0.1*cexpf(_Complex_I*phi);
            phi += dphi;
            if (phi > M_PI) phi -= 2*M_PI;
            if (phi <-M_PI) phi += 2*M_PI;
        }

        // run analysis filterbank
        firpfbch2_crcf_execute(qa, buf_0, buf_1);

        // run synthesis filterbank
        firpfbch2_crcf_execute(qs, buf_1, buf_2);

        // run result through spectrum estimation
        spgramcf_write(q, buf_2, M2);
        //spwaterfallcf_write(psd, buf_2, M2);
    }

    // explort to gnuplot
    spgramcf_export_gnuplot(q,"firpfbch2_flatness_test.gnu");
    //spwaterfallcf_set_dims    (psd,1200, 800);
    //spwaterfallcf_set_commands(psd,"set cbrange [-0.5:0.5]; set title 'waterfall'; unset grid;");
    //spwaterfallcf_export      (psd,"firpfbch2_flatness_test.waterfall");

    // destroy filterbank objects
    firpfbch2_crcf_destroy(qa); // analysis filterbank
    firpfbch2_crcf_destroy(qs); // synthesis filterbank
    spgramcf_destroy(q);
    //spwaterfallcf_destroy(psd);

    printf("done.\n");
    return 0;
}

