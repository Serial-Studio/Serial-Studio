//
// Golay(24,12) product code test
//
// rate 1/4 product code:
// input    : 12 x 12 bits = 144 bits = 18 bytes
// output   : 24 x 24 bits = 576 bits = 72 bytes
//

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "liquid.internal.h"

#define DEBUG (0)

typedef struct g2412p_s * g2412p;
g2412p g2412p_create  ();
void   g2412p_destroy (g2412p _q);
void   g2412p_print   (g2412p _q);
void   g2412p_encode  (g2412p _q, unsigned char * _msg_org, unsigned char * _msg_enc);
void   g2412p_decode  (g2412p _q, unsigned char * _msg_rec, unsigned char * _msg_dec);

int    g2412p_iterate (g2412p _q);
int    g2412p_step    (g2412p _q);

void   g2412p_load_row(g2412p _q, unsigned int _row);
void   g2412p_save_row(g2412p _q, unsigned int _row);
void   g2412p_load_col(g2412p _q, unsigned int _col);
void   g2412p_save_col(g2412p _q, unsigned int _col);

// pack 24-value soft-bit message into integer
unsigned int g2412p_pack_symbol(unsigned char * _msg_soft);

// unpack 24-bit integer into soft array
void g2412p_unpack_symbol(unsigned int    _msg_hard,
                          unsigned char * _msg_soft);

unsigned int liquid_hard_decision(unsigned char _soft_bit);

void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    printf("    ");
    for (i=0; i<_n; i++)
        printf("%1u ", (_x >> (_n-i-1)) & 1);
    printf("\n");
}

int main(int argc, char*argv[])
{
    unsigned int i;
    unsigned int j;

    unsigned char msg_org[144];
    unsigned char msg_enc[576];
    unsigned char msg_rec[576];
    unsigned char msg_dec[144];

    // create object to handle encoding/decoding
    g2412p q = g2412p_create();

    // initialize input array
    unsigned char p = 0x01;
    for (i=0; i<144; i++) {
        //msg_org[i] = i % 2;

        msg_org[i] = p & 1;
        p = ((p << 1) & 0xfe) | liquid_bdotprod(p,0xad);
        
        msg_org[i] = 0;
    }

#if DEBUG
    // print original message
    printf("msg_org:\n");
    for (i=0; i<12; i++) {
        printf("%3u: ", i);
        for (j=0; j<12; j++)
            printf("%2u", msg_org[12*i+j]);
        printf("\n");
    }
#endif

    // encode message
    g2412p_encode(q, msg_org, msg_enc);

#if DEBUG
    // print encoded message
    printf("msg_enc:\n");
    for (i=0; i<24; i++) {
        printf("%3u: ", i);
        for (j=0; j<24; j++)
            printf("%2u", msg_enc[24*i+j]);
        printf("\n");
    }
#endif

#if 0
    // corrupt message
    for (i=0; i<576; i++)
        msg_rec[i] = msg_enc[i] ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;
    msg_rec[  0] = 255 - msg_rec[  0];
    msg_rec[ 24] = 255 - msg_rec[ 24];
    msg_rec[ 48] = 255 - msg_rec[ 48];
    msg_rec[ 72] = 255 - msg_rec[ 72];
    msg_rec[ 96] = 255 - msg_rec[ 96];
    msg_rec[120] = 255 - msg_rec[120];
#else
    // modulate, add noise, demodulate soft
    float SNRdB = -4.0f;
    float nstd  = powf(10.0f, -SNRdB/20.0f);
    modemcf mod = modemcf_create(LIQUID_MODEM_BPSK);
    for (i=0; i<576; i++) {
        float complex v;
        unsigned int  s;
        modemcf_modulate(mod, msg_enc[i], &v);
        v += nstd*(randnf() + randnf()*_Complex_I) * M_SQRT1_2;
        modemcf_demodulate_soft(mod, v, &s, &msg_rec[i]);
    }
    modemcf_destroy(mod);
#endif

    // decode message
    g2412p_decode(q, msg_rec, msg_dec);

    // print decoded message
    printf("msg_org:\n");
    for (i=0; i<12; i++) {
        printf("%3u: ", i);
        for (j=0; j<12; j++)
            printf("%2u", msg_dec[12*i+j]);
        printf("\n");
    }

    // count errors
    unsigned int num_errors = 0;
    for (i=0; i<144; i++)
        num_errors += (msg_dec[i] & 1) == (msg_org[i] & 1) ? 0 : 1;
    printf("bit errors: %u / %u\n", num_errors, 144);
    
    // clean up allocated objects
    g2412p_destroy(q);

    printf("done\n");
    return 0;
}

