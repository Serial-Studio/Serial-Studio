// ASCII spectrogram example for real-valued input. This example demonstrates
// the functionality of the ASCII spectrogram for real-valued input siganls.
// A cosine signal with time-varying frequency is generated and the resulting
// spectral periodogram is printed to the screen. Because the time signal has
// no complex component, its spectrum is symmetric.

#include <unistd.h> // usleep
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
    asgramf q = asgramf_create(nfft);
    asgramf_set_scale(q, noise_floor+15.0f, 5.0f);

    unsigned int i;
    unsigned int n;
    float theta  = 0.0f;    // current instantaneous phase
    float dtheta = 0.0f;    // current instantaneous frequency
    float phi    = 0.0f;    // phase of sinusoidal frequency drift
    float dphi   = 0.003f;  // frequency of sinusoidal frequency drift

    float nstd = powf(10.0f,noise_floor/20.0f);  // noise standard deviation
    for (n=0; n<num_frames; n++) {
        // generate a frame of data samples
        for (i=0; i<nfft; i++) {
            // cosine wave of time-varying frequency with noise
            float x = cosf(theta) + nstd*randnf();

            // push sample into spectrogram object
            asgramf_push(q, x);

            // adjust frequency and phase
            theta  += dtheta;
            dtheta =  0.5f*M_PI + 0.4f*M_PI*sinf(phi) * liquid_hamming(n, num_frames);
            phi    += dphi;
        }

        // print the spectrogram to stdout
        asgramf_print(q);

        // sleep for some time before generating the next frame
        usleep(msdelay*1000);
    }

    asgramf_destroy(q);
    printf("done.\n");
    return 0;
}

