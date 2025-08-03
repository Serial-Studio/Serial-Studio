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

#include <stdlib.h>
#include "autotest/autotest.h"
#include "liquid.internal.h"

// Helper function to keep code base small
void modemcf_test_copy(modulation_scheme _ms)
{
    // create modem and randomize internal state
    modemcf modem_0 = modemcf_create(_ms);
    unsigned int i, s0, s1, M = 1 << modemcf_get_bps(modem_0);
    float complex x0, x1;
    for (i=0; i<10; i++) {
        // modulate random symbol
        modemcf_modulate(modem_0, rand() % M, &x0);

        // demodulate random sample
        modemcf_demodulate(modem_0, randnf() + _Complex_I*randnf(), &s0);
    }

    // copy modem
    modemcf modem_1 = modemcf_copy(modem_0);

    if (liquid_autotest_verbose) {
        printf("input: %20s, 0: %20s, 1:%20s\n",
                modulation_types[_ms                        ].name,
                modulation_types[modemcf_get_scheme(modem_0)].name,
                modulation_types[modemcf_get_scheme(modem_1)].name);
    }

    // ...
    for (i=0; i<10; i++) {
        // modulate random symbol
        unsigned int s = rand() % M;
        modemcf_modulate(modem_0, s, &x0);
        modemcf_modulate(modem_1, s, &x1);
        CONTEND_EQUALITY(x0, x1);

        // demodulate random sample
        float complex x = randnf() + _Complex_I*randnf();
        modemcf_demodulate(modem_0, x, &s0);
        modemcf_demodulate(modem_1, x, &s1);
        CONTEND_EQUALITY(s0, s1)
    }

    // clean it up
    modemcf_destroy(modem_0);
    modemcf_destroy(modem_1);
}

// AUTOTESTS: generic PSK
void autotest_modem_copy_psk2()      { modemcf_test_copy(LIQUID_MODEM_PSK2);      }
void autotest_modem_copy_psk4()      { modemcf_test_copy(LIQUID_MODEM_PSK4);      }
void autotest_modem_copy_psk8()      { modemcf_test_copy(LIQUID_MODEM_PSK8);      }
void autotest_modem_copy_psk16()     { modemcf_test_copy(LIQUID_MODEM_PSK16);     }
void autotest_modem_copy_psk32()     { modemcf_test_copy(LIQUID_MODEM_PSK32);     }
void autotest_modem_copy_psk64()     { modemcf_test_copy(LIQUID_MODEM_PSK64);     }
void autotest_modem_copy_psk128()    { modemcf_test_copy(LIQUID_MODEM_PSK128);    }
void autotest_modem_copy_psk256()    { modemcf_test_copy(LIQUID_MODEM_PSK256);    }

// AUTOTESTS: generic DPSK
void autotest_modem_copy_dpsk2()     { modemcf_test_copy(LIQUID_MODEM_DPSK2);     }
void autotest_modem_copy_dpsk4()     { modemcf_test_copy(LIQUID_MODEM_DPSK4);     }
void autotest_modem_copy_dpsk8()     { modemcf_test_copy(LIQUID_MODEM_DPSK8);     }
void autotest_modem_copy_dpsk16()    { modemcf_test_copy(LIQUID_MODEM_DPSK16);    }
void autotest_modem_copy_dpsk32()    { modemcf_test_copy(LIQUID_MODEM_DPSK32);    }
void autotest_modem_copy_dpsk64()    { modemcf_test_copy(LIQUID_MODEM_DPSK64);    }
void autotest_modem_copy_dpsk128()   { modemcf_test_copy(LIQUID_MODEM_DPSK128);   }
void autotest_modem_copy_dpsk256()   { modemcf_test_copy(LIQUID_MODEM_DPSK256);   }

// AUTOTESTS: generic ASK
void autotest_modem_copy_ask2()      { modemcf_test_copy(LIQUID_MODEM_ASK2);      }
void autotest_modem_copy_ask4()      { modemcf_test_copy(LIQUID_MODEM_ASK4);      }
void autotest_modem_copy_ask8()      { modemcf_test_copy(LIQUID_MODEM_ASK8);      }
void autotest_modem_copy_ask16()     { modemcf_test_copy(LIQUID_MODEM_ASK16);     }
void autotest_modem_copy_ask32()     { modemcf_test_copy(LIQUID_MODEM_ASK32);     }
void autotest_modem_copy_ask64()     { modemcf_test_copy(LIQUID_MODEM_ASK64);     }
void autotest_modem_copy_ask128()    { modemcf_test_copy(LIQUID_MODEM_ASK128);    }
void autotest_modem_copy_ask256()    { modemcf_test_copy(LIQUID_MODEM_ASK256);    }

