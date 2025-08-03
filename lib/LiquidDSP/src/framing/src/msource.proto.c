/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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
// Generic source generator
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// internal structure
struct MSOURCE(_s)
{
    // internal sources
    QSOURCE() *     sources;        // array of sources
    unsigned int    num_sources;    // number of sources
    int             id_counter;     // qsource id counter

    // channelizer description
    unsigned int    M;              // channelizer size
    unsigned int    m;              // channelizer filter semi-length
    float           as;             // channelizer filter stop-band suppression (dB)
    firpfbch2_crcf  ch;             // analysis channelizer object

    // buffers
    float complex * buf_freq;       // [size: M   x 1]
    float complex * buf_time;       // [size: M/2 x 1]
    unsigned int    read_index;     // output buffer read index
    unsigned int    num_blocks;     // output buffer read index

    // global counters
    unsigned long long num_samples; // total number of samples generated
};

//
// internal methods
//

// find index of qsource object by id within list, return -1 if not found
int MSOURCE(_find)(MSOURCE() _q, int _id);

// find qsource object by id within list, return NULL if not found
QSOURCE() MSOURCE(_get_source)(MSOURCE() _q, int _id);

// add source to list
int MSOURCE(_add_source)(MSOURCE() _q,
                         QSOURCE() _s);

// generate samples internally
int MSOURCE(_generate)(MSOURCE() _q);

// create msource object
MSOURCE() MSOURCE(_create)(unsigned int _M,
                           unsigned int _m,
                           float        _as)
{
    // validate input
    if (_M < 2)
        return liquid_error_config("msource%s_create(), number of subcarriers must be at least 2",EXTENSION);
    if (_M % 2)
        return liquid_error_config("msource%s_create(), number of subcarriers must be even",EXTENSION);
    if (_m==0)
        return liquid_error_config("msource%s_create(), filter semi-length must be greater than zero",EXTENSION);

    // allocate memory for main object
    MSOURCE() q = (MSOURCE()) malloc( sizeof(struct MSOURCE(_s)) );

    //
    q->sources     = NULL;
    q->num_sources = 0;
    q->id_counter  = 0;
    q->M           = _M;
    q->m           = _m;
    q->as          = _as;
    q->num_samples = 0;

    q->ch = firpfbch2_crcf_create_kaiser(LIQUID_SYNTHESIZER, q->M, q->m, q->as);

    q->buf_freq = (float complex*) malloc(q->M   * sizeof(float complex)); 
    q->buf_time = (float complex*) malloc(q->M/2 * sizeof(float complex)); 

    q->read_index = q->M/2; // indicate buffer is empty
    q->num_blocks = 0;

    // reset and return main object
    MSOURCE(_reset)(q);
    return q;
}

// create msource object with default parameters
MSOURCE() MSOURCE(_create_default)(void)
{
    return MSOURCE(_create)(1200, 4, 60);
}

// copy object
MSOURCE() MSOURCE(_copy)(MSOURCE() q_orig)
{
    // validate input
    if (q_orig == NULL)
        return liquid_error_config("msource%s_copy(), object cannot be NULL", EXTENSION);

    // create filter object and copy base parameters
    MSOURCE() q_copy = (MSOURCE()) malloc(sizeof(struct MSOURCE(_s)));
    memmove(q_copy, q_orig, sizeof(struct MSOURCE(_s)));

    // copy signal sources
    q_copy->sources = (QSOURCE()*) malloc(q_orig->num_sources*sizeof(QSOURCE()));
    unsigned int i;
    for (i=0; i<q_orig->num_sources; i++)
        q_copy->sources[i] = QSOURCE(_copy)(q_orig->sources[i]);

    // copy synthesis channelizer
    q_copy->ch = firpfbch2_crcf_copy(q_orig->ch);

    // copy buffers
    q_copy->buf_freq = (float complex *) liquid_malloc_copy(q_orig->buf_freq, q_orig->M,   sizeof(float complex));
    q_copy->buf_time = (float complex *) liquid_malloc_copy(q_orig->buf_time, q_orig->M/2, sizeof(float complex));

    return q_copy;
}

// destroy msource object, freeing all internal memory
int MSOURCE(_destroy)(MSOURCE() _q)
{
    // destroy internal objects
    unsigned int i;
    for (i=0; i<_q->num_sources; i++)
        QSOURCE(_destroy)(_q->sources[i]);

    // free list of sources
    free(_q->sources);

    // destroy channelizer
    firpfbch2_crcf_destroy(_q->ch);

    // free buffers
    free(_q->buf_freq);
    free(_q->buf_time);

    // free main object
    free(_q);
    return LIQUID_OK;
}

// reset msource internal state
int MSOURCE(_reset)(MSOURCE() _q)
{
    // reset all internal objects
    _q->read_index = _q->M/2;
    return LIQUID_OK;
}

