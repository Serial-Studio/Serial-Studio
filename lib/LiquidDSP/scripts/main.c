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

// 
// scripts/main.c
// 
// Auto-script generator test program

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autoscript.h"

void usage() {
    printf("Usage: autoscript_test DELIM TYPE [LIST OF FILES]\n");
    printf("    DELIM   :   path separation delimiter (e.g. '/')\n");
    printf("    TYPE    :   base type for parsing (e.g. \"benchmark\")\n");
    printf("    [LIST]  :   list of auto-script files, named MYPACKAGE_TYPE.h\n");
    printf("                with internal scripts \"void TYPE_MYSCRIPT(...)\"\n");
}

int main(int argc, char*argv[])
{
    //
    if (argc < 3) {
        // print help
        fprintf(stderr,"error: %s, too few arguments\n", argv[0]);
        usage();
        exit(1);
    } else {
        //printf("//  delim :   '%c'\n", argv[1][0]);
        //printf("//  type  :   \"%s\"\n", argv[2]);
    }

    // create autoscript object
    //  first argument  :   type string (e.g. "benchmark")
    //  second argument :   delimiter character (file system)
    autoscript q = autoscript_create(argv[2], argv[1][0]);

    // parse files
    int i;
    for (i=3; i<argc; i++) {
        autoscript_parse(q,argv[i]);
    }

    // print results
    autoscript_print(q);

    // destroy autoscript object
    autoscript_destroy(q);

    return 0;
}