struct g2412p_s {
    unsigned char msg_buf[576]; // 24 x 24 message array
    unsigned char reg0[24];     // 24 value register
    unsigned int  r;
    unsigned int  d;
};

g2412p g2412p_create()
{
    g2412p q = (g2412p) malloc( sizeof(struct g2412p_s) );

    return q;
}

void g2412p_destroy(g2412p _q)
{
    // free main object memory
    free(_q);
}

void g2412p_print(g2412p _q)
{
    unsigned int i;
#if 1
    unsigned int j;
    printf("msg_buf:\n");
    for (i=0; i<24; i++) {
        if (i==12)
            printf("------\n");
        printf("%2u:", i);
        for (j=0; j<24; j++)
            printf(" %s%3u", j==12 ? ":" : "", _q->msg_buf[24*i+j]);
        printf("\n");
    }
#else
    for (i=0; i<576; i++)
        printf(" %3u%s", _q->msg_buf[i], ((i+1)%24)==0 ? "\n" : "");
#endif
}

void g2412p_encode(g2412p          _q,
                   unsigned char * _msg_org,
                   unsigned char * _msg_enc)
{
    unsigned int r;
    unsigned int c;

    // encode rows
#if DEBUG
    printf("rows:\n");
#endif
    for (r=0; r<12; r++) {
        unsigned int sym_org = 0;
        for (c=0; c<12; c++) {
            sym_org <<= 1;
            sym_org |= _msg_org[12*r+c] ? 1 : 0;
        }
        unsigned int sym_enc = fec_golay2412_encode_symbol(sym_org);
        // make systematic (swap upper and lower 12 bit chunks)
        sym_enc = ((sym_enc >> 12) & 0xfff) | ((sym_enc << 12) & 0xfff000);
#if DEBUG
        printf(" %2u: 0x%.3x > 0x%.6x\n", r, sym_org, sym_enc);
#endif
        
        //
        for (c=0; c<24; c++) {
            unsigned int bit = (sym_enc >> (24-c-1)) & 1;
            _msg_enc[24*r+c] = bit;
        }
    }

    // encode columns
#if DEBUG
    printf("cols:\n");
#endif
    for (c=0; c<24; c++) {
        unsigned int sym_org = 0;
        for (r=0; r<12; r++) {
            sym_org <<= 1;
            sym_org |= _msg_enc[c+24*r] ? 1 : 0;
        }
        unsigned int sym_enc = fec_golay2412_encode_symbol(sym_org);
        // make systematic (swap upper and lower 12 bit chunks)
        sym_enc = ((sym_enc >> 12) & 0xfff) | ((sym_enc << 12) & 0xfff000);
#if DEBUG
        printf(" %2u: 0x%.3x > 0x%.6x\n", c, sym_org, sym_enc);
#endif
        
        // append only parity bits (no need to overwrite original bits)
        for (r=12; r<24; r++) {
            unsigned int bit = (sym_enc >> (24-r-1)) & 1;
            _msg_enc[c+24*r] = bit;
        }
    }

}

void g2412p_decode(g2412p          _q,
                   unsigned char * _msg_rec,
                   unsigned char * _msg_dec)
{
    unsigned int i;

    // copy received message to internal buffer
    memmove(_q->msg_buf, _msg_rec, 576*sizeof(unsigned char));

    // print
#if DEBUG
    g2412p_print(_q);
#endif

#if 1
    // iterate
    // while...
    for (i=0; i<30; i++) {
        int rc = g2412p_iterate(_q);

        if (rc == 0)
            break;
    }
    printf("steps: %u\n", i);
#endif
    
    // print
#if DEBUG
    g2412p_print(_q);
#endif

    // copy resulting output
#if 0
    for (i=0; i<12; i++)
        memmove(&_msg_dec[24*i], &_q->msg_buf[12*i], 12*sizeof(unsigned char));
#else
    unsigned int r;
    unsigned int c;
    unsigned int n = 0;
    for (r=0; r<12; r++) {
        for (c=0; c<12; c++) {
            _msg_dec[n++] = _q->msg_buf[24*r+c] > LIQUID_SOFTBIT_ERASURE ? 1 : 0;
        }
    }
#endif
}

