/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "autotest/autotest.h"
#include "liquid.h"

// 
// AUTOTEST : test simple recovery of frame
//
void qpacketmodem_modulated(unsigned int _payload_len,
                            int          _check,
                            int          _fec0,
                            int          _fec1,
                            int          _ms)
{
    // derived values
    unsigned int i;

    // create and configure packet encoder/decoder object
    qpacketmodem q = qpacketmodem_create();
    qpacketmodem_configure(q, _payload_len, _check, _fec0, _fec1, _ms);
    if (liquid_autotest_verbose)
        qpacketmodem_print(q);

    // initialize payload
    unsigned char payload_tx[_payload_len];
    unsigned char payload_rx[_payload_len];

    // initialize payload
    for (i=0; i<_payload_len; i++) {
        payload_tx[i] = rand() & 0xff;
        payload_rx[i] = rand() & 0xff;
    }

    // get frame length
    unsigned int frame_len = qpacketmodem_get_frame_len(q);

    // allocate memory for frame samples
    float complex frame[frame_len];

    // encode frame
    qpacketmodem_encode(q, payload_tx, frame);

    // decode frame
    int crc_pass = qpacketmodem_decode_soft(q, frame, payload_rx);

    // destroy object
    qpacketmodem_destroy(q);

    // check to see that frame was recovered
    CONTEND_EQUALITY( crc_pass, 1 );
    CONTEND_SAME_DATA( payload_tx, payload_rx, _payload_len );
}

void autotest_qpacketmodem_bpsk()   { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QPSK);    }
void autotest_qpacketmodem_qpsk()   { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QPSK);    }
void autotest_qpacketmodem_psk8()   { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_PSK8);    }
void autotest_qpacketmodem_qam16()  { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM16);   }
void autotest_qpacketmodem_sqam32() { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_SQAM32);  }
void autotest_qpacketmodem_qam64()  { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM64);   }
void autotest_qpacketmodem_sqam128(){ qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_SQAM128); }
void autotest_qpacketmodem_qam256() { qpacketmodem_modulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM256);  }

// test error vector magnitude estimation
void autotest_qpacketmodem_evm()
{
    unsigned int payload_len = 800;
    int          check       = LIQUID_CRC_32;
    int          fec0        = LIQUID_FEC_NONE;
    int          fec1        = LIQUID_FEC_NONE;
    int          ms          = LIQUID_MODEM_QPSK;
    float        SNRdB       = 25.0f;

    // create and configure packet encoder/decoder object
    qpacketmodem q = qpacketmodem_create();
    qpacketmodem_configure(q, payload_len, check, fec0, fec1, ms);

    // initialize payload
    unsigned char payload_rx[payload_len];

    // get frame length and allocate memory for frame samples
    unsigned int frame_len = qpacketmodem_get_frame_len(q);
    float complex frame[frame_len];

    // encode frame (random bits)
    qpacketmodem_encode(q, NULL, frame);

    // add noise
    float nstd  = powf(10.0f, -SNRdB/20.0f);
    unsigned int i;
    for (i=0; i<frame_len; i++)
        frame[i] += nstd*(randnf() + _Complex_I*randnf())*M_SQRT1_2;

    // decode frame and get EVM estimate
    qpacketmodem_decode_soft(q, frame, payload_rx);
    float evm = qpacketmodem_get_demodulator_evm(q);
    if (liquid_autotest_verbose)
        printf("  EVM: %.3f dB, SNR: %.3f dB\n", evm, SNRdB);

    // destroy object
    qpacketmodem_destroy(q);

    // check EVM estimate; don't bother to check to see that frame was recovered
    CONTEND_DELTA( -evm, SNRdB, 0.5f );
#if 0
    FILE * fid = fopen("qpacketmodem_evm.txt","a");
    fprintf(fid,"%12.4e\n", SNRdB+evm);
    fclose(fid);
#endif
}

