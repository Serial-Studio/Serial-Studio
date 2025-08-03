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

// 
// gasearch.c
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "liquid.internal.h"

#define LIQUID_GA_SEARCH_MAX_POPULATION_SIZE (1024)
#define LIQUID_GA_SEARCH_MAX_CHROMOSOME_SIZE (32)

#define LIQUID_DEBUG_GA_SEARCH 0

// Create a simple gasearch object; parameters are specified internally
//  _utility            :   chromosome fitness utility function
//  _userdata           :   user data, void pointer passed to _utility() callback
//  _parent             :   initial population parent chromosome, governs precision, etc.
//  _minmax             :   search direction
gasearch gasearch_create(gasearch_utility _utility,
                         void *           _userdata,
                         chromosome       _parent,
                         int              _minmax)
{
    return gasearch_create_advanced(_utility,_userdata,_parent,_minmax,16,0.02f);
}

// Create a gasearch object, specifying search parameters
//  _utility            :   chromosome fitness utility function
//  _userdata           :   user data, void pointer passed to _utility() callback
//  _parent             :   initial population parent chromosome, governs precision, etc.
//  _minmax             :   search direction
//  _population_size    :   number of chromosomes in population
//  _mutation_rate      :   probability of mutating chromosomes
gasearch gasearch_create_advanced(gasearch_utility _utility,
                                  void *           _userdata,
                                  chromosome       _parent,
                                  int              _minmax,
                                  unsigned int     _population_size,
                                  float            _mutation_rate)
{
    // validate input
    if (_utility == NULL)
        return liquid_error_config("gasearch_create(), utility function cannot be NULL")
    if (_parent == NULL)
        return liquid_error_config("gasearch_create(), parent cannot be NULL")
    if (_population_size < 2)
        return liquid_error_config("gasearch_create(), population size exceeds minimum");
    if (_population_size > LIQUID_GA_SEARCH_MAX_POPULATION_SIZE)
        return liquid_error_config("gasearch_create(), population size exceeds maximum (%u)",LIQUID_GA_SEARCH_MAX_POPULATION_SIZE);
    if (_mutation_rate < 0.0f || _mutation_rate > 1.0f)
        return liquid_error_config("gasearch_create(), mutation rate must be in [0,1]");

    // create object and initialize values
    gasearch ga = (gasearch) malloc( sizeof(struct gasearch_s) );
    ga->userdata        = _userdata;
    ga->num_parameters  = _parent->num_traits;
    ga->population_size = _population_size;
    ga->mutation_rate   = _mutation_rate;
    ga->get_utility     = _utility;
    ga->minimize        = ( _minmax==LIQUID_OPTIM_MINIMIZE ) ? 1 : 0;

    ga->bits_per_chromosome = _parent->num_bits;

    // initialize selection size be be 25% of population, minimum of 2
    ga->selection_size = ( ga->population_size >> 2 ) < 2 ? 2 : ga->population_size >> 2;

    // allocate internal arrays
    ga->population = (chromosome*) malloc( sizeof(chromosome)*(ga->population_size) );
    ga->utility = (float*) calloc( sizeof(float), ga->population_size );

    // create optimum chromosome (clone)
    ga->c = chromosome_create_clone(_parent);

    //printf("num_parameters: %d\n", ga->num_parameters);
    //printf("population_size: %d\n", ga->population_size);
    //printf("\nbits_per_chromosome: %d\n", ga->bits_per_chromosome);

    // create population
    unsigned int i;
    for (i=0; i<ga->population_size; i++)
        ga->population[i] = chromosome_create_clone(_parent);

    // initialize population to random, preserving first chromosome
    for (i=1; i<ga->population_size; i++)
        chromosome_init_random( ga->population[i] );

    // evaluate population
    gasearch_evaluate(ga);

    // rank chromosomes
    gasearch_rank(ga);

    // set global utility optimum
    ga->utility_opt = ga->utility[0];

    // return object
    return ga;
}

// destroy a gasearch object
int gasearch_destroy(gasearch _g)
{
    unsigned int i;
    for (i=0; i<_g->population_size; i++)
        chromosome_destroy( _g->population[i] );
    free(_g->population);

    // destroy optimum chromosome
    chromosome_destroy(_g->c);

    free(_g->utility);
    free(_g);
    return LIQUID_OK;
}

// print search parameter internals
int gasearch_print(gasearch _g)
{
    printf("<liquid.gasearch");
    printf(", traits=%u", _g->num_parameters);
    printf(", bits=%u", _g->bits_per_chromosome);
    printf(", population=%u", _g->population_size);
    printf(", selection=%u", _g->selection_size);
    printf(", mutation=%g", _g->mutation_rate);
    printf(">\n");
    return LIQUID_OK;
}

// set population/selection size
//  _q                  :   ga search object
//  _population_size    :   new population size (number of chromosomes)
//  _selection_size     :   selection size (number of parents for new generation)
int gasearch_set_population_size(gasearch     _g,
                                 unsigned int _population_size,
                                 unsigned int _selection_size)
{
    // validate input
    if (_population_size < 2)
        return liquid_error(LIQUID_EICONFIG,"gasearch_set_population_size(), population must be at least 2");
    if (_population_size > LIQUID_GA_SEARCH_MAX_POPULATION_SIZE)
        return liquid_error(LIQUID_EICONFIG,"gasearch_set_population_size(), population exceeds maximum (%u)",LIQUID_GA_SEARCH_MAX_POPULATION_SIZE);
    if (_selection_size == 0)
        return liquid_error(LIQUID_EICONFIG,"gasearch_set_population_size(), selection size must be greater than zero");
    if (_selection_size >= _population_size)
        return liquid_error(LIQUID_EICONFIG,"gasearch_set_population_size(), selection size must be less than population");

    // re-size arrays
    _g->population = (chromosome*) realloc( _g->population, _population_size*sizeof(chromosome) );
    _g->utility = (float*) realloc( _g->utility, _population_size*sizeof(float) );

    // initialize new chromosomes (copies)
    if (_population_size > _g->population_size) {

        unsigned int i;
        unsigned int k = _g->population_size-1; // least optimal

        for (i=_g->population_size; i<_population_size; i++) {
            // clone chromosome, copying internal values
            _g->population[i] = chromosome_create_clone(_g->population[k]);

            // copy utility
            _g->utility[i] = _g->utility[k];
        }
    }

    // set internal variables
    _g->population_size = _population_size;
    _g->selection_size  = _selection_size;
    return LIQUID_OK;
}

