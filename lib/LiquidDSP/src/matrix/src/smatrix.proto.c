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

// 
// sparse matrices
//

// sparse matrix structure
// example: the following floating-point sparse matrix is represented
//          with the corresponding values:
//    [ 0   0   0   0   0 ]
//    [ 0 2.3   0   0   0 ]
//    [ 0   0   0   0 1.2 ]
//    [ 0   0   0   0   0 ]
//    [ 0 3.4   0 4.4   0 ]
//    [ 0   0   0   0   0 ]
//
//  M (rows)        :   6
//  N (cols)        :   5
//  mlist           :   { {}, {1},   {4}, {},  {1,3}, {} }
//  nlist           :   { {}, {1,4}, {},  {4}, {2} }
//  mvals           :   { {}, {2.3}, {1.2}, {}, {3.4,4.4}, {} }
//  nvals           :   { {}, {2.3, 3.4}, {}, {4.4}, {1.2} }
//  num_mlist       :   { 0, 1, 1, 0, 2, 0 }
//  num_nlist       :   { 0, 2, 0, 1, 1 }
//  max_num_mlist   :   2
//  max_num_nlist   :   2
//
// NOTE: while this particular example does not show a particular
//       improvement in memory use, such a case can be made for
//       extremely large matrices which only have a few non-zero
//       entries.
//
struct SMATRIX(_s) {
    unsigned int M;                 // number of rows
    unsigned int N;                 // number of columns
    unsigned short int ** mlist;    // list of non-zero col indices in each row
    unsigned short int ** nlist;    // list of non-zero row indices in each col
    T ** mvals;                     // list of non-zero values in each row
    T ** nvals;                     // list of non-zero values in each col
    unsigned int * num_mlist;       // weight of each row, m
    unsigned int * num_nlist;       // weight of each row, n
    unsigned int max_num_mlist;     // maximum of num_mlist
    unsigned int max_num_nlist;     // maximum of num_nlist
};

int SMATRIX(_reset_max_mlist)(SMATRIX() _q);
int SMATRIX(_reset_max_nlist)(SMATRIX() _q);

// create _m x _n matrix, initialized with zeros
SMATRIX() SMATRIX(_create)(unsigned int _m,
                           unsigned int _n)
{
    if (_m==0 || _n==0)
        return liquid_error_config("smatrix%s_create(), dimensions must be greater than zero",EXTENSION);

    // create object and allocate memory
    SMATRIX() q = (SMATRIX()) malloc(sizeof(struct SMATRIX(_s)));
    q->M = _m;
    q->N = _n;

    unsigned int i;
    unsigned int j;

    // initialize size of each pointer list
    q->num_mlist = (unsigned int*)malloc( q->M*sizeof(unsigned int) );
    q->num_nlist = (unsigned int*)malloc( q->N*sizeof(unsigned int) );
    for (i=0; i<q->M; i++) q->num_mlist[i] = 0;
    for (j=0; j<q->N; j++) q->num_nlist[j] = 0;

    // initialize lists
    q->mlist = (unsigned short int **) malloc( q->M*sizeof(unsigned short int *) );
    q->nlist = (unsigned short int **) malloc( q->N*sizeof(unsigned short int *) );
    for (i=0; i<q->M; i++)
        q->mlist[i] = (unsigned short int *) malloc( q->num_mlist[i]*sizeof(unsigned short int) );
    for (j=0; j<q->N; j++)
        q->nlist[j] = (unsigned short int *) malloc( q->num_nlist[j]*sizeof(unsigned short int) );

    // initialize values
    q->mvals = (T **) malloc( q->M*sizeof(T*) );
    q->nvals = (T **) malloc( q->N*sizeof(T*) );
    for (i=0; i<q->M; i++)
        q->mvals[i] = (T *) malloc( q->num_mlist[i]*sizeof(T) );
    for (j=0; j<q->N; j++)
        q->nvals[j] = (T *) malloc( q->num_nlist[j]*sizeof(T) );

    // set maximum list size
    q->max_num_mlist = 0;
    q->max_num_nlist = 0;

    // return main object
    return q;
}

