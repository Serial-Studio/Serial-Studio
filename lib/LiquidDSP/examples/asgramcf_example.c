// ASCII spectrogram example for complex inputs. This example demonstrates
// the functionality of the ASCII spectrogram. A sweeping complex sinusoid
// is generated and the resulting spectral periodogram is printed to the
// screen.

#include <unistd.h> // usleep
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "liquid.h"

int main() {
    // options
    unsigned int nfft        =   64;    // transform size
    unsigned int num_frames  =  200;    // total number of frames
    unsigned int msdelay     =   25;    // delay between transforms [ms]
    float        noise_floor = -40.0f;  // noise floor

    // initialize objects
    asgramcf q = asgramcf_create(nfft);
    asgramcf_set_scale(q, noise_floor+15.0f, 5.0f);

    unsigned int i;
    unsigned int n;
    float theta  = 0.0f;    // current instantaneous phase
    float dtheta = 0.0f;    // current instantaneous frequency
    float phi    = 0.0f;    // phase of sinusoidal frequency drift
    float dphi   = 0.003f;  // frequency of sinusoidal frequency drift

    float complex x[nfft];
    float nstd = powf(10.0f,noise_floor/20.0f);  // noise standard deviation
    for (n=0; n<num_frames; n++) {
        // generate a frame of data samples
        for (i=0; i<nfft; i++) {
            // complex exponential
            x[i] = cexpf(_Complex_I*theta);

            // add noise to signal
            x[i] += nstd * (randnf() + _Complex_I*randnf()) * M_SQRT1_2;

            // adjust frequency and phase
            theta  += dtheta;
            dtheta =  0.9f*M_PI*sinf(phi) * liquid_hamming(n, num_frames);
            phi    += dphi;
        }

        // write block of samples to the spectrogram object
        asgramcf_write(q, x, nfft);

        // print result to screen
        asgramcf_print(q);

        // sleep for some time before generating the next frame
        usleep(msdelay*1000);
    }

    asgramcf_destroy(q);
    printf("done.\n");
    return 0;
}