// run single iteration
int g2412p_iterate(g2412p _q)
{
    unsigned int i;

    int rc = 0;

    // decode columns
    for (i=0; i<24; i++) {
        g2412p_load_col     (_q, i);
        rc += g2412p_step   (_q   );
        g2412p_save_col     (_q, i);
    }

    // decode rows
    for (i=0; i<12; i++) {
        g2412p_load_row     (_q, i);
        rc += g2412p_step   (_q   );
        g2412p_save_row     (_q, i);
    }

    return rc;
}

// 
int g2412p_step(g2412p _q)
{
    // TODO: determine if parity check passed

    // run hard-decision decoding
    unsigned int sym_rec = 0;
    unsigned int i;
    for (i=0; i<24; i++) {
        sym_rec <<= 1;
        sym_rec |= _q->reg0[i] > 127 ? 1 : 0;
    }

    // flip...
    //printf("  0x%.6x > ", sym_rec);
    sym_rec = ((sym_rec >> 12) & 0xfff) | ((sym_rec << 12) & 0xfff000);

    // decode to 12-bit symbol
    unsigned int sym_dec = fec_golay2412_decode_symbol(sym_rec);

    // re-encode 24-bit symbol
    unsigned int sym_enc = fec_golay2412_encode_symbol(sym_dec);

    int rc = (sym_rec == sym_enc) ? 0 : 1;

    // flip...
    sym_enc = ((sym_enc >> 12) & 0xfff) | ((sym_enc << 12) & 0xfff000);
    //printf("0x%.6x\n", sym_enc);

    // update register
    // TODO: this is the crux of the algorithm and should be adjusted to
    //       produce maximum performance.
    float alpha = 0.9f;         // percentage of old value to retain
    float beta  = 1.0f - alpha; // percentage of new value to retain
    for (i=0; i<24; i++) {
        //int v = (int)(_q->reg0[i]) + ((sym_enc >> i) & 1 ? 16 : -16);
        int p = (sym_enc >> (24-i-1)) & 1 ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;
        int v = ((int)(alpha*_q->reg0[i]) + beta*p);

        if      (v <   0) _q->reg0[i] = 0;
        else if (v > 255) _q->reg0[i] = 255;
        else              _q->reg0[i] = (unsigned char)v;
    }

    return rc;
}

// load 24-bit row into register
void g2412p_load_row(g2412p       _q,
                     unsigned int _row)
{
    //memmove(_q->reg0, &_q->msg_buf[_row*24], 24*sizeof(unsigned char));
    unsigned int c;
    for (c=0; c<24; c++)
        _q->reg0[c] = _q->msg_buf[24*_row + c];
}

void g2412p_save_row(g2412p       _q,
                     unsigned int _row)
{
    //memmove(&_q->msg_buf[_row*24], _q->reg0, 24*sizeof(unsigned char));
    unsigned int c;
    for (c=0; c<24; c++)
        _q->msg_buf[24*_row + c] = _q->reg0[c];
}

void g2412p_load_col(g2412p       _q,
                     unsigned int _col)
{
    unsigned int r;
    for (r=0; r<24; r++)
        _q->reg0[r] = _q->msg_buf[24*r + _col];
}

void g2412p_save_col(g2412p       _q,
                     unsigned int _col)
{
    unsigned int r;
    for (r=0; r<24; r++)
        _q->msg_buf[24*r + _col] = _q->reg0[r];
}

// pack 24-value soft-bit message into integer
unsigned int g2412p_pack_symbol(unsigned char * _msg_soft)
{
    unsigned int i;
    unsigned int sym = 0;
    for (i=0; i<24; i++) {
        sym <<= 1;
        sym |= liquid_hard_decision(_msg_soft[i]);
    }

    return sym;
}

// unpack 24-bit integer into soft array
void g2412p_unpack_symbol(unsigned int    _msg_hard,
                          unsigned char * _msg_soft)
{
    unsigned int i;
    for (i=0; i<24; i++) {
        _msg_soft[i] = _msg_hard & 1 ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;
        _msg_hard >>= 1;
    }
}

unsigned int liquid_hard_decision(unsigned char _soft_bit)
{
    return _soft_bit > LIQUID_SOFTBIT_ERASURE ? LIQUID_SOFTBIT_1 : LIQUID_SOFTBIT_0;
}