// create _m x _n matrix, initialized on array
SMATRIX() SMATRIX(_create_array)(T *          _v,
                                 unsigned int _m,
                                 unsigned int _n)
{
    // create object and allocate memory
    SMATRIX() q = SMATRIX(_create)(_m,_n);

    // initialize elements
    unsigned int i;
    unsigned int j;
    for (i=0; i<_m; i++) {
        for (j=0; j<_n; j++) {
            if (_v[i*_n + j] != 0)
                SMATRIX(_set)(q,i,j,_v[i*_n + j]);
        }
    }

    // return main object
    return q;
}

// destroy object
int SMATRIX(_destroy)(SMATRIX() _q)
{
    unsigned int i;
    unsigned int j;

    // free internal memory
    free(_q->num_mlist);
    free(_q->num_nlist);

    // free lists
    for (i=0; i<_q->M; i++) free(_q->mlist[i]);
    for (j=0; j<_q->N; j++) free(_q->nlist[j]);
    free(_q->mlist);
    free(_q->nlist);

    // free values
    for (i=0; i<_q->M; i++) free(_q->mvals[i]);
    for (j=0; j<_q->N; j++) free(_q->nvals[j]);
    free(_q->mvals);
    free(_q->nvals);

    // free main object memory
    free(_q);
    return LIQUID_OK;
}

// print compact form
int SMATRIX(_print)(SMATRIX() _q)
{
    printf("dims : %u %u\n", _q->M, _q->N);
    printf("max  : %u %u\n", _q->max_num_mlist, _q->max_num_nlist);
    unsigned int i;
    unsigned int j;
    printf("rows :");
    for (i=0; i<_q->M; i++) printf(" %u", _q->num_mlist[i]);
    printf("\n");
    printf("cols :");
    for (j=0; j<_q->N; j++) printf(" %u", _q->num_nlist[j]);
    printf("\n");

    // print mlist
    printf("row indices:\n");
    for (i=0; i<_q->M; i++) {
        if (_q->num_mlist[i] == 0)
            continue;
        printf("  %3u :", i);
        for (j=0; j<_q->num_mlist[i]; j++)
            printf(" %u", _q->mlist[i][j]);
        printf("\n");
    }

    // print nlist
    printf("column indices:\n");
    for (j=0; j<_q->N; j++) {
        if (_q->num_nlist[j] == 0)
            continue;
        printf("  %3u :", j);
        for (i=0; i<_q->num_nlist[j]; i++)
            printf(" %u", _q->nlist[j][i]);
        printf("\n");
    }
#if SMATRIX_FLOAT
    // print mvals
    printf("row values:\n");
    for (i=0; i<_q->M; i++) {
        printf("  %3u :", i);
        for (j=0; j<_q->num_mlist[i]; j++)
            printf(" %6.2f", _q->mvals[i][j]);
        printf("\n");
    }

    // print nvals
    printf("column values:\n");
    for (j=0; j<_q->N; j++) {
        printf("  %3u :", j);
        for (i=0; i<_q->num_nlist[j]; i++)
            printf(" %6.2f", _q->nvals[j][i]);
        printf("\n");
    }
#endif
    return LIQUID_OK;
}

// print expanded form
int SMATRIX(_print_expanded)(SMATRIX() _q)
{
    unsigned int i;
    unsigned int j;
    unsigned int t;

    // print in expanded 'regular' form
    for (i=0; i<_q->M; i++) {
        // reset counter
        t = 0;
        for (j=0; j<_q->N; j++) {
            if (t == _q->num_mlist[i]) {
                PRINTVAL_ZERO();
            } else if (_q->mlist[i][t] == j) {
                PRINTVAL(_q->mvals[i][t]);
                t++;
            } else {
                PRINTVAL_ZERO();
            }
        }
        printf("\n");
    }
    return LIQUID_OK;
}

// get matrix dimensions
int SMATRIX(_size)(SMATRIX()      _q,
                   unsigned int * _m,
                   unsigned int * _n)
{
    *_m = _q->M;
    *_n = _q->N;
    return LIQUID_OK;
}

