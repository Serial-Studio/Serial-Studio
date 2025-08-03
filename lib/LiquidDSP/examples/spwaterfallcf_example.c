// waterfall example

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "liquid.h"

#define OUTPUT_FILENAME "spwaterfallcf_waterfall_example.gnu"

int main()
{
    // spectral periodogram options
    unsigned int nfft        = 1800;    // spectral periodogram FFT size
    unsigned int time        = 1000;    // minimum time buffer
    unsigned int num_samples =  2e6;    // number of samples

    // create spectral waterfall object
    spwaterfallcf periodogram = spwaterfallcf_create_default(nfft,time);
    spwaterfallcf_print(periodogram);

    // create stream generator
    msourcecf gen = msourcecf_create_default();
    
    // add noise source (wide band)
    msourcecf_add_noise(gen, 0.0f, 1.00f, -40);

    // add noise source (narrow-band, pulsed)
    int id_noise = msourcecf_add_noise(gen, 0.4f, 0.10f, -20);

    // add tone
    msourcecf_add_tone(gen, -0.4f, 0.0f, 0);

    // add fsk modem
    msourcecf_add_fsk(gen, -0.35f, 0.03f, -30.0f, 2, 500);

    // add chirp signal
    msourcecf_add_chirp(gen, 0.17f, 0.10f, -50, 250e3, 0, 0);

    // add modulated data
    msourcecf_add_modem(gen, -0.1f, 0.15f, -10, LIQUID_MODEM_QPSK, 12, 0.25);

    // create buffers
    unsigned int  buf_len = 64;
    float complex buf[buf_len];

    // generate signals and push through spwaterfall object
    unsigned int total_samples   = 0;
    int state = 1;
    while (total_samples < num_samples) {
        // write samples to buffer
        msourcecf_write_samples(gen, buf, buf_len);

        // push resulting samples through waterfall object
        spwaterfallcf_write(periodogram, buf, buf_len);

        // accumulated samples
        total_samples += buf_len;

        // update state for noise source
        if (state == 0 && randf() < 1e-3f) {
            state = 1;
            msourcecf_enable(gen, id_noise);
            //printf("turning noise on\n");
        } else if (state == 1 && randf() < 3e-3f) {
            state = 0;
            msourcecf_disable(gen, id_noise);
            //printf("turning noise off\n");
        }
    }
    // export output files
    spwaterfallcf_set_rate    (periodogram,100e6);
    spwaterfallcf_set_freq    (periodogram,750e6);
    spwaterfallcf_set_dims    (periodogram,1200, 800);
    spwaterfallcf_set_commands(periodogram,"set cbrange [-45:25]; set title 'waterfall'");
    spwaterfallcf_export(periodogram,"spwaterfallcf_example");

    // destroy objects
    msourcecf_destroy(gen);
    spwaterfallcf_destroy(periodogram);

    printf("done.\n");
    return 0;
}


