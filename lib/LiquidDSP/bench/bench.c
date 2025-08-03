/*
 * Copyright (c) 2007 - 2022 Joseph Gaeddert
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

// bench.c
//
// This file is used in conjunction with benchinclude.h (generated with
// scripts/autoscript) to produce an executable for benchmarking the various
// signal processing algorithms in liquid.
//


// default include headers
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/resource.h>

// define benchmark function pointer
typedef void(benchmark_function_t) (
    struct rusage *_start,
    struct rusage *_finish,
    unsigned long int *_num_iterations);

// define benchmark_t
typedef struct {
    unsigned int           id;     // identification of benchmark
    benchmark_function_t * api;    // function interface
    const char *           name;   // name of function
    unsigned int           name_len;
    unsigned int           num_trials;
    float                  extime;
    float                  rate;
    float                  cycles_per_trial;//
    unsigned int           num_attempts;    // number of attempts to reach target time
} benchmark_t;

// define package_t
typedef struct {
    unsigned int id;            // package identification
    unsigned int index;         // index of first benchmark
    unsigned int num_scripts;   // number of tests in package
    const char* name;           // package name
} package_t;

// include auto-generated benchmark header
//
// defines the following symbols:
//   #define AUTOSCRIPT_VERSION
//   #define NUM_AUTOSCRIPTS
//   benchmark_t scripts[NUM_AUTOSCRIPTS]
//   #define NUM_PACKAGES
//   package_t packages[NUM_PACKAGES]
#include "benchmark_include.h"

// helper functions:
void estimate_cpu_clock(void);
void set_num_trials_from_cpu_speed(void);
void execute_benchmark(benchmark_t* _benchmark, int _verbose);
void execute_package(package_t* _package, int _verbose);

char convert_units(float * _s);
void print_benchmark_results(benchmark_t* _benchmark);
void print_package_results(package_t* _package);
double calculate_execution_time(struct rusage, struct rusage);

unsigned long int num_base_trials = 1<<12;
float cpu_clock = 1.0f; // cpu clock speed (Hz)
float runtime=0.050f;   // minimum run time (s)

FILE * fid; // output file id
void output_benchmark_to_file(FILE * _fid, benchmark_t * _benchmark);

void usage()
{
    // help
    printf("Usage: benchmark [OPTION]\n");
    printf("Execute benchmark scripts for liquid-dsp library.\n");
    printf("  -h           : display this help and exit\n");
    printf("  -v           : verbose\n");
    printf("  -q           : quiet\n");
    printf("  -f           : run fast version\n");
    printf("  -e           : estimate cpu clock frequency and exit\n");
    printf("  -c           : set cpu clock frequency (Hz)\n");
    printf("  -n <trials>  : set number of base trials\n");
    printf("  -p <package> : run specific package\n");
    printf("  -b <bench>   : run specific benchmark\n");
    printf("  -t <seconds> : set minimum execution time (s)\n");
    printf("  -l           : list available packages\n");
    printf("  -L           : list all available scripts\n");
    printf("  -s <search>  : run all packages/benchmarks matching search string\n");
    printf("  -o <file>    : output file (json)\n");
}

// main function
int main(int argc, char *argv[])
{
    // initialize timing variables
    unsigned int i, j;

    // options
    enum {RUN_ALL,
          RUN_SINGLE_BENCH,
          RUN_SINGLE_PACKAGE,
          RUN_SEARCH,
    } mode = RUN_ALL;
    unsigned int benchmark_id = 0;
    unsigned int package_id = 0;
    int verbose = 1;
    int autoscale = 1;
    int cpu_clock_detect = 1;
    char filename[256] = "benchmark.json";
    char search_string[128];

    // get input options
    int d;
    while((d = getopt(argc,argv,"hvqfec:n:b:p:t:lLs:o:")) != EOF){
        switch (d) {
        case 'h':   usage();        return 0;
        case 'v':   verbose = 1;    break;
        case 'q':   verbose = 0;    break;
        case 'f':
            num_base_trials = 100;
            runtime = 0.5e-3f;
            break;
        case 'e':
            estimate_cpu_clock();
            return 0;
        case 'c':
            cpu_clock = atof(optarg);
            if (cpu_clock < 0) {
                printf("error: cpu clock speed is negative (%f)\n", cpu_clock);
                return -1;
            }
            cpu_clock_detect = 0;
            break;
        case 'n':
            num_base_trials = atoi(optarg);
            autoscale = 0;
            break;
        case 'b':
            benchmark_id = atoi(optarg);
            if (benchmark_id >= NUM_AUTOSCRIPTS) {
                printf("error, cannot run benchmark %u; index exceeded\n", benchmark_id);
                return -1;
            } else {
                mode = RUN_SINGLE_BENCH;
            }
            break;
        case 'p':
            package_id = atoi(optarg);
            if (package_id >= NUM_PACKAGES) {
                printf("error, cannot run package %u; index exceeded\n", package_id);
                return -1;
            } else {
                mode = RUN_SINGLE_PACKAGE;
            }
            break;
        case 't':
            runtime = atof(optarg);
            if (runtime < 1e-3f)     runtime = 1e-3f;
            else if (runtime > 10.f) runtime = 10.0f;
            printf("minimum runtime: %d ms\n", (int) roundf(runtime*1e3));
            break;
        case 'l':
            // list only packages and exit
            for (i=0; i<NUM_PACKAGES; i++)
                printf("%u: %s\n", packages[i].id, packages[i].name);
            return 0;
        case 'L':
            // list packages, scripts and exit
            for (i=0; i<NUM_PACKAGES; i++) {
                printf("%u: %s\n", packages[i].id, packages[i].name);
                for (j=packages[i].index; j<packages[i].num_scripts+packages[i].index; j++)
                    printf("    %-3u: %-22s\n", scripts[j].id, scripts[j].name);
            }
            return 0;
        case 's':
            mode = RUN_SEARCH;
            strncpy(search_string, optarg, 128);
            search_string[127] = '\0';
            break;
        case 'o':
            strncpy(filename,optarg,255);
            filename[255] = '\0';
            break;
        default:
            usage();
            return 0;
        }
    }

    // run empty loop; a bug was found that sometimes the first package run
    // resulted in a longer execution time than what the benchmark really
    // reflected.  This loop prevents that from happening.
    for (i=0; i<1e6; i++) {
        // do nothing
    }

    if (cpu_clock_detect)
        estimate_cpu_clock();

    if (autoscale)
        set_num_trials_from_cpu_speed();

    switch (mode) {
    case RUN_ALL:
        for (i=0; i<NUM_PACKAGES; i++)
            execute_package( &packages[i], verbose );

        //for (i=0; i<NUM_PACKAGES; i++)
        //    print_package_results( &packages[i] );
        break;
    case RUN_SINGLE_BENCH:
        execute_benchmark( &scripts[benchmark_id], verbose );
        //print_benchmark_results( &scripts[benchmark_id] );
        break;
    case RUN_SINGLE_PACKAGE:
        execute_package( &packages[package_id], verbose );
        //print_package_results( &packages[package_id] );
        break;
    case RUN_SEARCH:
        printf("running all packages and benchmarks matching '%s'...\n", search_string);
        for (i=0; i<NUM_PACKAGES; i++) {
            // see if search string matches package name
            if (strstr(packages[i].name, search_string) != NULL) {
                // run the package
                execute_package( &packages[i], verbose );
            }
        }
        printf("running all remaining scripts matching '%s'...\n", search_string);
        for (i=0; i<NUM_AUTOSCRIPTS; i++) {
            // see if search string matches benchmark name
            if (strstr(scripts[i].name, search_string) != NULL && scripts[i].num_trials == 0) {
                // run the benchmark
                execute_benchmark( &scripts[i], verbose );
            }
        }
        break;
    default:
        fprintf(stderr,"invalid mode\n");
        exit(1);
    }

    if (strcmp(filename,"")==0)
        return 0;

    // export results to output .json file; try to open file for writing
    FILE * fid = fopen(filename,"w");
    if (!fid) {
        fprintf(stderr,"error: %s, could not open '%s' for writing\n", __FILE__, filename);
        return -1;
    }

    // print header
    time_t now;
    time(&now);
    char timestamp[80];
    strftime(timestamp,80,"%c",localtime(&now));
    fprintf(fid,"{\n");
    fprintf(fid,"  \"build-info\" : {},\n");
    fprintf(fid,"  \"timestamp\" : \"%s\",\n", timestamp);
    fprintf(fid,"  \"command-line\" : \"");
    for (i=0; i<(unsigned int)argc; i++)
        fprintf(fid," %s", argv[i]);
    fprintf(fid,"\",\n");
    fprintf(fid,"  \"cpu_clock_detect\":%s,\n", cpu_clock_detect ? "true" : "false");
    fprintf(fid,"  \"search string\"       : \"%s\",\n", mode == RUN_SEARCH ? search_string : "");
    fprintf(fid,"  \"runtime\"             : %12.8f,\n", runtime);
    fprintf(fid,"  \"cpu_clock\"           : %e,\n", cpu_clock);
    fprintf(fid,"  \"cpu_clock_determined\": \"%s\",\n", cpu_clock_detect ? "estimated" : "specified");
    fprintf(fid,"  \"num_trials\"          : %lu,\n", num_base_trials);
    fprintf(fid,"  \"benchmarks\"          : [\n");
    for (i=0; i<NUM_AUTOSCRIPTS; i++) {
        fprintf(fid,"    {\"id\":%5u, \"trials\":%12u, \"extime\":%12.4e, \"rate\":%12.4e, \"cycles_per_trial\":%12.4e, \"attempts\":%2u, \"name\":\"%s\"}%s\n",
                scripts[i].id,
                scripts[i].num_trials,
                scripts[i].extime,
                scripts[i].rate,
                scripts[i].cycles_per_trial,
                scripts[i].num_attempts,
                scripts[i].name,
                i==NUM_AUTOSCRIPTS-1 ? "" : ",");
    }
    fprintf(fid,"  ]\n");
    fprintf(fid,"}\n");
    fclose(fid);
    if (verbose)
        printf("output JSON results written to %s\n", filename);

    return 0;
}

// run basic benchmark to estimate CPU clock frequency
void estimate_cpu_clock(void)
{
    printf("  estimating cpu clock frequency...\n");
    unsigned long int i, n = 1<<4;
    struct rusage start, finish;
    double extime;
    
    // run trials until execution time threshold is exceeded
    do {
        // trials
        n <<= 1;

        // NOTE: Smart compilers will realize that this loop doesn't really do
        //       anything, so they won't actually compute anything. We need to
        //       actually do something interesting here to trick the compiler
        //       into actually crunching these numbers, and then later display
        //       the results, even if they're meaningless
        unsigned int k = 366001;    // large prime number
        unsigned int g = 184903;    // another large prime number
        unsigned int s = 1;
        getrusage(RUSAGE_SELF, &start);
        for (i=0; i<n; i++) {
            // perform mindless task
            s = (s*k) % g;
        }
        getrusage(RUSAGE_SELF, &finish);

        extime = calculate_execution_time(start, finish);

        // print results to screen
        // NOTE: it is necessary to do something with the variable 's' so that
        //       the compiler will actually run the above loop
        printf("%12lu trials in %8.3f ms, s = %6u\n", n, extime*1e3, s);
    } while (extime < 0.5 && n < (1<<28));

    // estimate cpu clock frequency
    cpu_clock = 9.5 * n / extime;

    printf("  performed %ld trials in %5.1f ms\n", n, extime * 1e3);
    
    float clock_format = cpu_clock;
    char clock_units = convert_units(&clock_format);
    printf("  estimated clock speed: %7.3f %cHz\n", clock_format, clock_units);
}

void set_num_trials_from_cpu_speed(void)
{
    unsigned long int min_trials = 256;
    num_base_trials = (unsigned long int) ( cpu_clock / 20e3 );
    num_base_trials = (num_base_trials < min_trials) ? min_trials : num_base_trials;

    printf("  setting number of base trials to %ld\n", num_base_trials);
}

void execute_benchmark(benchmark_t* _benchmark, int _verbose)
{
    unsigned long int n = num_base_trials;
    struct rusage start, finish;

    unsigned int num_attempts = 0;
    unsigned long int num_trials;
    do {
        // increment number of attempts
        num_attempts++;

        // set number of trials and run benchmark
        num_trials = n;
        _benchmark->api(&start, &finish, &num_trials);
        _benchmark->extime = calculate_execution_time(start, finish);

        // check exit criteria
        if (_benchmark->extime >= runtime) {
            break;
        } else if (num_attempts == 30) {
            fprintf(stderr,"warning: benchmark could not execute over minimum run time\n");
            break;
        } else {
            // increase number of trials
            n *= 2;
        }
    } while (1);

    _benchmark->num_trials = num_trials;
    _benchmark->num_attempts = num_attempts;
    _benchmark->rate = _benchmark->extime==0 ? 0 : (float)(_benchmark->num_trials) / _benchmark->extime;
    _benchmark->cycles_per_trial = _benchmark->extime==0 ? 0 : cpu_clock / (_benchmark->rate);

    if (_verbose)
        print_benchmark_results(_benchmark);
}

void execute_package(package_t* _package, int _verbose)
{
    if (_verbose)
        printf("%u: %s\n", _package->id, _package->name);
    
    unsigned int i;
    for (i=0; i<_package->num_scripts; i++) {
        execute_benchmark( &scripts[ i + _package->index ], _verbose );
    }
}

// convert raw value into metric units,
//   example: "0.01397s" -> "13.97 ms"
char convert_units(float * _v)
{
    char unit;
    if (*_v < 1e-9)     {   (*_v) *= 1e12;  unit = 'p';}
    else if (*_v < 1e-6){   (*_v) *= 1e9;   unit = 'n';}
    else if (*_v < 1e-3){   (*_v) *= 1e6;   unit = 'u';}
    else if (*_v < 1e+0){   (*_v) *= 1e3;   unit = 'm';}
    else if (*_v < 1e3) {   (*_v) *= 1e+0;  unit = ' ';}
    else if (*_v < 1e6) {   (*_v) *= 1e-3;  unit = 'k';}
    else if (*_v < 1e9) {   (*_v) *= 1e-6;  unit = 'M';}
    else if (*_v < 1e12){   (*_v) *= 1e-9;  unit = 'G';}
    else                {   (*_v) *= 1e-12; unit = 'T';}

    return unit;
}

void print_benchmark_results(benchmark_t* _b)
{
    // format trials (iterations)
    float trials_format = (float)(_b->num_trials);
    char trials_units = convert_units(&trials_format);

    // format time (seconds)
    float extime_format = _b->extime;
    char extime_units = convert_units(&extime_format);

    // format rate (trials/second)
    float rate_format = _b->rate;
    char rate_units = convert_units(&rate_format);

    // format processor efficiency (cycles/trial)
    float cycles_format = _b->cycles_per_trial;
    char cycles_units = convert_units(&cycles_format);

    printf("  %-3u: [%2u] %-30s: %6.2f %c trials / %6.2f %cs (%6.2f %c t/s, %6.2f %c c/t)\n",
        _b->id, _b->num_attempts, _b->name,
        trials_format, trials_units,
        extime_format, extime_units,
        rate_format, rate_units,
        cycles_format, cycles_units);
}

void print_package_results(package_t* _package)
{
    unsigned int i;
    printf("%u: %s:\n", _package->id, _package->name);
    for (i=_package->index; i<(_package->index+_package->num_scripts); i++)
        print_benchmark_results( &scripts[i] );

    printf("\n");
}

double calculate_execution_time(struct rusage _start, struct rusage _finish)
{
    return _finish.ru_utime.tv_sec - _start.ru_utime.tv_sec
        + 1e-6*(_finish.ru_utime.tv_usec - _start.ru_utime.tv_usec)
        + _finish.ru_stime.tv_sec - _start.ru_stime.tv_sec
        + 1e-6*(_finish.ru_stime.tv_usec - _start.ru_stime.tv_usec);
}