// zero all values, retaining memory allocation
int SMATRIX(_clear)(SMATRIX() _q)
{
    unsigned int i;
    unsigned int j;
    
    // clear row entries
    for (i=0; i<_q->M; i++) {
        for (j=0; j<_q->num_mlist[i]; j++) {
            _q->mvals[i][j] = 0;
        }
    }

    // clear column entries
    for (j=0; j<_q->N; j++) {
        for (i=0; i<_q->num_nlist[j]; i++) {
            _q->nvals[j][i] = 0;
        }
    }
    return LIQUID_OK;
}

// zero all values, clearing memory
int SMATRIX(_reset)(SMATRIX() _q)
{
    unsigned int i;
    unsigned int j;
    for (i=0; i<_q->M; i++) _q->num_mlist[i] = 0;
    for (j=0; j<_q->N; j++) _q->num_nlist[j] = 0;

    _q->max_num_mlist = 0;
    _q->max_num_nlist = 0;
    return LIQUID_OK;
}

// determine if element is set
int SMATRIX(_isset)(SMATRIX()    _q,
                    unsigned int _m,
                    unsigned int _n)
{
    // validate input
    if (_m >= _q->M || _n >= _q->N) {
        liquid_error(LIQUID_EIRANGE,"SMATRIX(_isset)(%u,%u), index exceeds matrix dimension (%u,%u)",_m, _n, _q->M, _q->N);
        return 0;
    }

    unsigned int j;
    for (j=0; j<_q->num_mlist[_m]; j++) {
        if (_q->mlist[_m][j] == _n)
            return 1;
    }
    return 0;
}

// insert element at index
int SMATRIX(_insert)(SMATRIX()    _q,
                     unsigned int _m,
                     unsigned int _n,
                     T            _v)
{
    // validate input
    if (_m >= _q->M || _n >= _q->N)
        return liquid_error(LIQUID_EIRANGE,"SMATRIX(_insert)(%u,%u), index exceeds matrix dimension (%u,%u)",_m, _n, _q->M, _q->N);

    // check to see if element is already set
    if (SMATRIX(_isset)(_q,_m,_n)) {
        // simply set the value and return
        printf("SMATRIX(_insert), value already set...\n");
        return SMATRIX(_set)(_q, _m, _n, _v);
    }

    // increment list sizes
    _q->num_mlist[_m]++;
    _q->num_nlist[_n]++;

    // reallocate indices lists at this index
    _q->mlist[_m] = (unsigned short int*) realloc(_q->mlist[_m], _q->num_mlist[_m]*sizeof(unsigned short int));
    _q->nlist[_n] = (unsigned short int*) realloc(_q->nlist[_n], _q->num_nlist[_n]*sizeof(unsigned short int));
    
    // reallocate values lists at this index
    _q->mvals[_m] = (T*) realloc(_q->mvals[_m], _q->num_mlist[_m]*sizeof(T));
    _q->nvals[_n] = (T*) realloc(_q->nvals[_n], _q->num_nlist[_n]*sizeof(T));

    // find index within list to insert new value
    unsigned int mindex = smatrix_indexsearch(_q->mlist[_m], _q->num_mlist[_m]-1, _n);
    unsigned int nindex = smatrix_indexsearch(_q->nlist[_n], _q->num_nlist[_n]-1, _m);
    //printf("inserting value (n=%3u) at m=%3u, index=%3u\n", _n, _m, mindex);
    //printf("inserting value (m=%3u) at n=%3u, index=%3u\n", _m, _n, nindex);

    // insert indices to appropriate place in list
    memmove(&_q->mlist[_m][mindex+1], &_q->mlist[_m][mindex], (_q->num_mlist[_m]-mindex-1)*sizeof(unsigned short int));
    memmove(&_q->nlist[_n][nindex+1], &_q->nlist[_n][nindex], (_q->num_nlist[_n]-nindex-1)*sizeof(unsigned short int));
    _q->mlist[_m][mindex] = _n;
    _q->nlist[_n][nindex] = _m;

    // insert values to appropriate place in list
    memmove(&_q->mvals[_m][mindex+1], &_q->mvals[_m][mindex], (_q->num_mlist[_m]-mindex-1)*sizeof(T));
    memmove(&_q->nvals[_n][nindex+1], &_q->nvals[_n][nindex], (_q->num_nlist[_n]-nindex-1)*sizeof(T));
    _q->mvals[_m][mindex] = _v;
    _q->nvals[_n][nindex] = _v;

    // update maximum
    if (_q->num_mlist[_m] > _q->max_num_mlist) _q->max_num_mlist = _q->num_mlist[_m];
    if (_q->num_nlist[_n] > _q->max_num_nlist) _q->max_num_nlist = _q->num_nlist[_n];
    return LIQUID_OK;
}

