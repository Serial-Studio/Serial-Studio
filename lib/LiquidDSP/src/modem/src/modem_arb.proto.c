/*
 * Copyright (c) 2007 - 2021 Joseph Gaeddert
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
// modem_arb.c
//

// create arbitrary digital modem object
MODEM() MODEM(_create_arbitrary)(float complex * _table,
                                 unsigned int    _M)
{
    // strip out bits/symbol
    unsigned int m = liquid_nextpow2(_M);
    if ( (1<<m) != _M )
        return liquid_error_config("modem%s_create_arbitrary(), input constellation size must be power of 2", EXTENSION);

    // create arbitrary modem object, not initialized
    MODEM() q = MODEM(_create_arb)(m);

    // initialize object from table
    MODEM(_arb_init)(q, _table, _M);

    // print table
    printf("modem arb[%u]\n", q->M);
    unsigned int i;
    for (i=0; i<q->M; i++)
        printf("  %3u: %12.8f %12.8f\n", i, crealf(q->symbol_map[i]), cimagf(q->symbol_map[i]));

    // return object
    return q;
}


// create an arbitrary modem object
MODEM() MODEM(_create_arb)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_ARB;

    MODEM(_init)(q, _bits_per_symbol);

    q->M = q->M;
    q->symbol_map = (TC*) calloc( q->M, sizeof(TC) );

    q->modulate_func   = &MODEM(_modulate_arb);
    q->demodulate_func = &MODEM(_demodulate_arb);

    return q;
}

// modulate arbitrary modem type
int MODEM(_modulate_arb)(MODEM()      _q,
                         unsigned int _sym_in,
                         TC *         _y)
{
    if (_sym_in >= _q->M)
        return liquid_error(LIQUID_EIRANGE,"modulate_arb(), input symbol exceeds maximum");

    // map sample directly to output
    *_y = _q->symbol_map[_sym_in]; 
    return LIQUID_OK;
}

// demodulate arbitrary modem type
int MODEM(_demodulate_arb)(MODEM()        _q,
                           TC             _x,
                           unsigned int * _sym_out)
{
    // search for symbol nearest to received sample
    unsigned int i;
    unsigned int s=0;
    T d;            // distance
    T d_min = 0.0f; // minimum distance

    for (i=0; i<_q->M; i++) {
        // compute distance from received symbol to constellation point
        d = cabsf(_x - _q->symbol_map[i]);

        // retain symbol with minimum distance
        if ( i==0 || d < d_min ) {
            d_min = d;
            s = i;
        }
    }

    // set output symbol
    *_sym_out = s;

    // re-modulate symbol and store state
    MODEM(_modulate_arb)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
    return LIQUID_OK;
}

// create a V.29 modem object (4 bits/symbol)
MODEM() MODEM(_create_V29)()
{
    MODEM() q = MODEM(_create_arb)(4);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb_V29,16);
#endif
    return q;
}

// create an arb16opt (optimal 16-qam) modem object
MODEM() MODEM(_create_arb16opt)()
{
    MODEM() q = MODEM(_create_arb)(4);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb16opt,16);
#endif
    return q;
}

// create an arb32opt (optimal 32-qam) modem object
MODEM() MODEM(_create_arb32opt)()
{
    MODEM() q = MODEM(_create_arb)(5);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb32opt,32);
#endif
    return q;
}

// create an arb64opt (optimal 64-qam) modem object
MODEM() MODEM(_create_arb64opt)()
{
    MODEM() q = MODEM(_create_arb)(6);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb64opt,64);
#endif
    return q;
}

// create an arb128opt (optimal 128-qam) modem object
MODEM() MODEM(_create_arb128opt)()
{
    MODEM() q = MODEM(_create_arb)(7);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb128opt,128);
#endif
    return q;
}

// create an arb256opt (optimal 256-qam) modem object
MODEM() MODEM(_create_arb256opt)()
{
    MODEM() q = MODEM(_create_arb)(8);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb256opt,256);
#endif
    return q;
}

// create an arb64vt (64-qam vt logo) modem object
MODEM() MODEM(_create_arb64vt)()
{
    MODEM() q = MODEM(_create_arb)(6);
#if T == float
    MODEM(_arb_init)(q,(TC*)modem_arb_vt64,64);
#endif
    return q;
}

// initialize an arbitrary modem object
//  _mod        :   modem object
//  _symbol_map :   arbitrary modem symbol map
//  _len        :   number of symbols in the map
int MODEM(_arb_init)(MODEM()         _q,
                     float complex * _symbol_map,
                     unsigned int    _len)
{
#ifdef LIQUID_VALIDATE_INPUT
    if (_q->scheme != LIQUID_MODEM_ARB)
        return liquid_error(LIQUID_EICONFIG,"modem%s_arb_init(), modem is not of arbitrary type", EXTENSION);
    if (_len != _q->M)
        return liquid_error(LIQUID_EICONFIG,"modem%s_arb_init(), array sizes do not match", EXTENSION);
#endif

    unsigned int i;
    for (i=0; i<_len; i++)
        _q->symbol_map[i] = _symbol_map[i];

    // balance I/Q channels
    if (_q->scheme == LIQUID_MODEM_ARB)
        MODEM(_arb_balance_iq)(_q);

    // scale modem to have unity energy
    return MODEM(_arb_scale)(_q);
}

// initialize an arbitrary modem object on a file
//  _mod        :   modem object
//  _filename   :   name of the data file
int MODEM(_arb_init_file)(MODEM() _q,
                          char *  _filename)
{
    // try to open file
    FILE * fid = fopen(_filename, "r");
    if (fid == NULL)
        return liquid_error(LIQUID_EIO,"modem%s_arb_init_file(), could not open file", EXTENSION);

    unsigned int i, results;
    T sym_i, sym_q;
    for (i=0; i<_q->M; i++) {
        if ( feof(fid) )
            return liquid_error(LIQUID_EIO,"modem%s_arb_init_file(), premature EOF for '%s'",
                    EXTENSION,_filename);

        results = fscanf(fid, "%f %f\n", &sym_i, &sym_q);
        _q->symbol_map[i] = sym_i + _Complex_I*sym_q;

        // ensure proper number of symbols were read
        if (results < 2)
            return liquid_error(LIQUID_EIO,"modem%s_arb_init_file(), unable to parse line", EXTENSION);
    }
    fclose(fid);

    // balance I/Q channels
    if (_q->scheme == LIQUID_MODEM_ARB)
        MODEM(_arb_balance_iq)(_q);

    // scale modem to have unity energy
    return MODEM(_arb_scale)(_q);
}

// scale arbitrary modem constellation points
int MODEM(_arb_scale)(MODEM() _q)
{
    unsigned int i;

    // calculate energy
    T mag, e = 0.0f;
    for (i=0; i<_q->M; i++) {
        mag = cabsf(_q->symbol_map[i]);
        e += mag*mag;
    }

    e = sqrtf( e / _q->M );

    for (i=0; i<_q->M; i++) {
        _q->symbol_map[i] /= e;
    }
    return LIQUID_OK;
}

// balance an arbitrary modem's I/Q points
int MODEM(_arb_balance_iq)(MODEM() _q)
{
    TC mean=0.0f;
    unsigned int i;

    // accumulate average signal
    for (i=0; i<_q->M; i++) {
        mean += _q->symbol_map[i];
    }
    mean /= (T) (_q->M);

    // subtract mean value from reference levels
    for (i=0; i<_q->M; i++) {
        _q->symbol_map[i] -= mean;
    }
    return LIQUID_OK;
}

// demodulate arbitrary modem type (soft)
int MODEM(_demodulate_soft_arb)(MODEM()         _q,
                               TC              _r,
                               unsigned int  * _s,
                               unsigned char * _soft_bits)
{
    unsigned int bps = _q->m;
    unsigned int M   = _q->M;

    // gamma = 1/(2*sigma^2), approximate for constellation size
    T gamma = 1.2f*_q->M;

    unsigned int s=0;       // hard decision output
    unsigned int k;         // bit index
    unsigned int i;         // symbol index
    T d;                // distance for this symbol
    TC x_hat;    // re-modulated symbol

    T dmin_0[bps];
    T dmin_1[bps];
    for (k=0; k<bps; k++) {
        dmin_0[k] = 4.0f;
        dmin_1[k] = 4.0f;
    }
    T dmin = 0.0f;

    for (i=0; i<M; i++) {
        // compute distance from received symbol
        x_hat = _q->symbol_map[i];
        d = crealf( (_r-x_hat)*conjf(_r-x_hat) );

        // set hard-decision...
        if (d < dmin || i==0) {
            s = i;
            dmin = d;
        }

        for (k=0; k<bps; k++) {
            // strip bit
            if ( (s >> (bps-k-1)) & 0x01 ) {
                if (d < dmin_1[k]) dmin_1[k] = d;
            } else {
                if (d < dmin_0[k]) dmin_0[k] = d;
            }
        }
    }

    // make assignments
    for (k=0; k<bps; k++) {
        int soft_bit = ((dmin_0[k] - dmin_1[k])*gamma)*16 + 127;
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit = 0;
        _soft_bits[k] = (unsigned char)soft_bit;
    }

    // hard decision

    // set hard output symbol
    *_s = s;

    // re-modulate symbol and store state
    MODEM(_modulate_arb)(_q, *_s, &_q->x_hat);
    _q->r = _r;
    return LIQUID_OK;
}