// print
int MSOURCE(_print)(MSOURCE() _q)
{
    printf("<liquid.msource%s, M=%u, m=%u, as=%.1f dB, %llu samples>\n",
            EXTENSION, _q->M, _q->m, _q->as, _q->num_samples);
#if 0
    unsigned int i;
    for (i=0; i<_q->num_sources; i++)
        QSOURCE(_print)(_q->sources[i]);
#endif
    return LIQUID_OK;
}

// add user-defined source
int MSOURCE(_add_user)(MSOURCE()          _q,
                       float              _fc,
                       float              _bw,
                       float              _gain,
                       void *             _userdata,
                       MSOURCE(_callback) _callback)
{
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, _bw, _gain);
    QSOURCE(_init_user)(s, _userdata, (void*)_callback);
    return MSOURCE(_add_source)(_q, s);
}

// add tone source
int MSOURCE(_add_tone)(MSOURCE() _q,
                       float     _fc,
                       float     _bw,
                       float     _gain)
{
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, _bw, _gain);
    QSOURCE(_init_tone)(s);
    return MSOURCE(_add_source)(_q, s);
}

// add chirp source
int MSOURCE(_add_chirp)(MSOURCE() _q,
                        float     _fc,
                        float     _bw,
                        float     _gain,
                        float     _duration,
                        int       _negate,
                        int       _single)
{
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, _bw, _gain);
    QSOURCE(_init_chirp)(s, _duration, _negate, _single);
    return MSOURCE(_add_source)(_q, s);
}

// add noise source
int MSOURCE(_add_noise)(MSOURCE() _q,
                        float     _fc,
                        float     _bw,
                        float     _gain)
{
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, _bw, _gain);
    QSOURCE(_init_noise)(s);
    return MSOURCE(_add_source)(_q, s);
}

// add linear modem source
int MSOURCE(_add_modem)(MSOURCE()    _q,
                        float        _fc,
                        float        _bw,
                        float        _gain,
                        int          _ms,
                        unsigned int _m,
                        float        _beta)
{
    // create object with double the bandwidth to account for 2 samples/symbol
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, 2*_bw, _gain);
    QSOURCE(_init_modem)(s, _ms, _m, _beta);
    return MSOURCE(_add_source)(_q, s);
}

// add frequency-shift keying modem source
int MSOURCE(_add_fsk)(MSOURCE()    _q,
                      float        _fc,
                      float        _bw,
                      float        _gain,
                      unsigned int _m,
                      unsigned int _k)
{
    // create object with double the bandwidth to account for 2 samples/symbol
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, 2*_bw, _gain);
    QSOURCE(_init_fsk)(s, _m, _k);
    return MSOURCE(_add_source)(_q, s);
}

// add GMSK modem source
int MSOURCE(_add_gmsk)(MSOURCE()    _q,
                       float        _fc,
                       float        _bw,
                       float        _gain,
                       unsigned int _m,
                       float        _bt)
{
    // create object with double the bandwidth to account for 2 samples/symbol
    QSOURCE() s = QSOURCE(_create)(_q->M, _q->m, _q->as, _fc, 2*_bw, _gain);
    QSOURCE(_init_gmsk)(s, _m, _bt);
    return MSOURCE(_add_source)(_q, s);
}

// remove signal
int MSOURCE(_remove)(MSOURCE() _q,
                     int       _id)
{
    // find source object matching id
    unsigned int i;
    int id_found = 0;
    for (i=0; i<_q->num_sources; i++) {
        if (QSOURCE(_get_id)(_q->sources[i]) == _id) {
            id_found = 1;
            break;
        }
    }

    // check to see if id was found
    if (!id_found)
        return liquid_error(LIQUID_EIRANGE,"msource%s_remove(), signal id (%d) not found",EXTENSION,_id);

    // delete source
    QSOURCE(_destroy)(_q->sources[i]);

    //
    _q->num_sources--;

    // shift sources down
    for (; i<_q->num_sources; i++)
        _q->sources[i] = _q->sources[i+1];

    return LIQUID_OK;
}

// enable/disable signal
int MSOURCE(_enable)(MSOURCE() _q,
                     int       _id)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_enable(), could not find source with id %u",EXTENSION,_id);

    // set source gain
    return QSOURCE(_enable)(source);
}

int MSOURCE(_disable)(MSOURCE() _q,
                      int       _id)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_disable(), could not find source with id %u",EXTENSION,_id);

    // set source gain
    return QSOURCE(_disable)(source);
}

// Get number of samples generated by the object so far
unsigned long long int MSOURCE(_get_num_samples)(MSOURCE() _q)
{
    return _q->num_samples;
}