// delete element at index
int SMATRIX(_delete)(SMATRIX()    _q,
                     unsigned int _m,
                     unsigned int _n)
{
    // validate input
    if (_m > _q->M || _n > _q->N)
        return liquid_error(LIQUID_EIRANGE,"SMATRIX(_delete)(%u,%u), index exceeds matrix dimension (%u,%u)",_m, _n, _q->M, _q->N);

    // check to see if element is already not set
    if (!SMATRIX(_isset)(_q,_m,_n))
        return LIQUID_OK;

    // remove value from mlist (shift left)
    unsigned int i;
    unsigned int j;
    unsigned int t=0;
    for (j=0; j<_q->num_mlist[_m]; j++) {
        if (_q->mlist[_m][j] == _n)
            t = j;
    }
    for (j=t; j<_q->num_mlist[_m]-1; j++)
        _q->mlist[_m][j] = _q->mlist[_m][j+1];

    // remove value from nlist (shift left)
    t = 0;
    for (i=0; i<_q->num_nlist[_n]; i++) {
        if (_q->nlist[_n][i] == _m)
            t = i;
    }
    for (i=t; i<_q->num_nlist[_n]-1; i++)
        _q->nlist[_n][i] = _q->nlist[_n][i+1];

    // reduce sizes
    _q->num_mlist[_m]--;
    _q->num_nlist[_n]--;

    // reallocate
    _q->mlist[_m] = (unsigned short int*) realloc(_q->mlist[_m], _q->num_mlist[_m]*sizeof(unsigned short int));
    _q->nlist[_n] = (unsigned short int*) realloc(_q->nlist[_n], _q->num_nlist[_n]*sizeof(unsigned short int));

    // reset maxima
    if (_q->max_num_mlist == _q->num_mlist[_m]+1)
        SMATRIX(_reset_max_mlist)(_q);

    if (_q->max_num_nlist == _q->num_nlist[_n]+1)
        SMATRIX(_reset_max_nlist)(_q);
    return LIQUID_OK;
}

// set element value at index
int SMATRIX(_set)(SMATRIX()    _q,
                  unsigned int _m,
                  unsigned int _n,
                  T            _v)
{
    // validate input
    if (_m >= _q->M || _n >= _q->N)
        return liquid_error(LIQUID_EIRANGE,"SMATRIX(_set)(%u,%u), index exceeds matrix dimension (%u,%u)",_m, _n, _q->M, _q->N);

    // insert new element if not already allocated
    if (!SMATRIX(_isset)(_q,_m,_n))
        return SMATRIX(_insert)(_q,_m,_n,_v);

    // set value
    unsigned int i;
    unsigned int j;
    for (j=0; j<_q->num_mlist[_m]; j++) {
        if (_q->mlist[_m][j] == _n)
            _q->mvals[_m][j] = _v;
    }

    for (i=0; i<_q->num_nlist[_n]; i++) {
        if (_q->nlist[_n][i] == _m)
            _q->nvals[_n][i] = _v;
    }
    return LIQUID_OK;
}


// get element value at index (return zero if not set)
T SMATRIX(_get)(SMATRIX()    _q,
                unsigned int _m,
                unsigned int _n)
{
    // validate input
    if (_m >= _q->M || _n >= _q->N) {
        liquid_error(LIQUID_EIRANGE,"SMATRIX(_get)(%u,%u), index exceeds matrix dimension (%u,%u)",_m, _n, _q->M, _q->N);
        return 0;
    }

    unsigned int j;
    for (j=0; j<_q->num_mlist[_m]; j++) {
        if (_q->mlist[_m][j] == _n)
            return _q->mvals[_m][j];
    }
    return 0;
}

