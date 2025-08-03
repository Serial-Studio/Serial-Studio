/*
 * Copyright (c) 2007 - 2024 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// spwaterfall (spectral periodogram waterfall)
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include <complex.h>
#include "liquid.internal.h"

struct SPWATERFALL(_s) {
    // options
    unsigned int    nfft;           // FFT length
    unsigned int    time;           // minimum time buffer
    SPGRAM()        periodogram;    // spectral periodogram object

    // buffers
    T *             psd;            // time/frequency buffer [nfft x 2*time]
    unsigned int    index_time;     // time index for writing to buffer
    unsigned int    rollover;       // number of FFTs to take before writing to output

    // parameters for display purposes only
    float           frequency;      // center frequency [Hz]
    float           sample_rate;    // sample rate [Hz]
    unsigned int    width;          // image width [pixels]
    unsigned int    height;         // image height [pixels]
    char *          commands;       // commands to execute directly before 'plot'
};

//
// internal methods
//

// compute spectral periodogram output (complex values) from internal periodogram object
int SPWATERFALL(_step)(SPWATERFALL() _q);

// consolidate buffer by taking log-average of two separate spectral estimates in time
int SPWATERFALL(_consolidate_buffer)(SPWATERFALL() _q);

// export files
int SPWATERFALL(_export_bin)(SPWATERFALL() _q, const char * _base);
int SPWATERFALL(_export_gnu)(SPWATERFALL() _q, const char * _base);

// create spwaterfall object
//  _nfft       : FFT size
//  _wtype      : window type, e.g. LIQUID_WINDOW_HAMMING
//  _window_len : window length
//  _delay      : delay between transforms, _delay > 0
SPWATERFALL() SPWATERFALL(_create)(unsigned int _nfft,
                                   int          _wtype,
                                   unsigned int _window_len,
                                   unsigned int _delay,
                                   unsigned int _time)
{
    // validate input
    if (_nfft < 2)
        return liquid_error_config("spwaterfall%s_create(), fft size must be at least 2", EXTENSION);
    if (_window_len > _nfft)
        return liquid_error_config("spwaterfall%s_create(), window size cannot exceed fft size", EXTENSION);
    if (_window_len == 0)
        return liquid_error_config("spwaterfall%s_create(), window size must be greater than zero", EXTENSION);
    if (_wtype == LIQUID_WINDOW_KBD && _window_len % 2)
        return liquid_error_config("spwaterfall%s_create(), KBD window length must be even", EXTENSION);
    if (_delay == 0)
        return liquid_error_config("spwaterfall%s_create(), delay must be greater than 0", EXTENSION);
    if (_time == 0)
        return liquid_error_config("spwaterfall%s_create(), time must be greater than 0", EXTENSION);

    // allocate memory for main object
    SPWATERFALL() q = (SPWATERFALL()) malloc(sizeof(struct SPWATERFALL(_s)));

    // set input parameters
    q->nfft         = _nfft;
    q->time         = _time;
    q->frequency    =  0;
    q->sample_rate  = -1;
    q->width        = 800;
    q->height       = 800;
    q->commands     = NULL;

    // create buffer to hold aggregated power spectral density
    // NOTE: the buffer is two-dimensional time/frequency grid that is two times
    //       'nfft' and 'time' to account for log-average consolidation each time
    //       the buffer gets filled
    q->psd = (T*) malloc( 2 * q->nfft * q->time * sizeof(T));

    // create spectral periodogram object
    q->periodogram = SPGRAM(_create)(_nfft, _wtype, _window_len, _delay);

    // reset the object
    SPWATERFALL(_reset)(q);

    // return new object
    return q;
}

// create default spwaterfall object (Kaiser-Bessel window)
SPWATERFALL() SPWATERFALL(_create_default)(unsigned int _nfft,
                                           unsigned int _time)
{
    // validate input
    if (_nfft < 2)
        return liquid_error_config("spwaterfall%s_create_default(), fft size must be at least 2", EXTENSION);
    if (_time < 2)
        return liquid_error_config("spwaterfall%s_create_default(), fft size must be at least 2", EXTENSION);

    return SPWATERFALL(_create)(_nfft, LIQUID_WINDOW_KAISER, _nfft/2, _nfft/4, _time);
}

SPWATERFALL() SPWATERFALL(_copy)(SPWATERFALL() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("spwaterfall%s_copy(), object cannot be NULL", EXTENSION);

    // allocate memory for main object
    SPWATERFALL() q_copy = (SPWATERFALL()) malloc(sizeof(struct SPWATERFALL(_s)));

    // copy all memory and overwrite specific values
    memmove(q_copy, q_orig, sizeof(struct SPWATERFALL(_s)));

    // copy spectral periodogram object and internal state
    q_copy->periodogram = SPGRAM(_copy)(q_orig->periodogram);

    // create new buffer and copy values
    q_copy->psd = (T*) malloc( 2 * q_copy->nfft * q_copy->time * sizeof(T));
    memmove(q_copy->psd, q_orig->psd, 2 * q_copy->nfft * q_copy->time * sizeof(T));

    // set commands value
    q_copy->commands = NULL;
    SPWATERFALL(_set_commands)(q_copy, q_orig->commands);

    // return object
    return q_copy;
}

// destroy spwaterfall object
int SPWATERFALL(_destroy)(SPWATERFALL() _q)
{
    // free allocated memory
    free(_q->psd);
    free(_q->commands);

    // destroy internal spectral periodogram object
    SPGRAM(_destroy)(_q->periodogram);

    // free main object
    free(_q);
    return LIQUID_OK;
}

// clears the internal state of the spwaterfall object, but not
// the internal buffer
int SPWATERFALL(_clear)(SPWATERFALL() _q)
{
    SPGRAM(_clear)(_q->periodogram);
    memset(_q->psd, 0x00, 2*_q->nfft*_q->time*sizeof(T));
    _q->index_time = 0;
    return LIQUID_OK;
}

// reset the spwaterfall object to its original state completely
int SPWATERFALL(_reset)(SPWATERFALL() _q)
{
    SPWATERFALL(_clear)(_q);
    SPGRAM(_reset)(_q->periodogram);
    _q->rollover = 1;
    return LIQUID_OK;
}

// prints the spwaterfall object's parameters
int SPWATERFALL(_print)(SPWATERFALL() _q)
{
    printf("<spwaterfall%s: nfft=%u, time=%u>\n", EXTENSION, _q->nfft, _q->time);
    return LIQUID_OK;
}

// Get number of samples processed since object was created
uint64_t SPWATERFALL(_get_num_samples_total)(SPWATERFALL() _q)
{
    return SPGRAM(_get_num_samples_total)(_q->periodogram);
}

// Get FFT size (columns in PSD output)
unsigned int SPWATERFALL(_get_num_freq)(SPWATERFALL() _q)
{
    return _q->nfft;
}

// Get number of accumulated FFTs (rows in PSD output)
unsigned int SPWATERFALL(_get_num_time)(SPWATERFALL() _q)
{
    return _q->index_time;
}

// Get window length used in spectral estimation
unsigned int SPWATERFALL(_get_window_len)(SPWATERFALL() _q)
{
    return SPGRAM(_get_window_len)(_q->periodogram);
}

// Get delay between transforms used in spectral estimation
unsigned int SPWATERFALL(_get_delay)(SPWATERFALL() _q)
{
    return SPGRAM(_get_delay)(_q->periodogram);
}

// Get window type used in spectral estimation
int SPWATERFALL(_get_wtype)(SPWATERFALL() _q)
{
    return SPGRAM(_get_wtype)(_q->periodogram);
}

// Get power spectral density (PSD), size: nfft x time
const float * SPWATERFALL(_get_psd)(SPWATERFALL() _q)
{
    return (const T *) _q->psd;
}

// set center freuqncy
int SPWATERFALL(_set_freq)(SPWATERFALL() _q,
                           float         _freq)
{
    _q->frequency = _freq;
    return LIQUID_OK;
}

// set sample rate
int SPWATERFALL(_set_rate)(SPWATERFALL() _q,
                           float         _rate)
{
    // validate input
    if (_rate <= 0.0f)
        return liquid_error(LIQUID_EICONFIG,"spwaterfall%s_set_rate(), sample rate must be greater than zero", EXTENSION);

    _q->sample_rate = _rate;
    return LIQUID_OK;
}

// set image dimensions
int SPWATERFALL(_set_dims)(SPWATERFALL() _q,
                           unsigned int  _width,
                           unsigned int  _height)
{
    _q->width  = _width;
    _q->height = _height;
    return LIQUID_OK;
}

// set image dimensions
int SPWATERFALL(_set_commands)(SPWATERFALL() _q,
                               const char *  _commands)
{
    // clear memory with NULL pointer
    if (_commands == NULL) {
        free(_q->commands);
        _q->commands = NULL;
        return LIQUID_OK;
    }

    // sanity check
    unsigned int n = strlen(_commands);
    if (n > 1<<14) {
        SPWATERFALL(_set_commands)(_q, "# error: input string size limit exceeded");
        return liquid_error(LIQUID_EICONFIG,"spwaterfall%s_set_commands(), input string size exceeds reasonable limits",EXTENSION);
    }

    // reallocate memory, copy input, and return
    _q->commands = (char*) realloc(_q->commands, n+1);
    memmove(_q->commands, _commands, n);
    _q->commands[n] = '\0';
    return LIQUID_OK;
}

// push a single sample into the spwaterfall object
//  _q      :   spwaterfall object
//  _x      :   input sample
int SPWATERFALL(_push)(SPWATERFALL() _q,
                       TI            _x)
{
    if (SPGRAM(_push)(_q->periodogram, _x))
        return liquid_error(LIQUID_EINT,"spwaterfall%s_push(), could not push to internal spgram object",EXTENSION);
    if (SPWATERFALL(_step)(_q))
        return liquid_error(LIQUID_EINT,"spwaterfall%s_push(), could not step internal state",EXTENSION);

    return LIQUID_OK;
}

// write a block of samples to the spwaterfall object
//  _q      :   spwaterfall object
//  _x      :   input buffer [size: _n x 1]
//  _n      :   input buffer length
int SPWATERFALL(_write)(SPWATERFALL() _q,
                        TI *          _x,
                        unsigned int  _n)
{
    // TODO: be smarter about how to write and execute samples
    unsigned int i;
    for (i=0; i<_n; i++)
        SPWATERFALL(_push)(_q, _x[i]);
    return LIQUID_OK;
}

// export output files
//  _q    : spwaterfall object
//  _base : base filename
int SPWATERFALL(_export)(SPWATERFALL() _q,
                         const char *  _base)
{
    if (SPWATERFALL(_export_bin)(_q, _base))
        return liquid_error(LIQUID_EIO,"spwaterfall%s_export(), could not export binary file to '%s.bin'",EXTENSION,_base);
    if (SPWATERFALL(_export_gnu)(_q, _base))
        return liquid_error(LIQUID_EIO,"spwaterfall%s_export(), could not export gnuplot file to '%s.gnu'",EXTENSION,_base);
    return LIQUID_OK;
}

// compute spectral periodogram output from current buffer contents
//  _q : spwaterfall object
int SPWATERFALL(_step)(SPWATERFALL() _q)
{
    // determine if we need to extract PSD estimate from periodogram
    if (SPGRAM(_get_num_transforms)(_q->periodogram) >= _q->rollover) {
        //printf("index : %u\n", _q->index_time);
        // get PSD estimate from periodogram object, placing result in
        // proper location in internal buffer
        SPGRAM(_get_psd)(_q->periodogram, _q->psd + _q->nfft*_q->index_time);

        // soft reset of internal state, counters
        SPGRAM(_clear)(_q->periodogram);

        // increment buffer counter
        _q->index_time++;

        // determine if buffer is full and we need to consolidate buffer
        if (_q->index_time == 2*_q->time)
            SPWATERFALL(_consolidate_buffer)(_q);
    }
    return LIQUID_OK;
}

// consolidate buffer by taking log-average of two separate spectral estimates in time
//  _q : spwaterfall object
int SPWATERFALL(_consolidate_buffer)(SPWATERFALL() _q)
{
    // assert(_q->index_time == 2*_q->time);
    //printf("consolidating... (rollover = %10u, total samples : %16llu, index : %u)\n",
    //        _q->rollover, SPGRAM(_get_num_samples_total)(_q->periodogram), _q->index_time);
    unsigned int i; // time index
    unsigned int k; // freq index
    for (i=0; i<_q->time; i++) {
        for (k=0; k<_q->nfft; k++) {
            // convert to linear, compute average, convert back to log
            T v0 = powf(10.0f, _q->psd[ (2*i + 0)*_q->nfft + k ]*0.1f);
            T v1 = powf(10.0f, _q->psd[ (2*i + 1)*_q->nfft + k ]*0.1f);

            // save result
            _q->psd[ i*_q->nfft + k ] = 10.0f*log10f(0.5f*(v0+v1));
        }
    }

    // update time index
    _q->index_time = _q->time;

    // update rollover counter
    _q->rollover *= 2;
    return LIQUID_OK;
}

// export gnuplot file
//  _q        : spwaterfall object
//  _filename : input buffer [size: _n x 1]
int SPWATERFALL(_export_bin)(SPWATERFALL() _q,
                             const char *  _base)
{
    // add '.bin' extension to base
    int n = strlen(_base);
    char filename[n+5];
    sprintf(filename,"%s.bin", _base);

    // open output file for writing
    FILE * fid = fopen(filename,"w");
    if (fid == NULL) {
        liquid_error(LIQUID_EICONFIG,"spwaterfall%s_export_bin(), could not open '%s' for writing",
                EXTENSION, filename);
        return -1;
    }

    unsigned int i;
    
    // write header
    float nfftf = (float)(_q->nfft);
    fwrite(&nfftf, sizeof(float), 1, fid);
    for (i=0; i<_q->nfft; i++) {
        float f = (float)i/nfftf - 0.5f;
        fwrite(&f, sizeof(float), 1, fid);
    }
    
    // write output spectral estimate
    // TODO: force conversion from type 'T' to type 'float'
    uint64_t total_samples = SPGRAM(_get_num_samples_total)(_q->periodogram);
    for (i=0; i<_q->index_time; i++) {
        float n = (float)i / (float)(_q->index_time) * (float)total_samples;
        fwrite(&n, sizeof(float), 1, fid);
        fwrite(&_q->psd[i*_q->nfft], sizeof(float), _q->nfft, fid);
    }

    // close it up
    fclose(fid);
    //printf("results written to %s\n", filename);
    return LIQUID_OK;
}

// export gnuplot file
//  _q        : spwaterfall object
//  _filename : input buffer [size: _n x 1]
int SPWATERFALL(_export_gnu)(SPWATERFALL() _q,
                             const char *  _base)
{
    // add '.bin' extension to base
    int n = strlen(_base);
    char filename[n+5];
    sprintf(filename,"%s.gnu", _base);

    // open output file for writing
    FILE * fid = fopen(filename,"w");
    if (fid == NULL)
        return liquid_error(LIQUID_EICONFIG,"spwaterfall%s_export_gnu(), could not open '%s' for writing",EXTENSION,filename);
    
    // scale to thousands, millions, billions (etc.) automatically
    uint64_t total_samples = SPGRAM(_get_num_samples_total)(_q->periodogram);
    char units  = ' ';
    float scale = 1.0f;
    liquid_get_scale((float)total_samples/4, &units, &scale);

    fprintf(fid,"#!/usr/bin/gnuplot\n");
    fprintf(fid,"reset\n");
    fprintf(fid,"set terminal png size %u,%u enhanced font 'Verdana,10'\n", _q->width, _q->height);
    fprintf(fid,"set output '%s.png'\n", _base);
    fprintf(fid,"unset key\n");
    fprintf(fid,"set style line 11 lc rgb '#808080' lt 1\n");
    fprintf(fid,"set border 3 front ls 11\n");
    fprintf(fid,"set style line 12 lc rgb '#888888' lt 0 lw 1\n");
    fprintf(fid,"set grid front ls 12\n");
    fprintf(fid,"set tics nomirror out scale 0.75\n");
    fprintf(fid,"set yrange [0:%f]\n", (float)(total_samples-1)*scale);
    fprintf(fid,"set ylabel 'Sample Index'\n");
    fprintf(fid,"set format y '%%.0f %c'\n", units);
    fprintf(fid,"# disable colorbar tics\n");
    fprintf(fid,"set cbtics scale 0\n");
    fprintf(fid,"set palette negative defined ( \\\n");
    fprintf(fid,"    0 '#D53E4F',\\\n");
    fprintf(fid,"    1 '#F46D43',\\\n");
    fprintf(fid,"    2 '#FDAE61',\\\n");
    fprintf(fid,"    3 '#FEE08B',\\\n");
    fprintf(fid,"    4 '#E6F598',\\\n");
    fprintf(fid,"    5 '#ABDDA4',\\\n");
    fprintf(fid,"    6 '#66C2A5',\\\n");
    fprintf(fid,"    7 '#3288BD' )\n");
    fprintf(fid,"\n");
    if (_q->sample_rate < 0) {
        fprintf(fid,"set xrange [-0.5:0.5]\n");
        float xtics = 0.1f;
        fprintf(fid,"set xtics %f\n", xtics);
        fprintf(fid,"set xlabel 'Normalized Frequency [f/F_s]'\n");
        if (_q->commands != NULL)
            fprintf(fid,"%s\n", _q->commands);
        fprintf(fid,"plot '%s.bin' u 1:($2*%e):3 binary matrix with image\n", _base, scale);
    } else {
        char unit;
        float g = 1.0f;
        float f_hi = _q->frequency + 0.5f*_q->sample_rate; // highest frequency
        liquid_get_scale(f_hi/2, &unit, &g);
        fprintf(fid,"set xlabel 'Frequency [%cHz]'\n", unit);
        // target xtics spacing roughly every 60-80 pixels
        float xn = ((float) _q->width * 0.8f) / 70.0f;  // rough number of tics
        //float xs = _q->sample_rate * g / xn;            // normalized spacing
        float xt = 1.0f;                                // round to nearest 1, 2, 5, or 10
        // potential xtic spacings
        float spacing[] = {0.01,0.02,0.05,0.1,0.2,0.5,1.0,2.0,5.0,10.0,20.0,50.0,100.0,200.0,500.0,-1.0f};
        unsigned int i=0;
        while (spacing[i] > 0) {
            if (_q->sample_rate*g/spacing[i] < 1.2f*xn) {
                xt = spacing[i];
                break;
            }
            i++;
        }
        //printf("xn:%f, xs:%f, xt:%f\n", xn, xs, xt);
        fprintf(fid,"set xrange [%f:%f]\n", g*(_q->frequency-0.5*_q->sample_rate), g*(_q->frequency+0.5*_q->sample_rate));
        fprintf(fid,"set xtics %f\n", xt);
        if (_q->commands != NULL)
            fprintf(fid,"%s\n", _q->commands);
        fprintf(fid,"plot '%s.bin' u ($1*%f+%f):($2*%e):3 binary matrix with image\n",
                _base,
                g*(_q->sample_rate < 0 ? 1 : _q->sample_rate),
                g*_q->frequency,
                scale);
    }
    fclose(fid);

    // close it up
#if 0
    printf("results written to %s\n", filename);
    printf("index time       : %u\n", _q->index_time);
    printf("rollover         : %u\n", _q->rollover);
    printf("total transforms : %llu\n", SPGRAM(_get_num_transforms_total)(_q->periodogram));
#endif
    return LIQUID_OK;
}

