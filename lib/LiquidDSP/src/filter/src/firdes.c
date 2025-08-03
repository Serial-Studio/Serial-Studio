/*
 * Copyright (c) 2007 - 2023 Joseph Gaeddert
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
// Finite impulse response filter design
//
// References:
//  [Herrmann:1973] O. Herrmann, L. R. Rabiner, and D. S. K. Chan,
//      "Practical design rules for optimum finite impulse response
//      lowpass digital filters," Bell Syst. Tech. Journal, vol. 52,
//      pp. 769--99, July-Aug. 1973
//  [Vaidyanathan:1993] Vaidyanathan, P. P., "Multirate Systems and
//      Filter Banks," 1993, Prentice Hall, Section 3.2.1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

// method to estimate required filter length
#define ESTIMATE_REQ_FILTER_LEN_METHOD_KAISER   (0)
#define ESTIMATE_REQ_FILTER_LEN_METHOD_HERRMANN (1)

// select filter estimate method
#define ESTIMATE_REQ_FILTER_LEN_METHOD          (0)

const char * liquid_firfilt_type_str[LIQUID_FIRFILT_NUM_TYPES][2] = {
    // short,    long name
    {"unknown",  "unknown"},

    // Nyquist filter prototypes
    {"kaiser",   "Nyquist Kaiser filter"},
    {"pm",       "Parks-McClellan filter"},
    {"rcos",     "raised-cosine filter"},
    {"fexp",     "flipped exponential"},
    {"fsech",    "flipped hyperbolic secant"},
    {"farcsech", "flipped arc-hyperbolic secant"},

    // root-Nyquist filter prototypes
    {"arkaiser", "root-Nyquist Kaiser (approximate optimum)"},
    {"rkaiser",  "root-Nyquist Kaiser (true optimum)"},
    {"rrcos",    "root raised-cosine"},
    {"hm3",      "harris-Moerder-3 filter"},
    {"gmsktx",   "GMSK transmit filter"},
    {"gmskrx",   "GMSK receive filter"},
    {"rfexp",    "root flipped exponential"},
    {"rfsech",   "root flipped hyperbolic secant"},
    {"rfarcsech","root flipped arc-hyperbolic secant"},
};

// estimate required filter length given transition bandwidth and
// stop-band attenuation
//  _df     :   transition bandwidth (0 < _df < 0.5)
//  _as     :   stopband suppression level [dB] (_as > 0)
unsigned int estimate_req_filter_len(float _df,
                                     float _as)
{
    if (_df > 0.5f || _df <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len(), invalid bandwidth : %f", _df);
        return 0;
    } else if (_as <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len(), invalid stopband level : %f", _as);
        return 0;
    }

    // compute filter length estimate
#if ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_KAISER
    // use Kaiser's estimate
    unsigned int h_len = (unsigned int) estimate_req_filter_len_Kaiser(_df,_as);
#elif ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_HERRMANN
    // use Herrmann's estimate
    unsigned int h_len = (unsigned int) estimate_req_filter_len_Herrmann(_df,_as);
#else
#   error "invalid required filter length estimation method"
#endif
    
    return h_len;
}

// estimate filter stop-band attenuation given
//  _df     :   transition bandwidth (0 < _b < 0.5)
//  _n      :   filter length
float estimate_req_filter_As(float        _df,
                             unsigned int _n)
{
    // run search for stop-band attenuation which gives these results
    float as0   = 0.01f;    // lower bound
    float as1   = 200.0f;   // upper bound

    float as_hat = 0.0f;    // stop-band attenuation estimate
    float N_hat = 0.0f;     // filter length estimate

    // perform simple bisection search
    unsigned int num_iterations = 20;
    unsigned int i;
    for (i=0; i<num_iterations; i++) {
        // bisect limits
        as_hat = 0.5f*(as1 + as0);
#if ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_KAISER
        N_hat = estimate_req_filter_len_Kaiser(_df, as_hat);
#elif ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_HERRMANN
        N_hat = estimate_req_filter_len_Herrmann(_df, as_hat);
#else
#       error "invalid required filter length estimation method"
#endif

        //printf("range[%8.2f, %8.2f] as-hat=%8.2fdB, N=%8.2f (target: %3u taps)\n",
        //        as0, as1, as_hat, N_hat, _n);

        // update limits
        if (N_hat < (float)_n) {
            as0 = as_hat;
        } else {
            as1 = as_hat;
        }
    }
    return as_hat;
}

// estimate filter transition bandwidth given
//  _as     :   stop-band attenuation [dB], _as > 0
//  _n      :   filter length
float estimate_req_filter_df(float        _as,
                             unsigned int _n)
{
    // run search for stop-band attenuation which gives these results
    float df0   = 1e-3f;    // lower bound
    float df1   = 0.499f;   // upper bound

    float df_hat = 0.0f;    // stop-band attenuation estimate
    float N_hat = 0.0f;     // filter length estimate

    // perform simple bisection search
    unsigned int num_iterations = 20;
    unsigned int i;
    for (i=0; i<num_iterations; i++) {
        // bisect limits
        df_hat = 0.5f*(df1 + df0);
#if ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_KAISER
        N_hat = estimate_req_filter_len_Kaiser(df_hat, _as);
#elif ESTIMATE_REQ_FILTER_LEN_METHOD == ESTIMATE_REQ_FILTER_LEN_METHOD_HERRMANN
        N_hat = estimate_req_filter_len_Herrmann(df_hat, _as);
#else
#       error "invalid required filter length estimation method"
#endif

        //printf("range[%8.5f, %8.5f] df-hat=%8.5fdB, N=%8.2f (target: %3u taps)\n",
        //        df0, df1, df_hat, N_hat, _n);

        // update limits
        if (N_hat < (float)_n) {
            df1 = df_hat;
        } else {
            df0 = df_hat;
        }
    }
    return df_hat;

}


// estimate required filter length given transition bandwidth and
// stop-band attenuation (algorithm from [Vaidyanathan:1993])
//  _df     :   transition bandwidth (0 < _df < 0.5)
//  _as     :   stop-band attenuation [dB] (as > 0)
float estimate_req_filter_len_Kaiser(float _df,
                                     float _as)
{
    if (_df > 0.5f || _df <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len_Kaiser(), invalid bandwidth : %f", _df);
        return 0.0f;
    } else if (_as <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len(), invalid stopband level : %f", _as);
        return 0.0f;
    }

    // compute filter length estimate
    return (_as - 7.95f)/(14.26f*_df);
}


// estimate required filter length given transition bandwidth and
// stop-band attenuation (algorithm from [Herrmann:1973])
//  _df     :   transition bandwidth (0 < _df < 0.5)
//  _as     :   stop-band attenuation [dB] (as > 0)
float estimate_req_filter_len_Herrmann(float _df,
                                       float _as)
{
    if (_df > 0.5f || _df <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len_Herrmann(), invalid bandwidth : %f", _df);
        return 0.0f;
    } else if (_as <= 0.0f) {
        liquid_error(LIQUID_EICONFIG,"estimate_req_filter_len(), invalid stopband level : %f", _as);
        return 0.0f;
    }

    // Gaeddert's revisions:
    if (_as > 105.0f)
        return estimate_req_filter_len_Kaiser(_df,_as);

    _as += 7.4f;

    // compute delta_1, delta_2
    float d1, d2;
    d1 = d2 = powf(10.0, -_as/20.0);

    // compute log of delta_1, delta_2
    float t1 = log10f(d1);
    float t2 = log10f(d2);

    // compute D_infinity(delta_1, delta_2)
    float Dinf = (0.005309f*t1*t1 + 0.07114f*t1 - 0.4761f)*t2 -
                 (0.002660f*t1*t1 + 0.59410f*t1 + 0.4278f);

    // compute f(delta_1, delta_2)
    float f = 11.012f + 0.51244f*(t1-t2);

    // compute filter length estimate
    float h_len = (Dinf - f*_df*_df) / _df + 1.0f;
    
    return h_len;
}

// returns the Kaiser window beta factor give the filter's target
// stop-band attenuation (as) [Vaidyanathan:1993]
//  _as     :   target filter's stop-band attenuation [dB], _as > 0
float kaiser_beta_As(float _as)
{
    _as = fabsf(_as);
    float beta;
    if (_as > 50.0f)
        beta = 0.1102f*(_as - 8.7f);
    else if (_as > 21.0f)
        beta = 0.5842*powf(_as - 21, 0.4f) + 0.07886f*(_as - 21);
    else
        beta = 0.0f;

    return beta;
}

// Design FIR filter using generic window/taper method
//  _wtype  : window type, e.g. LIQUID_WINDOW_HAMMING
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _arg    : window-specific argument, if required
//  _h      : output coefficient buffer, [size: _n x 1]
int liquid_firdes_windowf(int          _wtype,
                          unsigned int _n,
                          float        _fc,
                          float        _arg,
                          float *      _h)
{
    // validate inputs
    if (_fc <= 0.0f || _fc > 0.5f) {
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_window(), cutoff frequency (%12.4e) out of range (0, 0.5)", _fc);
    } else if (_n == 0) {
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_window(), filter length must be greater than zero");
    }

    float t, h1, h2; 
    unsigned int i;
    for (i=0; i<_n; i++) {
        // time vector
        t = (float)i - (float)(_n-1)/2.0f;

        // sinc prototype
        h1 = sincf(2.0f*_fc*t);

        // window
        h2 = liquid_windowf(_wtype,i,_n,_arg);

        // composite
        _h[i] = h1*h2;
    }
    return LIQUID_OK;
}

// Design FIR using kaiser window
//  _n      : filter length, _n > 0
//  _fc     : cutoff frequency, 0 < _fc < 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _mu     : fractional sample offset, -0.5 < _mu < 0.5
//  _h      : output coefficient buffer, [size: _n x 1]
int liquid_firdes_kaiser(unsigned int _n,
                         float _fc,
                         float _as,
                         float _mu,
                         float *_h)
{
    // validate inputs
    if (_mu < -0.5f || _mu > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_kaiser(), _mu (%12.4e) out of range [-0.5,0.5]", _mu);
    if (_fc <= 0.0f || _fc > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_kaiser(), cutoff frequency (%12.4e) out of range (0, 0.5)", _fc);
    if (_n == 0)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_kaiser(), filter length must be greater than zero");

    // choose kaiser beta parameter (approximate)
    float beta = kaiser_beta_As(_as);

    // TODO: invoke liquid_firdes_windowf()
    float t, h1, h2; 
    unsigned int i;
    for (i=0; i<_n; i++) {
        t = (float)i - (float)(_n-1)/2 + _mu;
     
        // sinc prototype
        h1 = sincf(2.0f*_fc*t);

        // kaiser window
        h2 = liquid_kaiser(i,_n,beta);

        //printf("t = %f, h1 = %f, h2 = %f\n", t, h1, h2);

        // composite
        _h[i] = h1*h2;
    }
    return LIQUID_OK;
}

// Design finite impulse response notch filter
//  _m      : filter semi-length, m in [1,1000]
//  _f0     : filter notch frequency (normalized), -0.5 <= _fc <= 0.5
//  _as     : stop-band attenuation [dB], _as > 0
//  _h      : output coefficient buffer, [size: 2*_m+1 x 1]
int liquid_firdes_notch(unsigned int _m,
                        float        _f0,
                        float        _as,
                        float *      _h)
{
    // validate inputs
    if (_m < 1 || _m > 1000)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_notch(), _m (%12u) out of range [1,1000]", _m);
    if (_f0 < -0.5f || _f0 > 0.5f)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_notch(), notch frequency (%12.4e) must be in [-0.5,0.5]", _f0);
    if (_as <= 0.0f)
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_notch(), stop-band suppression (%12.4e) must be greater than zero", _as);

    // choose kaiser beta parameter (approximate)
    float beta = kaiser_beta_As(_as);

    // design filter
    unsigned int h_len = 2*_m+1;
    unsigned int i;
    float scale = 0.0f;
    for (i=0; i<h_len; i++) {
        // tone at carrier frequency
        float p = -cosf(2.0f*M_PI*_f0*((float)(i) - (float)_m));

        // window
        float w = liquid_kaiser(i,h_len,beta);

        // save un-normalized filter
        _h[i] = p*w;

        // accumulate scale
        scale += _h[i] * p;
    }

    // normalize
    for (i=0; i<h_len; i++)
        _h[i] /= scale;

    // add impulse and return
    _h[_m] += 1.0f;
    return LIQUID_OK;
}

// Design (root-)Nyquist filter from prototype
//  _type   : filter type (e.g. LIQUID_FIRFILT_RRC)
//  _k      : samples/symbol
//  _m      : symbol delay
//  _beta   : excess bandwidth factor, _beta in [0,1]
//  _dt     : fractional sample delay
//  _h      : output coefficient buffer (length: 2*k*m+1)
int liquid_firdes_prototype(liquid_firfilt_type _type,
                            unsigned int        _k,
                            unsigned int        _m,
                            float               _beta,
                            float               _dt,
                            float *             _h)
{
    // compute filter parameters
    unsigned int h_len = 2*_k*_m + 1;   // length
    float fc = 0.5f / (float)_k;        // cut-off frequency
    float df = _beta / (float)_k;       // transition bandwidth
    float as = estimate_req_filter_As(df,h_len);   // stop-band attenuation

    // Parks-McClellan algorithm parameters
    float bands[6] = {  0.0f,       fc-0.5f*df,
                        fc,         fc,
                        fc+0.5f*df, 0.5f};
    float des[3] = { (float)_k, 0.5f*_k, 0.0f };
    float weights[3] = {1.0f, 1.0f, 1.0f};
    liquid_firdespm_wtype wtype[3] = {  LIQUID_FIRDESPM_FLATWEIGHT,
                                        LIQUID_FIRDESPM_FLATWEIGHT,
                                        LIQUID_FIRDESPM_FLATWEIGHT};

    switch (_type) {
    // Nyquist filter prototypes
    case LIQUID_FIRFILT_KAISER:     return liquid_firdes_kaiser   (h_len, fc, as, _dt, _h);
    case LIQUID_FIRFILT_PM:
        // NOTE: input timing offset is ignored here
        return firdespm_run(h_len, 3, bands, des, weights, wtype, LIQUID_FIRDESPM_BANDPASS, _h);
    case LIQUID_FIRFILT_RCOS:       return liquid_firdes_rcos     (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_FEXP:       return liquid_firdes_fexp     (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_FSECH:      return liquid_firdes_fsech    (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_FARCSECH:   return liquid_firdes_farcsech (_k, _m, _beta, _dt, _h);

    // root-Nyquist filter prototypes
    case LIQUID_FIRFILT_ARKAISER:   return liquid_firdes_arkaiser (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_RKAISER:    return liquid_firdes_rkaiser  (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_RRC:        return liquid_firdes_rrcos    (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_hM3:        return liquid_firdes_hM3      (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_GMSKTX:     return liquid_firdes_gmsktx   (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_GMSKRX:     return liquid_firdes_gmskrx   (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_RFEXP:      return liquid_firdes_rfexp    (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_RFSECH:     return liquid_firdes_rfsech   (_k, _m, _beta, _dt, _h);
    case LIQUID_FIRFILT_RFARCSECH:  return liquid_firdes_rfarcsech(_k, _m, _beta, _dt, _h);
    default:
        return liquid_error(LIQUID_EICONFIG,"liquid_firdes_prototype(), filter type '%d'", _type);
    }
    return LIQUID_OK;
}


// Design FIR doppler filter
//  _n      : filter length
//  _fd     : normalized doppler frequency (0 < _fd < 0.5)
//  _K      : Rice fading factor (K >= 0)
//  _theta  : LoS component angle of arrival
//  _h      : output coefficient buffer
int liquid_firdes_doppler(unsigned int _n,
                          float        _fd,
                          float        _K,
                          float        _theta,
                          float *      _h)
{
    // TODO: add error checking
    float t, J, r, w;
    float beta = 4; // kaiser window parameter
    unsigned int i;
    for (i=0; i<_n; i++) {
        // time sample
        t = (float)i - (float)(_n-1)/2;

        // Bessel
        J = 1.5*liquid_besselj0f(fabsf((float)(2*M_PI*_fd*t)));

        // Rice-K component
        r = 1.5*_K/(_K+1)*cosf(2*M_PI*_fd*t*cosf(_theta));

        // Window
        w = liquid_kaiser(i, _n, beta);

        // composite
        _h[i] = (J+r)*w;

        //printf("t=%f, J=%f, r=%f, w=%f\n", t, J, r, w);
    }
    return LIQUID_OK;
}


// 
// filter analysis
//

// liquid_filter_autocorr()
//
// Compute auto-correlation of filter at a specific lag.
//
//  _h      :   filter coefficients [size: _h_len x 1]
//  _h_len  :   filter length
//  _lag    :   auto-correlation lag (samples)
float liquid_filter_autocorr(float *      _h,
                             unsigned int _h_len,
                             int          _lag)
{
    // auto-correlation is even symmetric
    _lag = abs(_lag);

    // lag outside of filter length is zero
    if (_lag >= _h_len) return 0.0f;

    // compute auto-correlation
    float rxx=0.0f; // initialize auto-correlation to zero
    unsigned int i;
    for (i=_lag; i<_h_len; i++)
        rxx += _h[i] * _h[i-_lag];

    return rxx;
}

// liquid_filter_crosscorr()
//
// Compute cross-correlation of two filters at a specific lag.
//
//  _h      :   filter coefficients [size: _h_len]
//  _h_len  :   filter length
//  _g      :   filter coefficients [size: _g_len]
//  _g_len  :   filter length
//  _lag    :   cross-correlation lag (samples)
float liquid_filter_crosscorr(float *      _h,
                              unsigned int _h_len,
                              float *      _g,
                              unsigned int _g_len,
                              int          _lag)
{
    // cross-correlation is odd symmetric
    if (_h_len < _g_len) {
        return liquid_filter_crosscorr(_g, _g_len,
                                       _h, _h_len,
                                       -_lag);
    }

    // at this point _h_len > _g_len
    // assert(_h_len > _g_len);

    if (_lag <= -(int)_g_len) return 0.0f;
    if (_lag >=  (int)_h_len) return 0.0f;

    int ig = _lag < 0 ? -_lag : 0;  // starting index for _g
    int ih = _lag > 0 ?  _lag : 0;  // starting index for _h

    // compute length of overlap
    //     condition 1:             condition 2:          condition 3:
    //    [------ h ------]     [------ h ------]     [------ h ------]
    //  [-- g --]                    [-- g --]                  [-- g --]
    //   >|  n  |<                  >|   n   |<                >|  n  |<
    //
    int n;
    if (_lag < 0)
        n = (int)_g_len + _lag;
    else if (_lag < (_h_len-_g_len))
        n = _g_len;
    else
        n = _h_len - _lag;

    // compute cross-correlation
    float rxy=0.0f; // initialize auto-correlation to zero
    int i;
    for (i=0; i< n; i++)
        rxy += _h[ih+i] * _g[ig+i];

    return rxy;
}

// liquid_filter_isi()
//
// Compute inter-symbol interference (ISI)--both RMS and
// maximum--for the filter _h.
//
//  _h      :   filter coefficients [size: 2*_k*_m+1 x 1]
//  _k      :   filter over-sampling rate (samples/symbol)
//  _m      :   filter delay (symbols)
//  _rms    :   output root mean-squared ISI
//  _max    :   maximum ISI
void liquid_filter_isi(float *      _h,
                       unsigned int _k,
                       unsigned int _m,
                       float *      _rms,
                       float *      _max)
{
    unsigned int h_len = 2*_k*_m+1;

    // compute zero-lag auto-correlation
    float rxx0 = liquid_filter_autocorr(_h,h_len,0);
    //printf("rxx0 = %12.8f\n", rxx0);

    unsigned int i;
    float isi_rms = 0.0f;
    float isi_max = 0.0f;
    float e;
    for (i=1; i<=2*_m; i++) {
        e = liquid_filter_autocorr(_h,h_len,i*_k) / rxx0;
        e = fabsf(e);

        isi_rms += e*e;
        
        if (i==1 || e > isi_max)
            isi_max = e;
    }

    *_rms = sqrtf( isi_rms / (float)(2*_m) );
    *_max = isi_max;
}

// Compute relative out-of-band energy
//
//  _h      :   filter coefficients [size: _h_len x 1]
//  _h_len  :   filter length
//  _fc     :   analysis cut-off frequency
//  _nfft   :   fft size
float liquid_filter_energy(float *      _h,
                           unsigned int _h_len,
                           float        _fc,
                           unsigned int _nfft)
{
    // validate input
    if (_fc < 0.0f || _fc > 0.5f) {
        liquid_error(LIQUID_EICONFIG,"liquid_filter_energy(), cut-off frequency must be in [0,0.5]");
        return 0.0f;
    } else if (_h_len == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_filter_energy(), filter length must be greater than zero");
        return 0.0f;
    } else if (_nfft == 0) {
        liquid_error(LIQUID_EICONFIG,"liquid_filter_energy(), fft size must be greater than zero");
        return 0.0f;
    }

    // allocate memory for complex phasor
    float complex expjwt[_h_len];

    // initialize accumulators
    float e_total = 0.0f;       // total energy
    float e_stopband = 0.0f;    // stop-band energy

    // create dotprod object
    dotprod_crcf dp = dotprod_crcf_create(_h,_h_len);

    unsigned int i;
    unsigned int k;
    for (i=0; i<_nfft; i++) {
        float f = 0.5f * (float)i / (float)(_nfft);
        
        // initialize complex phasor
        for (k=0; k<_h_len; k++)
            expjwt[k] = cexpf(_Complex_I*2*M_PI*f*k);

        // compute vector dot product
        float complex v;
        dotprod_crcf_execute(dp, expjwt, &v);

        // accumulate output
        float e2 = crealf( v*conjf(v) );
        e_total += e2;
        e_stopband += (f >= _fc) ? e2 : 0.0f;
    }

    // destroy dotprod object
    dotprod_crcf_destroy(dp);

    // return energy ratio
    return e_stopband / e_total;
}

// Get static frequency response from filter coefficients at particular
// frequency with real-valued coefficients
//  _h      : coefficients, [size: _h_len x 1]
//  _h_len  : length of coefficients array
//  _fc     : center frequency for analysis, -0.5 <= _fc <= 0.5
//  _H      : pointer to output value
int liquid_freqrespf(float *         _h,
                     unsigned int    _h_len,
                     float           _fc,
                     float complex * _H)
{
    // compute dot product between coefficients and exp{ 2 pi fc {0..n-1} }
    float complex H = 0.0f;
    unsigned int i;
    for (i=0; i<_h_len; i++)
        H += _h[i] * cexpf(-_Complex_I*2*M_PI*_fc*i);

    // set return value
    *_H = H;
    return LIQUID_OK;
}

// Get static frequency response from filter coefficients at particular
// frequency with complex coefficients
//  _h      : coefficients, [size: _h_len x 1]
//  _h_len  : length of coefficients array
//  _fc     : center frequency for analysis, -0.5 <= _fc <= 0.5
//  _H      : pointer to output value
int liquid_freqrespcf(float complex * _h,
                      unsigned int    _h_len,
                      float           _fc,
                      float complex * _H)
{
    // compute dot product between coefficients and exp{ 2 pi fc {0..n-1} }
    float complex H = 0.0f;
    unsigned int i;
    for (i=0; i<_h_len; i++)
        H += _h[i] * cexpf(-_Complex_I*2*M_PI*_fc*i);

    // set return value
    *_H = H;
    return LIQUID_OK;
}

// returns filter type based on input string
int liquid_getopt_str2firfilt(const char * _str)
{
    // compare each string to short name
    unsigned int i;
    for (i=0; i<LIQUID_FIRFILT_NUM_TYPES; i++) {
        if (strcmp(_str,liquid_firfilt_type_str[i][0])==0) {
            return i;
        }
    }

    liquid_error(LIQUID_EICONFIG,"liquid_getopt_str2firfilt(), unknown/unsupported type: %s", _str);
    return LIQUID_FIRFILT_UNKNOWN;
}