// AUTOTESTS: generic QAM
void autotest_modem_copy_qam4()      { modemcf_test_copy(LIQUID_MODEM_QAM4);      }
void autotest_modem_copy_qam8()      { modemcf_test_copy(LIQUID_MODEM_QAM8);      }
void autotest_modem_copy_qam16()     { modemcf_test_copy(LIQUID_MODEM_QAM16);     }
void autotest_modem_copy_qam32()     { modemcf_test_copy(LIQUID_MODEM_QAM32);     }
void autotest_modem_copy_qam64()     { modemcf_test_copy(LIQUID_MODEM_QAM64);     }
void autotest_modem_copy_qam128()    { modemcf_test_copy(LIQUID_MODEM_QAM128);    }
void autotest_modem_copy_qam256()    { modemcf_test_copy(LIQUID_MODEM_QAM256);    }

// AUTOTESTS: generic APSK (maps to specific APSK modems internally)
void autotest_modem_copy_apsk4()     { modemcf_test_copy(LIQUID_MODEM_APSK4);     }
void autotest_modem_copy_apsk8()     { modemcf_test_copy(LIQUID_MODEM_APSK8);     }
void autotest_modem_copy_apsk16()    { modemcf_test_copy(LIQUID_MODEM_APSK16);    }
void autotest_modem_copy_apsk32()    { modemcf_test_copy(LIQUID_MODEM_APSK32);    }
void autotest_modem_copy_apsk64()    { modemcf_test_copy(LIQUID_MODEM_APSK64);    }
void autotest_modem_copy_apsk128()   { modemcf_test_copy(LIQUID_MODEM_APSK128);   }
void autotest_modem_copy_apsk256()   { modemcf_test_copy(LIQUID_MODEM_APSK256);   }

// AUTOTESTS: Specific modems
void autotest_modem_copy_bpsk()      { modemcf_test_copy(LIQUID_MODEM_BPSK);      }
void autotest_modem_copy_qpsk()      { modemcf_test_copy(LIQUID_MODEM_QPSK);      }
void autotest_modem_copy_ook()       { modemcf_test_copy(LIQUID_MODEM_OOK);       }
void autotest_modem_copy_sqam32()    { modemcf_test_copy(LIQUID_MODEM_SQAM32);    }
void autotest_modem_copy_sqam128()   { modemcf_test_copy(LIQUID_MODEM_SQAM128);   }
void autotest_modem_copy_V29()       { modemcf_test_copy(LIQUID_MODEM_V29);       }
void autotest_modem_copy_arb16opt()  { modemcf_test_copy(LIQUID_MODEM_ARB16OPT);  }
void autotest_modem_copy_arb32opt()  { modemcf_test_copy(LIQUID_MODEM_ARB32OPT);  }
void autotest_modem_copy_arb64opt()  { modemcf_test_copy(LIQUID_MODEM_ARB64OPT);  }
void autotest_modem_copy_arb128opt() { modemcf_test_copy(LIQUID_MODEM_ARB128OPT); }
void autotest_modem_copy_arb256opt() { modemcf_test_copy(LIQUID_MODEM_ARB256OPT); }
void autotest_modem_copy_arb64vt()   { modemcf_test_copy(LIQUID_MODEM_ARB64VT);   }
void autotest_modem_copy_pi4dqpsk()  { modemcf_test_copy(LIQUID_MODEM_PI4DQPSK);  }

// test errors and invalid configuration
void autotest_modem_config()
{
#if LIQUID_STRICT_EXIT
    AUTOTEST_WARN("skipping modem config test with strict exit enabled\n");
    return;
#endif
#if !LIQUID_SUPPRESS_ERROR_OUTPUT
    fprintf(stderr,"warning: ignore potential errors here; checking for invalid configurations\n");
#endif
    // test copying/creating invalid objects
    CONTEND_ISNULL( modemcf_copy(NULL) );
    CONTEND_ISNULL( modemcf_create(LIQUID_MODEM_ARB) );
    CONTEND_ISNULL( modemcf_create(-1) );

    // create object and check configuration
    modemcf q = modemcf_create(LIQUID_MODEM_QAM64);
    CONTEND_EQUALITY( LIQUID_OK, modemcf_print(q) );

    // internal: try to initialize using invalid configuration
    CONTEND_INEQUALITY( LIQUID_OK, modemcf_init(q,0) );
    CONTEND_INEQUALITY( LIQUID_OK, modemcf_init(q,77) );

    // internal: try to modulate using invalid inputs
    float complex sym;
    CONTEND_INEQUALITY( LIQUID_OK, modemcf_modulate    (q,8193,&sym) );
    CONTEND_INEQUALITY( LIQUID_OK, modemcf_modulate_map(q,8193,&sym) );
    CONTEND_INEQUALITY( LIQUID_OK, modemcf_demodsoft_gentab(q,227) );

    modemcf_destroy(q);
}

