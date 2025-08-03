/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
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
// framedatastats.c
//
// Default and generic frame statistics
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <complex.h>

#include "liquid.internal.h"

// reset framedatastats object
int framedatastats_reset(framedatastats_s * _stats)
{
    if (_stats == NULL)
        return LIQUID_OK;

    _stats->num_frames_detected = 0;
    _stats->num_headers_valid   = 0;
    _stats->num_payloads_valid  = 0;
    _stats->num_bytes_received  = 0;
    return LIQUID_OK;
}

// print framedatastats object
int framedatastats_print(framedatastats_s * _stats)
{
    if (_stats == NULL)
        return LIQUID_OK;

    float percent_headers  = 0.0f;
    float percent_payloads = 0.0f;
    if (_stats->num_frames_detected > 0) {
        percent_headers  = 100.0f*(float)_stats->num_headers_valid  / (float)_stats->num_frames_detected;
        percent_payloads = 100.0f*(float)_stats->num_payloads_valid / (float)_stats->num_frames_detected;
    }

    printf("  frames detected   : %u\n", _stats->num_frames_detected);
    printf("  headers valid     : %-6u (%8.4f %%)\n", _stats->num_headers_valid,  percent_headers);
    printf("  payloads valid    : %-6u (%8.4f %%)\n", _stats->num_payloads_valid, percent_payloads);
    printf("  bytes received    : %lu\n", _stats->num_bytes_received);
    return LIQUID_OK;
}

