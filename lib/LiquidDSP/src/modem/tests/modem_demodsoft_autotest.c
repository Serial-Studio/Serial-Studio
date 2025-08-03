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
// soft demodulation tests
//

#include "autotest/autotest.h"
#include "liquid.h"

// Help function to keep code base small
void modemcf_test_demodsoft(modulation_scheme _ms)
{
    // generate mod/demod
    modemcf mod   = modemcf_create(_ms);
    modemcf demod = modemcf_create(_ms);

    // 
    unsigned int bps = modemcf_get_bps(demod);

    // run the test
    unsigned int i, s, M=1<<bps;
    unsigned int sym_soft;
    unsigned char soft_bits[bps];
    float complex x;
    
    for (i=0; i<M; i++) {
        // modulate symbol
        modemcf_modulate(mod, i, &x);

        // demodulate using soft-decision
        modemcf_demodulate_soft(demod, x, &s, soft_bits);

        // check hard-decision output
        CONTEND_EQUALITY(s, i);

        // check soft bits
        liquid_pack_soft_bits(soft_bits, bps, &sym_soft);
        CONTEND_EQUALITY(sym_soft, i);

        // check phase error, evm, etc.
        //CONTEND_DELTA( modemcf_get_demodulator_phase_error(demod), 0.0f, 1e-3f);
        //CONTEND_DELTA( modemcf_get_demodulator_evm(demod), 0.0f, 1e-3f);
    }
    // clean it up
    modemcf_destroy(mod);
    modemcf_destroy(demod);
}

// AUTOTESTS: generic PSK
void autotest_demodsoft_psk2()      { modemcf_test_demodsoft(LIQUID_MODEM_PSK2);      }
void autotest_demodsoft_psk4()      { modemcf_test_demodsoft(LIQUID_MODEM_PSK4);      }
void autotest_demodsoft_psk8()      { modemcf_test_demodsoft(LIQUID_MODEM_PSK8);      }
void autotest_demodsoft_psk16()     { modemcf_test_demodsoft(LIQUID_MODEM_PSK16);     }
void autotest_demodsoft_psk32()     { modemcf_test_demodsoft(LIQUID_MODEM_PSK32);     }
void autotest_demodsoft_psk64()     { modemcf_test_demodsoft(LIQUID_MODEM_PSK64);     }
void autotest_demodsoft_psk128()    { modemcf_test_demodsoft(LIQUID_MODEM_PSK128);    }
void autotest_demodsoft_psk256()    { modemcf_test_demodsoft(LIQUID_MODEM_PSK256);    }

// AUTOTESTS: generic DPSK
void autotest_demodsoft_dpsk2()     { modemcf_test_demodsoft(LIQUID_MODEM_DPSK2);     }
void autotest_demodsoft_dpsk4()     { modemcf_test_demodsoft(LIQUID_MODEM_DPSK4);     }
void autotest_demodsoft_dpsk8()     { modemcf_test_demodsoft(LIQUID_MODEM_DPSK8);     }
void autotest_demodsoft_dpsk16()    { modemcf_test_demodsoft(LIQUID_MODEM_DPSK16);    }
void autotest_demodsoft_dpsk32()    { modemcf_test_demodsoft(LIQUID_MODEM_DPSK32);    }
void autotest_demodsoft_dpsk64()    { modemcf_test_demodsoft(LIQUID_MODEM_DPSK64);    }
void autotest_demodsoft_dpsk128()   { modemcf_test_demodsoft(LIQUID_MODEM_DPSK128);   }
void autotest_demodsoft_dpsk256()   { modemcf_test_demodsoft(LIQUID_MODEM_DPSK256);   }

