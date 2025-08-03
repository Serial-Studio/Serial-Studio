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
// scripts/autoscript.c
// 
// auto-script generator object definition
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autoscript.h"

#define NAME_LEN            (256)
#define AUTOSCRIPT_VERSION  "0.3.1"
#define DEBUG_AUTOSCRIPT    0

struct script_s {
    char name[NAME_LEN];
    struct package_s * package;
};

struct package_s {
    char name[NAME_LEN];
    char filename[NAME_LEN];
    struct script_s * scripts;
    unsigned int num_scripts;
};

struct autoscript_s {
    char type[NAME_LEN];
    char delim;
    unsigned int num_packages;
    struct package_s * packages;
};

// create autoscript generator object
autoscript autoscript_create(char * _type,
                             char _delim)
{
    autoscript q = (autoscript) malloc(sizeof(struct autoscript_s));

    // copy type and delimiter (path separator)
    strncpy(q->type, _type, NAME_LEN-1);
    q->type[NAME_LEN-1] = '\0';
    q->delim = _delim;

    q->num_packages = 0;
    q->packages = NULL;

    return q;
}

void autoscript_destroy(autoscript _q)
{
    // free all internal scripts
    unsigned int i;
    for (i=0; i<_q->num_packages; i++) {
        if (_q->packages[i].scripts != NULL)
            free(_q->packages[i].scripts);
    }

    // free all internal packages
    if (_q->packages != NULL)
        free(_q->packages);

    // free main object memory
    free(_q);
}

// parse filename and file
void autoscript_parse(autoscript _q,
                      char * _filename)
{
#if DEBUG_AUTOSCRIPT
    fprintf(stderr,"autoscript_parse('%s')...\n", _filename);
#endif

    char package_name[NAME_LEN];

    // parse filename...
    autoscript_parsefilename(_q, _filename, package_name);

    // parse actual file...
    autoscript_parsefile(_q, _filename, package_name);
}

void autoscript_print(autoscript _q)
{
    unsigned int i;
    unsigned int j;
    unsigned int n;

    // count total number of scripts
    unsigned int num_scripts = 0;
    for (i=0; i<_q->num_packages; i++)
        num_scripts += _q->packages[i].num_scripts;

    // print header
    printf("// auto-generated file, do not edit\n");
    printf("// invoked with script type '%s' and delimiter '%c'\n", _q->type, _q->delim);
    printf("// the following types need to be defined externally:\n");
    printf("//      typedef ...(%s_function_t)(...);\n", _q->type);
    printf("//      typedef struct {\n");
    printf("//          unsigned int id;        // script identification number\n");
    printf("//          %s_function_t * api;    // pointer to function API\n", _q->type);
    printf("//          const char * name;      // script name\n");
    printf("//          ...\n");
    printf("//      } %s_t;\n", _q->type);
    printf("//      typedef struct {\n");
    printf("//          unsigned int id;        // package identification number\n");
    printf("//          unsigned int index;     // index of first script\n");
    printf("//          unsigned int n;         // number of scripts in package\n");
    printf("//          const char * name;      // name of package\n");
    printf("//      } package_t;\n");
    printf("\n");
    
    printf("#ifndef __LIQUID_AUTOSCRIPT_INCLUDE_H__\n");
    printf("#define __LIQUID_AUTOSCRIPT_INCLUDE_H__\n\n");

    printf("#define AUTOSCRIPT_VERSION \"%s\"\n\n", AUTOSCRIPT_VERSION);

    printf("// number of packages\n");
    printf("#define NUM_PACKAGES (%u)\n\n", _q->num_packages);

    printf("// number of scripts\n");
    printf("#define NUM_AUTOSCRIPTS (%u)\n\n", num_scripts);

    printf("// function declarations\n");
    for (i=0; i<_q->num_packages; i++) {
        struct package_s * p = &_q->packages[i];
        printf("// %s\n", p->filename);
#if 0
        for (j=0; j<p->num_scripts; j++)
            printf("void benchmark_%s(AUTOSCRIPT_ARGS);\n", p->scripts[j].name);
#else
        for (j=0; j<p->num_scripts; j++)
            printf("%s_function_t %s_%s;\n", _q->type,
                                             _q->type,
                                             p->scripts[j].name);
#endif
    }
    printf("\n");

    // find longest name
    unsigned int max_len = 0;
    for (i=0; i<_q->num_packages; i++) {
        //struct package_s * p = &_q->packages[i];
        for (j=0; j<_q->packages[i].num_scripts; j++) {
            unsigned int str_len = strlen(_q->packages[i].scripts[j].name);
            max_len = str_len > max_len ? str_len : max_len;
        }
    }
    char space[max_len+1];
    memset(space, ' ', max_len);
    space[max_len] = '\0';

    printf("// array of scripts\n");
    printf("%s_t scripts[NUM_AUTOSCRIPTS] = {\n", _q->type);
    n=0;
    for (i=0; i<_q->num_packages; i++) {
        struct package_s * p = &_q->packages[i];
        for (j=0; j<p->num_scripts; j++) {
            // print formatting line, adding appropriate space to align columns
            printf("    {.id = %4u, .api = &%s_%s,%s .name = \"%s\"}",
                n,
                _q->type,
                p->scripts[j].name,
                &space[strlen(p->scripts[j].name)],
                p->scripts[j].name);
            if ( n < num_scripts-1 )
                printf(",");
            printf("\n");
            n++;
        }
    }
    printf("};\n\n");

    n=0;
    printf("// array of packages\n");
    printf("package_t packages[NUM_PACKAGES] = {\n");
    for (i=0; i<_q->num_packages; i++) {
        printf("    {.id = %4u, .index = %4u, .num_scripts = %4u, .name = \"%s\"}",
            i,
            n,
            _q->packages[i].num_scripts,
            _q->packages[i].name);
        if (i<_q->num_packages-1)
            printf(",");
        printf("\n");

        n += _q->packages[i].num_scripts;
    }
    printf("};\n\n");

    printf("#endif // __LIQUID_AUTOSCRIPT_INCLUDE_H__\n\n");

}



