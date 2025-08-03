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

#include "autotest/autotest.h"
#include "liquid.h"

// 
// AUTOTEST: Gamma
//
void autotest_gamma()
{
    // error tolerance
    float tol = 1e-5f;

    // test vectors
    float v[12][2] = {
        {0.0001f, 9999.42288323161f     },
        {0.001f,   999.423772484595f    },
        {0.01f,     99.4325851191505f   },
        {0.1f,       9.51350769866873f  },
        {0.2f,       4.59084371199880f  },
        {0.5f,       1.77245385090552f  },
        {1.5f,       0.886226925452758f },
        {2.5f,       1.329340388179140f },
        {3.2f,       2.42396547993537f  },
        {4.1f,       6.81262286301667f  },
        {5.3f,      38.0779764499523f   },
        {12.0f, 39916800.0000000f       }};

    unsigned int i;
    for (i=0; i<12; i++) {
        // extract test vector
        float z = v[i][0];
        float g = v[i][1];

        // compute gamma
        float gamma = liquid_gammaf(z);

        // compute relative error
        float error = fabsf(gamma-g) / fabsf(g);

        // print results
        if (liquid_autotest_verbose)
            printf("  gamma(%12.4e) = %12.4e (expected %12.4e) %12.4e\n", z, gamma, g, error);

        // run test
        CONTEND_LESS_THAN(error, tol);
    }
}

// 
// AUTOTEST: lngamma
//
void autotest_lngamma()
{
    // error tolerance
    float tol = 1e-4f;

    // test vectors
    CONTEND_DELTA( liquid_lngammaf(1.00000000000000e-05), 1.15129196928958e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(1.47910838816821e-05), 1.11214774616959e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(2.18776162394955e-05), 1.07300339056431e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(3.23593656929628e-05), 1.03385883900717e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(4.78630092322638e-05), 9.94713997633970e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(7.07945784384137e-05), 9.55568727630758e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.04712854805090e-04), 9.16422823723390e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.54881661891248e-04), 8.77275982391398e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(2.29086765276777e-04), 8.38127754918765e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(3.38844156139203e-04), 7.98977478095072e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(5.01187233627273e-04), 7.59824172030200e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(7.41310241300918e-04), 7.20666389700363e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.09647819614319e-03), 6.81501995916639e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.62181009735893e-03), 6.42327843686104e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(2.39883291901949e-03), 6.03139302698778e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(3.54813389233576e-03), 5.63929577576287e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(5.24807460249773e-03), 5.24688733606614e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(7.76247116628692e-03), 4.85402329836329e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.14815362149688e-02), 4.46049557831785e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.69824365246175e-02), 4.06600834803635e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(2.51188643150958e-02), 3.67014984368726e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(3.71535229097173e-02), 3.27236635981559e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(5.49540873857625e-02), 2.87195653880158e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(8.12830516164099e-02), 2.46812982675138e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.20226443461741e-01), 2.06022544058646e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(1.77827941003892e-01), 1.64828757901128e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(2.63026799189538e-01), 1.23436563201614e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(3.89045144994281e-01), 8.25181176502332e-01, tol );
    CONTEND_DELTA( liquid_lngammaf(5.75439937337158e-01), 4.37193579132034e-01, tol );
    CONTEND_DELTA( liquid_lngammaf(8.51138038202378e-01), 1.05623142071343e-01, tol );
    CONTEND_DELTA( liquid_lngammaf(1.25892541179417e+00),-1.00254418080515e-01, tol );
    CONTEND_DELTA( liquid_lngammaf(1.86208713666287e+00),-5.19895823734552e-02, tol );
    CONTEND_DELTA( liquid_lngammaf(2.75422870333817e+00), 4.78681466346387e-01, tol );
    CONTEND_DELTA( liquid_lngammaf(4.07380277804113e+00), 1.88523210546678e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(6.02559586074358e+00), 4.83122059829788e+00, tol );
    CONTEND_DELTA( liquid_lngammaf(8.91250938133746e+00), 1.04177681572532e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(1.31825673855641e+01), 2.04497048921129e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(1.94984459975805e+01), 3.78565107279246e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(2.88403150312661e+01), 6.73552537656878e+01, tol );
    CONTEND_DELTA( liquid_lngammaf(4.26579518801593e+01), 1.16490742768456e+02, tol );
    CONTEND_DELTA( liquid_lngammaf(6.30957344480194e+01), 1.97262133863497e+02, tol );
    CONTEND_DELTA( liquid_lngammaf(9.33254300796992e+01), 3.28659150940827e+02, tol );
    CONTEND_DELTA( liquid_lngammaf(1.38038426460289e+02), 5.40606126998515e+02, tol );

    // test very large numbers
    CONTEND_DELTA( liquid_lngammaf(140), 550.278651724286, tol);
    CONTEND_DELTA( liquid_lngammaf(150), 600.009470555327, tol);
    CONTEND_DELTA( liquid_lngammaf(160), 650.409682895655, tol);
    CONTEND_DELTA( liquid_lngammaf(170), 701.437263808737, tol);
}

// 
// AUTOTEST: upper incomplete Gamma
//
void autotest_uppergamma()
{
    float tol = 1e-3f;

    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.001f), 1.04649f,  tol);

    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.01f),  1.04646f,  tol);

    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.1f),   1.04295f,  tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.2f),   1.03231f,  tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.3f),   1.01540f,  tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.4f),   0.993237f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.5f),   0.966782f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.6f),   0.936925f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.7f),   0.904451f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.8f),   0.870053f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 0.9f),   0.834330f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 1.0f),   0.797796f, tol);

    CONTEND_DELTA(liquid_uppergammaf(2.1f, 2.0f),   0.455589f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 3.0f),   0.229469f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 4.0f),   0.107786f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 5.0f),   0.0484292f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 6.0f),   0.0211006f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 7.0f),   0.00898852f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 8.0f),   0.00376348f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 9.0f),   0.00155445f, tol);
    CONTEND_DELTA(liquid_uppergammaf(2.1f, 10.0f),  0.000635002f, tol);
}

// 
// AUTOTEST: Factorial
//
void autotest_factorial()
{
    float tol = 1e-3f;
    CONTEND_DELTA(liquid_factorialf(0), 1,   tol);
    CONTEND_DELTA(liquid_factorialf(1), 1,   tol);
    CONTEND_DELTA(liquid_factorialf(2), 2,   tol);
    CONTEND_DELTA(liquid_factorialf(3), 6,   tol);
    CONTEND_DELTA(liquid_factorialf(4), 24,  tol);
    CONTEND_DELTA(liquid_factorialf(5), 120, tol);
    CONTEND_DELTA(liquid_factorialf(6), 720, tol);
}

// 
// AUTOTEST: nchoosek
//
void autotest_nchoosek()
{
    float tol = 1e-3f;

    // nchoosek(6, k)
    CONTEND_DELTA(liquid_nchoosek(6,    0),      1,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    1),      6,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    2),     15,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    3),     20,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    4),     15,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    5),      6,     tol);
    CONTEND_DELTA(liquid_nchoosek(6,    6),      1,     tol);

    // nchoosek(7, k)
    CONTEND_DELTA(liquid_nchoosek(7,    0),      1,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    1),      7,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    2),     21,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    3),     35,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    4),     35,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    5),     21,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    6),      7,     tol);
    CONTEND_DELTA(liquid_nchoosek(7,    7),      1,     tol);

    // test very large numbers
    CONTEND_DELTA(liquid_nchoosek(124,  5),     225150024,  5000);
}