// AUTOTESTS: generic ASK
void autotest_demodsoft_ask2()      { modemcf_test_demodsoft(LIQUID_MODEM_ASK2);      }
void autotest_demodsoft_ask4()      { modemcf_test_demodsoft(LIQUID_MODEM_ASK4);      }
void autotest_demodsoft_ask8()      { modemcf_test_demodsoft(LIQUID_MODEM_ASK8);      }
void autotest_demodsoft_ask16()     { modemcf_test_demodsoft(LIQUID_MODEM_ASK16);     }
void autotest_demodsoft_ask32()     { modemcf_test_demodsoft(LIQUID_MODEM_ASK32);     }
void autotest_demodsoft_ask64()     { modemcf_test_demodsoft(LIQUID_MODEM_ASK64);     }
void autotest_demodsoft_ask128()    { modemcf_test_demodsoft(LIQUID_MODEM_ASK128);    }
void autotest_demodsoft_ask256()    { modemcf_test_demodsoft(LIQUID_MODEM_ASK256);    }

// AUTOTESTS: generic QAM
void autotest_demodsoft_qam4()      { modemcf_test_demodsoft(LIQUID_MODEM_QAM4);      }
void autotest_demodsoft_qam8()      { modemcf_test_demodsoft(LIQUID_MODEM_QAM8);      }
void autotest_demodsoft_qam16()     { modemcf_test_demodsoft(LIQUID_MODEM_QAM16);     }
void autotest_demodsoft_qam32()     { modemcf_test_demodsoft(LIQUID_MODEM_QAM32);     }
void autotest_demodsoft_qam64()     { modemcf_test_demodsoft(LIQUID_MODEM_QAM64);     }
void autotest_demodsoft_qam128()    { modemcf_test_demodsoft(LIQUID_MODEM_QAM128);    }
void autotest_demodsoft_qam256()    { modemcf_test_demodsoft(LIQUID_MODEM_QAM256);    }

// AUTOTESTS: generic APSK (maps to specific APSK modems internally)
void autotest_demodsoft_apsk4()     { modemcf_test_demodsoft(LIQUID_MODEM_APSK4);     }
void autotest_demodsoft_apsk8()     { modemcf_test_demodsoft(LIQUID_MODEM_APSK8);     }
void autotest_demodsoft_apsk16()    { modemcf_test_demodsoft(LIQUID_MODEM_APSK16);    }
void autotest_demodsoft_apsk32()    { modemcf_test_demodsoft(LIQUID_MODEM_APSK32);    }
void autotest_demodsoft_apsk64()    { modemcf_test_demodsoft(LIQUID_MODEM_APSK64);    }
void autotest_demodsoft_apsk128()   { modemcf_test_demodsoft(LIQUID_MODEM_APSK128);   }
void autotest_demodsoft_apsk256()   { modemcf_test_demodsoft(LIQUID_MODEM_APSK256);   }

// AUTOTESTS: Specific modems
void autotest_demodsoft_bpsk()      { modemcf_test_demodsoft(LIQUID_MODEM_BPSK);      }
void autotest_demodsoft_qpsk()      { modemcf_test_demodsoft(LIQUID_MODEM_QPSK);      }
void autotest_demodsoft_ook()       { modemcf_test_demodsoft(LIQUID_MODEM_OOK);       }
void autotest_demodsoft_sqam32()    { modemcf_test_demodsoft(LIQUID_MODEM_SQAM32);    }
void autotest_demodsoft_sqam128()   { modemcf_test_demodsoft(LIQUID_MODEM_SQAM128);   }
void autotest_demodsoft_V29()       { modemcf_test_demodsoft(LIQUID_MODEM_V29);       }
void autotest_demodsoft_arb16opt()  { modemcf_test_demodsoft(LIQUID_MODEM_ARB16OPT);  }
void autotest_demodsoft_arb32opt()  { modemcf_test_demodsoft(LIQUID_MODEM_ARB32OPT);  }
void autotest_demodsoft_arb64opt()  { modemcf_test_demodsoft(LIQUID_MODEM_ARB64OPT);  }
void autotest_demodsoft_arb128opt() { modemcf_test_demodsoft(LIQUID_MODEM_ARB128OPT); }
void autotest_demodsoft_arb256opt() { modemcf_test_demodsoft(LIQUID_MODEM_ARB256OPT); }
void autotest_demodsoft_arb64vt()   { modemcf_test_demodsoft(LIQUID_MODEM_ARB64VT);   }
void autotest_demodsoft_pi4dqpsk()  { modemcf_test_demodsoft(LIQUID_MODEM_PI4DQPSK);  }