//
// internal methods
//

// parse filename
//  _q              :   generator object
//  _filename       :   name of file
//  _package_name   :   output package name (stripped filename)
void autoscript_parsefilename(autoscript _q,
                              char * _filename,
                              char * _package_name)
{
#if DEBUG_AUTOSCRIPT
    fprintf(stderr,"autoscript_parsefilename('%s')...\n", _filename);
#endif
    char * sptr;                // pointer to base name
    char * tptr;                // pointer to terminating tag
    char pathsep = _q->delim;   // path separator character

    // generate tag (e.g. "_benchmark")
    unsigned int tag_len = strlen(_q->type) + 2;
    char tag[tag_len];
    tag[0] = '_';
    strcpy(tag+1, _q->type);
    tag[tag_len-1] = '\0';
    //printf("// tag : '%s'\n", tag);

    // try to strip out path: find rightmost occurrence of pathsep
    //printf("%s\n", _filename);
    sptr = strrchr(_filename, pathsep);   // obtain index of last pathsep
    if (sptr == NULL) {
        // path delimiter not found
        sptr = _filename;
    } else {
        sptr++;   // increment to remove path separator character
    }

    // try to strip out tag
    tptr = strstr( sptr, tag );
    if (tptr == NULL) {
        // TODO : handle this case differently otherwise
        fprintf(stderr,"error: autoscript_parsefilename('%s'), tag '%s' not found\n", _filename, tag);
        exit(1);
    }

    // copy base name to output
    int n = tptr - sptr;
    strncpy(_package_name, sptr, n);
    _package_name[n] = '\0';

    //printf("// package name : '%s'\n", _package_name);
}