// initialize to identity matrix
int SMATRIX(_eye)(SMATRIX() _q)
{
    // reset all elements
    SMATRIX(_reset)(_q);

    // set values along diagonal
    unsigned int i;
    unsigned int dmin = _q->M < _q->N ? _q->M : _q->N;
    for (i=0; i<dmin; i++)
        SMATRIX(_set)(_q, i, i, 1);
    return LIQUID_OK;
}

// multiply two sparse matrices
int SMATRIX(_mul)(SMATRIX() _a,
                  SMATRIX() _b,
                  SMATRIX() _c)
{
    // validate input
    if (_c->M != _a->M || _c->N != _b->N || _a->N != _b->M)
        return liquid_error(LIQUID_EIRANGE,"SMATRIX(_mul)(), invalid dimensions");

    // clear output matrix (retain memory allocation)
    SMATRIX(_clear)(_c);

    unsigned int r; // output row
    unsigned int c; // output column
    
    unsigned int i;
    unsigned int j;

    for (r=0; r<_c->M; r++) {
        
        // find number of non-zero entries in row 'r' of matrix '_a'
        unsigned int nnz_a_row = _a->num_mlist[r];

        // if this number is zero, there will not be any non-zero
        // entries in the corresponding row of the output matrix '_c'
        if (nnz_a_row == 0)
            continue;
        
        for (c=0; c<_c->N; c++) {

            // find number of non-zero entries in column 'c' of matrix '_b'
            unsigned int nnz_b_col = _b->num_nlist[c];

            T p = 0;
            int set_value = 0;

            // find common elements between non-zero elements in
            // row 'r' of matrix '_a' and col 'c' of matrix '_b'
            i=0;    // reset array index for rows of '_a'
            j=0;    // reset array index for cols of '_b'
            while (i < nnz_a_row && j < nnz_b_col) {
                // 
                unsigned int ca = _a->mlist[r][i];
                unsigned int rb = _b->nlist[c][j];
                if (ca == rb) {
                    // match found between _a[r,ca] and _b[rb,c]
                    p += _a->mvals[r][i] * _b->nvals[c][j];
                    set_value = 1;
                    i++;
                    j++;
                } else if (ca < rb)
                    i++;    // increment index for '_a'
                else
                    j++;    // increment index for '_b'

            }

            // set value if any multiplications have been made
            if (set_value) {
#if SMATRIX_BOOL
                // set result modulo 2
                SMATRIX(_set)(_c, r, c, p % 2);
#else
                SMATRIX(_set)(_c, r, c, p);
#endif
            }
        }
    }
    return LIQUID_OK;
}

// multiply by vector
//  _q  :   sparse matrix
//  _x  :   input vector [size: _n x 1]
//  _y  :   output vector [size: _m x 1]
int SMATRIX(_vmul)(SMATRIX() _q,
                   T *       _x,
                   T *       _y)
{
    unsigned int i;
    unsigned int j;
    
    // initialize to zero
    for (i=0; i<_q->M; i++)
        _y[i] = 0;

    for (i=0; i<_q->M; i++) {

        // running total
        T p = 0;

        // only compute multiplications on non-zero entries
        for (j=0; j<_q->num_mlist[i]; j++)
            p += _q->mvals[i][j] * _x[ _q->mlist[i][j] ];

        // set output value appropriately
#if SMATRIX_BOOL
        _y[i] = p % 2;
#else
        _y[i] = p;
#endif
    }
    return LIQUID_OK;
}


// 
// internal methods
//

// find maximum mlist length
int SMATRIX(_reset_max_mlist)(SMATRIX() _q)
{
    unsigned int i;

    _q->max_num_mlist = 0;
    for (i=0; i<_q->M; i++) {
        if (_q->num_mlist[i] > _q->max_num_mlist)
            _q->max_num_mlist = _q->num_mlist[i];
    }
    return LIQUID_OK;
}

// find maximum nlist length
int SMATRIX(_reset_max_nlist)(SMATRIX() _q)
{
    unsigned int j;

    _q->max_num_nlist = 0;
    for (j=0; j<_q->N; j++) {
        if (_q->num_nlist[j] > _q->max_num_nlist)
            _q->max_num_nlist = _q->num_nlist[j];
    }
    return LIQUID_OK;
}