// Get number of samples generated by specific source so far
int MSOURCE(_get_num_samples_source)(MSOURCE()           _q,
                                     int                 _id,
                                     unsigned long int * _num_samples)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_get_num_samples_source(), could not find source with id %u",EXTENSION,_id);

    *_num_samples = QSOURCE(_get_num_samples)(source);
    return LIQUID_OK;
}

// set signal gain
//  _q      :   msource object
//  _id     :   source id
//  _gain_dB:   signal gain in dB
int MSOURCE(_set_gain)(MSOURCE() _q,
                       int       _id,
                       float     _gain_dB)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_set_gain(), could not find source with id %u",EXTENSION,_id);

    // set source gain
    return QSOURCE(_set_gain)(source, _gain_dB);
}

// set signal gain
//  _q      :   msource object
//  _id     :   source id
//  _gain_dB:   signal gain in dB
int MSOURCE(_get_gain)(MSOURCE() _q,
                       int       _id,
                       float *   _gain_dB)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_get_gain(), could not find source with id %u",EXTENSION,_id);

    // set source gain
    *_gain_dB = QSOURCE(_get_gain)(source);
    return LIQUID_OK;
}

// set carrier offset to signal
//  _q      :   msource object
//  _id     :   source id
//  _fc     :   carrier offset, fc in [-0.5,0.5]
int MSOURCE(_set_frequency)(MSOURCE() _q,
                            int       _id,
                            float     _dphi)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_set_frequency(), could not find source with id %u",EXTENSION,_id);

    // set source frequency
    return QSOURCE(_set_frequency)(source, _dphi);
}

// set carrier offset to signal
//  _q      :   msource object
//  _id     :   source id
//  _fc     :   carrier offset, fc in [-0.5,0.5]
int MSOURCE(_get_frequency)(MSOURCE() _q,
                            int       _id,
                            float *   _dphi)
{
    QSOURCE() source = MSOURCE(_get_source)(_q, _id);
    if (source == NULL)
        return liquid_error(LIQUID_EIRANGE,"msource%s_get_frequency(), could not find source with id %u",EXTENSION,_id);

    // set source frequency
    *_dphi = QSOURCE(_get_frequency)(source);
    return LIQUID_OK;
}

// write block of samples to output buffer
//  _q      : synchronizer object
//  _buf    : output buffer [size: _buf_len x 1]
//  _buf_len: output buffer size
int MSOURCE(_write_samples)(MSOURCE()    _q,
                             TO *         _buf,
                             unsigned int _buf_len)
{
    unsigned int i;
    for (i=0; i<_buf_len; i++) {
        // generate more samples if needed
        if (_q->read_index >= _q->M/2) {
            MSOURCE(_generate)(_q);
        }

        // write output sample and update counter
        _buf[i] = _q->buf_time[_q->read_index++];
    }
    return LIQUID_OK;
}

//
// internal msource methods
//

// find qsource object by id within list, return -1 if not found
int MSOURCE(_find)(MSOURCE() _q, int _id)
{
    unsigned int i;
    for (i=0; i<_q->num_sources; i++) {
        if (QSOURCE(_get_id)(_q->sources[i]) == _id)
            return (int)i;
    }
    return -1;
}

// get source by id
QSOURCE() MSOURCE(_get_source)(MSOURCE() _q,
                               int       _id)
{
    int index = MSOURCE(_find)(_q, _id);
    if (index < 0)
        return liquid_error_config("msource%s_get_source(), could not find source with id %u",EXTENSION,_id);
    return _q->sources[index];
}

// add source to list
int MSOURCE(_add_source)(MSOURCE() _q,
                         QSOURCE() _s)
{
    if (_s == NULL)
        return -1;

    // reallocate
    if (_q->num_sources == 0) {
        _q->sources = (QSOURCE()*) malloc(sizeof(QSOURCE()));
    } else {
        _q->sources = (QSOURCE()*) realloc(_q->sources,
                                           (_q->num_sources+1)*sizeof(QSOURCE()));
    }

    // append new object to end of list
    _q->sources[_q->num_sources] = _s;
    _q->num_sources++;

    // set id and increment internal counter
    int id = _q->id_counter;
    QSOURCE(_set_id)(_s, id);
    _q->id_counter++;

    // return id to user
    return id;
}

// generate samples internally
int MSOURCE(_generate)(MSOURCE() _q)
{
    // clear buffer
    memset(_q->buf_freq, 0, _q->M*sizeof(float complex));

    // add sources into main frequency buffer
    unsigned int i;
    for (i=0; i<_q->num_sources; i++)
        QSOURCE(_generate_into)(_q->sources[i], _q->buf_freq);

    // run synthesis channelizer
    firpfbch2_crcf_execute(_q->ch, _q->buf_freq, _q->buf_time);

    // update state
    _q->read_index = 0;
    _q->num_blocks++;
    _q->num_samples += _q->M / 2;
    return LIQUID_OK;
}