// parse file
//  _q              :   generator object
//  _filename       :   name of file
//  _package_name   :   input package name (stripped filename)
void autoscript_parsefile(autoscript _q,
                          char * _filename,
                          char * _package_name)
{
    // flag indicating if package has been added or not
    int package_added = 0;

#if DEBUG_AUTOSCRIPT
    fprintf(stderr,"autoscript_parsefile('%s')...\n", _filename);
#endif
    // try to open file...
    FILE * fid = fopen(_filename,"r");
    if (!fid) {
        fprintf(stderr,"error: autoscript_parsefile('%s'), could not open file for reading\n", _filename);
        exit(1);
    }
    // generate tag (e.g. "void benchmark_");
    unsigned int tag_len = 5 + strlen(_q->type) + 2;
    char tag[tag_len];
    sprintf(tag, "void %s_", _q->type);

    // parse file, looking for key
    char buffer[1024];      // line buffer
    char basename[1024];    // base script name
    char * cptr = NULL;     // readline return value
    char * sptr = NULL;     // tag string pointer
    int cterm;              // terminating character
    unsigned int n=0;       // line number
    do {
        // increment line number
        n++;

        // read line
        cptr = fgets(buffer, 1024, fid);

        if (cptr != NULL) {
            // search for key
            sptr = strstr(cptr, tag);
            if (sptr == NULL) {
                // no value found
                continue;
            }
            // key found: find terminating character and strip out base name
            sptr += strlen(tag);    // increment string by tag length
            cterm = strcspn( sptr, " (\t\n\r" );
            if (cterm == strlen(sptr)) {
                // no terminating character found
                continue;
            }
            // copy base name
            strncpy( basename, sptr, cterm );
            basename[cterm] = '\0';
            //printf("// line %3u : '%s'\n", n, basename);
                    
            // key found: add package if not already done
            if (!package_added) {
                // TODO : add base name
                autoscript_addpackage(_q, _package_name, _filename);
                package_added = 1;
            }
            autoscript_addscript(_q, _package_name, basename);
            printf("// adding %s to package %s from %s:%u\n", basename, _package_name, _filename, n);
        }
    } while (!feof(fid));

    // close file
    fclose(fid);
}

void autoscript_addpackage(autoscript _q,
                           char * _package_name,
                           char * _filename)
{
    // verify that package name is unique with existing scripts
    unsigned int i;
    for (i=0; i<_q->num_packages; i++) {
        if (strcmp(_q->packages[i].name, _package_name)==0) {
            fprintf(stderr,"error: duplicate package name '%s', defined in:\n", _package_name);
            fprintf(stderr,"  %s\n", _q->packages[i].filename);
            fprintf(stderr,"  %s\n", _filename);
            exit(1);
        }
    }

    // increase package size
    _q->num_packages++;

    // re-allocate memory for packages
    _q->packages = (struct package_s *) realloc(_q->packages,
                                                _q->num_packages*sizeof(struct package_s));

    // initialize new package
    strncpy(_q->packages[_q->num_packages-1].name, _package_name, NAME_LEN-1);
    _q->packages[_q->num_packages-1].name[NAME_LEN-1] = '\0';

    // initialize new package
    strncpy(_q->packages[_q->num_packages-1].filename, _filename, NAME_LEN-1);
    _q->packages[_q->num_packages-1].filename[NAME_LEN-1] = '\0';

    // initialize number of scripts
    _q->packages[_q->num_packages-1].num_scripts = 0;

    // set scripts pointer to NULL
    _q->packages[_q->num_packages-1].scripts = NULL;
}

void autoscript_addscript(autoscript _q,
                          char * _package_name,
                          char * _script_name)
{
    // first validate that package exists
    unsigned int i;
    int package_found = 0;
    for (i=0; i<_q->num_packages; i++) {
        if ( strcmp(_q->packages[i].name, _package_name)==0 ) {
            package_found = 1;
            break;
        }
    }

    if (!package_found) {
        fprintf(stderr,"error: autoscript_addscript(), unknown package '%s'\n", _package_name);
        exit(1);
    }

    struct package_s * p = &_q->packages[i];

    // increase script size
    p->num_scripts++;

    // re-allocate memory for scripts
    p->scripts = (struct script_s *) realloc(p->scripts,
                                             p->num_scripts*sizeof(struct script_s));

    // initialize new script
    strncpy(p->scripts[p->num_scripts-1].name, _script_name, NAME_LEN-1);
    p->scripts[p->num_scripts-1].name[NAME_LEN-1] = '\0';

    // associate script with package
    //_q->scripts[_q->num_scripts-1].package = &_q->packages[i];
}