// set mutation rate
int gasearch_set_mutation_rate(gasearch _g,
                               float    _mutation_rate)
{
    if (_mutation_rate < 0.0f || _mutation_rate > 1.0f)
        return liquid_error(LIQUID_EIRANGE,"gasearch_set_mutation_rate(), mutation rate must be in [0,1]");

    _g->mutation_rate = _mutation_rate;
    return LIQUID_OK;
}

// Execute the search
//  _g              :   ga search object
//  _max_iterations :   maximum number of iterations to run before bailing
//  _tarutility :   target utility
float gasearch_run(gasearch     _g,
                   unsigned int _max_iterations,
                   float        _tarutility)
{
    unsigned int i=0;
    do {
        i++;
        gasearch_evolve(_g);
    } while (
        optim_threshold_switch(_g->utility[0], _tarutility, _g->minimize) &&
        i < _max_iterations);

    // return optimum utility
    return _g->utility_opt;
}

// iterate over one evolution of the search algorithm
int gasearch_evolve(gasearch _g)
{
    // Inject random chromosome at end
    chromosome_init_random(_g->population[_g->population_size-1]);

    // Crossover
    gasearch_crossover(_g);

    // Mutation
    gasearch_mutate(_g);

    // Evaluation
    gasearch_evaluate(_g);

    // Rank
    gasearch_rank(_g);

    if ( optim_threshold_switch(_g->utility_opt,
                                _g->utility[0],
                                _g->minimize) )
    {
        // update optimum
        _g->utility_opt = _g->utility[0];

        // copy optimum chromosome
        chromosome_copy(_g->population[0], _g->c);

#if LIQUID_DEBUG_GA_SEARCH
        printf("  utility: %0.2E", _g->utility_opt);
        chromosome_printf(_g->c);
#endif
    }
    return LIQUID_OK;
}

// get optimal chromosome
//  _g              :   ga search object
//  _c              :   output optimal chromosome
//  _utility_opt    :   fitness of _c
int gasearch_getopt(gasearch    _g,
                     chromosome _c,
                     float *    _utility_opt)
{
    // copy optimum chromosome
    chromosome_copy(_g->c, _c);

    // copy optimum utility
    *_utility_opt = _g->utility_opt;
    return LIQUID_OK;
}

// evaluate fitness of entire population
int gasearch_evaluate(gasearch _g)
{
    unsigned int i;
    for (i=0; i<_g->population_size; i++)
        _g->utility[i] = _g->get_utility(_g->userdata, _g->population[i]);
    return LIQUID_OK;
}

// crossover population
int gasearch_crossover(gasearch _g)
{
    chromosome p1, p2;      // parental chromosomes
    chromosome c;           // child chromosome
    unsigned int threshold;

    unsigned int i;
    for (i=_g->selection_size; i<_g->population_size; i++) {
        // ensure fittest member is used at least once as parent
        p1 = (i==_g->selection_size) ? _g->population[0] : _g->population[rand() % _g->selection_size];
        p2 = _g->population[rand() % _g->selection_size];
        threshold = rand() % _g->bits_per_chromosome;

        c = _g->population[i];

        //printf("  gasearch_crossover, p1: %d, p2: %d, c: %d\n", p1, p2, c);
        chromosome_crossover(p1, p2, c, threshold);
    }
    return LIQUID_OK;
}

// mutate population
int gasearch_mutate(gasearch _g)
{
    unsigned int i;
    unsigned int index;

    // mutate all but first (best) chromosome
    //for (i=_g->selection_size; i<_g->population_size; i++) {
    for (i=1; i<_g->population_size; i++) {
        // generate random number and mutate if within mutation_rate range
        unsigned int num_mutations = 0;
        // force at least one mutation (otherwise nothing has changed)
        while ( randf() < _g->mutation_rate || num_mutations == 0) {
            // generate random mutation index
            index = rand() % _g->bits_per_chromosome;

            // mutate chromosome at index
            chromosome_mutate( _g->population[i], index );

            //
            num_mutations++;

            if (num_mutations == _g->bits_per_chromosome)
                break;
        }
    }
    return LIQUID_OK;
}

// rank population by fitness
int gasearch_rank(gasearch _g)
{
    unsigned int i, j;
    float u_tmp;        // temporary utility placeholder
    chromosome c_tmp;   // temporary chromosome placeholder (pointer)

    for (i=0; i<_g->population_size; i++) {
        for (j=_g->population_size-1; j>i; j--) {
            if ( optim_threshold_switch(_g->utility[j], _g->utility[j-1], !(_g->minimize)) ) {
                // swap chromosome pointers
                c_tmp = _g->population[j];
                _g->population[j] = _g->population[j-1];
                _g->population[j-1] = c_tmp;

                // swap utility values
                u_tmp = _g->utility[j];
                _g->utility[j] = _g->utility[j-1];
                _g->utility[j-1] = u_tmp;
            }
        }
    }
    return LIQUID_OK;
}