// 
// AUTOTEST : test un-modulated frame symbols (hard-decision demod)
//
void qpacketmodem_unmodulated(unsigned int _payload_len,
                              int          _check,
                              int          _fec0,
                              int          _fec1,
                              int          _ms)
{
    // derived values
    unsigned int i;

    // create and configure packet encoder/decoder object
    qpacketmodem q = qpacketmodem_create();
    qpacketmodem_configure(q, _payload_len, _check, _fec0, _fec1, _ms);
    if (liquid_autotest_verbose)
        qpacketmodem_print(q);

    // initialize payload
    unsigned char payload_tx[_payload_len];
    unsigned char payload_rx[_payload_len];

    // initialize payload
    for (i=0; i<_payload_len; i++) {
        payload_tx[i] = rand() & 0xff;
        payload_rx[i] = rand() & 0xff;
    }

    // get frame length (symbols)
    unsigned int frame_len = qpacketmodem_get_frame_len(q);

    // allocate memory for frame samples
    unsigned char frame_syms[frame_len];

    // encode frame symbols
    qpacketmodem_encode_syms(q, payload_tx, frame_syms);

    // decode frame symbols
    int crc_pass = qpacketmodem_decode_syms(q, frame_syms, payload_rx);

    // destroy object
    qpacketmodem_destroy(q);

    // check to see that frame was recovered
    CONTEND_EQUALITY( crc_pass, 1 );
    CONTEND_SAME_DATA( payload_tx, payload_rx, _payload_len );
}

void autotest_qpacketmodem_unmod_bpsk()   { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QPSK);    }
void autotest_qpacketmodem_unmod_qpsk()   { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QPSK);    }
void autotest_qpacketmodem_unmod_psk8()   { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_PSK8);    }
void autotest_qpacketmodem_unmod_qam16()  { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM16);   }
void autotest_qpacketmodem_unmod_sqam32() { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_SQAM32);  }
void autotest_qpacketmodem_unmod_qam64()  { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM64);   }
void autotest_qpacketmodem_unmod_sqam128(){ qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_SQAM128); }
void autotest_qpacketmodem_unmod_qam256() { qpacketmodem_unmodulated(400,LIQUID_CRC_32,LIQUID_FEC_NONE,LIQUID_FEC_NONE, LIQUID_MODEM_QAM256);  }

// test copy operation
void autotest_qpacketmodem_copy()
{
    // derived values
    unsigned int i;
    unsigned int payload_len = 400;
    crc_scheme   check       = LIQUID_CRC_24;
    fec_scheme   fec0        = LIQUID_FEC_SECDED7264;
    fec_scheme   fec1        = LIQUID_FEC_HAMMING128;
    int          ms          = LIQUID_MODEM_PI4DQPSK;

    // create and configure packet encoder/decoder object
    qpacketmodem q0 = qpacketmodem_create();
    qpacketmodem_configure(q0, payload_len, check, fec0, fec1, ms);

    // initialize buffers
    unsigned int frame_len = qpacketmodem_get_frame_len(q0);
    unsigned char payload_tx  [payload_len];
    float complex frame_syms_0[frame_len];
    float complex frame_syms_1[frame_len];
    unsigned char payload_rx_0[payload_len];
    unsigned char payload_rx_1[payload_len];

    // initialize payload
    for (i=0; i<payload_len; i++)
        payload_tx[i] = rand() & 0xff;

    // encode frame symbols
    qpacketmodem_encode(q0, payload_tx, frame_syms_0);

    // copy object, encode, and compare outputs
    qpacketmodem q1 = qpacketmodem_copy(q0);
    qpacketmodem_encode(q1, payload_tx, frame_syms_1);
    CONTEND_SAME_DATA( frame_syms_0, frame_syms_1, frame_len );

    // initialize received vector (can be random; just testing for equality with objects)
    for (i=0; i<payload_len; i++) {
        frame_syms_0[i] = randnf() + _Complex_I*randnf();
        frame_syms_1[i] =  frame_syms_0[i];
    }

    // decode frame symbols and compare outputs
    int crc_pass_0 = qpacketmodem_decode(q0, frame_syms_0, payload_rx_0);
    int crc_pass_1 = qpacketmodem_decode(q1, frame_syms_1, payload_rx_1);

    // check to see that frame was recovered
    CONTEND_SAME_DATA( payload_rx_0, payload_rx_1, payload_len );
    CONTEND_EQUALITY( crc_pass_0, crc_pass_1 );

    // destroy objects
    qpacketmodem_destroy(q0);
    qpacketmodem_destroy(q1);
}

