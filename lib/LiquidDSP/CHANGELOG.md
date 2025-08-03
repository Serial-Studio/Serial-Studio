# Changelog

## 1.7.0 - 2025-02-01

Version 1.7.0 includes support for the [CMake](https://cmake.org) build
system. While the existing autotools environment will be supported for a time,
this will be removed in a future release.
This version also includes many changes to improve
[testing coverage](https://app.codecov.io/gh/jgaeddert/liquid-dsp/)
across all interfaces, and make the interfaces across objects consistent.

  * build
    - migrated to the [CMake](https://cmake.org) build system, including
      building examples, sandbox programs, benchmarks, autotests, and
      detecting SIMD instruction extensions.
    - increased testing across the board including automated memory validation
      with [valgrind](https://valgrind.org)
    - detected and fixed memory leaks for numerous methods and testing
      harnesses including `bessel_azpdf` (thanks, @andreasbombe),
      `ofdmflexframesync`, `qdsync`, `qs1dsearch` (thanks, @nowls),
      `framesync64`, `eqrls`, and `fskdem`
    - added new `LIQUID_ENOCONV` error type to identify errors where
      algorithms did not converge
    - added new `LIQUID_ENOIMP` error type to identify methods that are not
      yet implemented
    - replaced warnings with internal error handling for many existing object
      methods
    - fixed SONAME not set in shared library (thanks again, @andreasbombe)
    - migrated README from markdown to re-structured text
  * framing
    - added more description to method definitions such as qpacketmodem and
      qdetector
    - dsssframe64: extended functionality to use qdsync, added standard
      methods such as copy(), added interfaces for specifying thresholds,
      reduced default spreading gain
  * filter
    - firinterp: added flush() method to run zeros through filter
    - rresamp: allow for default bandwidth with an input of -1
  * nco
    - fixed issue where frequency was being set improperly, added more
      extensive testing
    - improved the NCO object with VCO precision (thanks, @ArtemPisarenko)
  * random
    - added more extensive testing for various distributions to ensure values
      are generated properly

## 1.6.0 - 2023-06-19

Version 1.6.0 includes a new qdsync object to greatly simplify the frame
synchronization process in liquid, allowing for both detection and channel
impairment correction with a simplified interface. Additionally, code
coverage has been increased to 85% across the entire project, with numerous
bug fixes, stability improvements, and massive testing enhancements. From
an "architectural" standpoint, objects have been migrated to use standard
methods for consistency.

  * build
    - increased code coverage to 85% globally across entire project. This
      is the single largest effort included in this version and touches
      most modules in some way, most particularly the framing objects
    - cleaning build to remove compiler warnings (e.g. unused variables)
    - stripped version number off archive
  * dotprod
    - added support for AVX512-F (thanks, @vankxr!)
  * framing
    - added numerous tests to increase coverage to 84%
    - framesync64: using new qdsync object for simplified operation
    - qdsync: new frame detector and synchronizer to much more easily
      support frame processing. The object not only detects the frame, but
      also provides an initial carrier frequency, phase, and timign offset,
      and also corrects for these impairments, passing the results to the
      user in a clean callback function.
  * modem
    - cpfskmod: increasing phase stability for long runs
  * multichannel
    - added numerous tests to increase coverage to 88%
  * optim
    - added numerous tests to increase coverage to 92%
  * sequence
    - msequence: extended support for state variables up to m=31, reversed
      order for generator polynomial and internal state definition to be
      more consistent with literature and readily-available genpolys

## 1.5.0 - 2022-11-20

This release includes substantially improved testing coverage, deep copy()
methods for nearly all objects, improved speed, and resolves a number of
issues and pull requests.

  * build
    - added support for PlatformIO (https://platformio.org) for embeedded
      development (thanks, @jcw!)
    - incorporated recursive copy() methods to objects to facilitate c++ copy
      constructors for bindings; now all objects can be deep copied to a new
      object with their entire memory and state preserved
    - added convenience method to malloc and copy memory arrays
    - improved support for error codes across objects and methods
    - cleaned up spelling errors across project (thanks, @nowls!)
    - scrubbed function argument variable names to avoid underscore followed
      by a capital letter, causing trouble with pre-compiler processing
    - added basic test to check linking to installed library, incorporating
      into CI/CD pipelines
    - added more example programs
  * autotest
    - increased coverage testing (81% across entire project)
    - added the ability to "hammer" a particular test by running repeatedly on
      incremental seeds to assess specific edge cases (e.g. with random data)
    - added timer to show execution time for each test and identify areas for
      speed improvements
    - added methods for testing spectral response of various fields
    - added special directory for storing output logs: autotest/logs/
  * benchmark
    - replacing old C-based benchmark comparison with simpler python version
  * dotprod
    - added support for AVX SIMD in vector dot products (thanks, @vankxr!)
  * fft
    - adding macro to allow for aligned memory allocation if FFTW is used
      (thanks, @nowls!)
  * filter
    - added new halfband filter design using Parks-McClellan algorithm and
      qs1dsearch method to provide as exact a specification as possible
    - added method to retrieve filter response from coefficients array
    - dds: adding methods to get/set scale
    - firhilb, iirhilb: added block processing method
    - msresamp, resamp: adding method to provide the exact number of output
      samples with provided input size
    - msresamp2, resamp2: using better halfband filter design for exact user
      specifications
    - resamp: adding methods to get/set scale, fixing filter bank resolution
      (was hard-coded, now respects user configuration)
  * framing
    - framesync64: added methods to set callback and userdata (context)
      fields, adding support for exporting debugging files for post-analysis
      as well as python script for processing, adding better estimate of
      error vector magnitude
    - msource: added convenience method to recall number of samples generated
      by each source
    - ofdmflexframesync: added methods to set callback and userdata (context)
      fields
    - qpacketmodem: returning much better estimate of error vector magnitude
    - qsource: fixed issue with carrier frequency adjustment
  * optim
    - added qs1dsearch object to perform quad-section 1-dimensional search:
      similar to bisection search, but to find potentially non-continuous
      minimum/maximum of function

## 1.4.0 - 2022-02-03

  * autotest
    - automated code coverage testing (72%)
  * build
    - incorporated error handling in most methods with integer-based return value
  * agc
    - added interface to check if AGC is locked or not
  * buffer
    - cbuffer: added method to check if buffer is empty
  * dotprod
    - included methods to create/recreate object in reverse order
  * equalization
    - eqlms: added convenience methods for copying coefficients and running as
      a decimator
  * fft
    - spgram: adding convenience methods for getting/setting properties
    - spwaterfall: adding convenience methods for getting/setting properties
  * filter
    - new direct digital synthesis (DDS) family of objects which allows
      cascaded half-band interpolation/decimation with arbitrary frequency
      translation
    - new fractional delay (fdelay) family of objects which allow for
      adjustable large fractional delays
    - firdecim: adding convenience method to get decimation rate
    - firfilt: adding convenience methods to get/copy coefficients as well as
      create object using firdespm algorithm
    - firinterp: adding convenience methods for new ways to create object as
      well as getting object properties
    - firpfb: adding convenience methods to create default object, write
      samples
    - resamp2: adding methods to get/set output scale
    - rresamp: adding methods to operate with block execution
  * framing
    - new arbitrary rate symstreamr object (derived from symstream family)
    - framesync64: adding methods for getting/resetting frame data statistics
      as well as getting/setting detection threshold
    - gmskframegen/gmskframesync: extending methods to support easier
      operation, getting/resetting frame data statistics, more control over
      parameters in create methods
    - ofdmflexframesync: adding methods to get/reset frame data statistics
    - qdetector: adding method to get threshold
    - qpilotsync: adding method to get error vector magnitude
    - symstream: adding convenience methods for getting properties
    - symtrack: adding convenience methods for getting/setting properties
  * math
    - adding `liquid_` prefix to winodowing functions while supporting backwards
      compatibility with older methods
    - adding `liquid_` prefix to internal polynomial functions, using
      common double-precision method for finding polynomial roots
  * multichannel
    - firpfbch2: adding convenience methods to get object properties
  * modem
    - modem: adding type extension for more consistency: `modem` -> `modemcf`,
      supporting backwards compatibility with API shim

## 1.3.1 - 2019-07-28

  * autotest
    - runs with random seeds (based on time) for diveristy
    - output .json file for post-analysis
  * build
    - cleaned up compiler warnings across most platforms
    - incorporated continuous integration script
    - compact header APIs across all interfaces in liquid.h
    - consistent build across Linux and macOS
  * agc
    - added more convenience methods, improved autotest stability
  * fft
    - spwaterfall less verbose with more convenience methods
  * filter
    - new rresamp family of objects to implement rational rate
      resampling; very useful for fixed buffer sizes
    - resamp now uses fixed-point phase for faster computation
    - fixed issues with msresamp2 ordering to have expected roll-off
      performance
    - added notch filter design option for firfilt (with autotest)
  * framing
    - completely reworked msource family of objects to use firpfbch2
      family of objects for computationally efficient
    - added preliminary fskframe generator and synchronizer objects
  * math
    - improved functions for speed, is_prime()
    - improved stability and consistency of root-finding algorithms
  * multichannel
    - added new firpfbchr family of objects for arbitrarily setting
      number of channels and down-sampling rates
  * modem
    - refactored objects for amplitude modulation/demodulation to use
      Hilbert transform, added autotest scripts
  * nco
    - improving consistency across platforms

## 1.3.1 - 2017-10-23

  * improved selection of SSE/MMX extension flags for gcc
  * agc
    - adding squelch functionality back into gain control object
  * filter
    - adding callback function for Parks-McClellan algorithm to allow generic 
      filter prototyping
    - fixed double-free bug in iirfilt
  * fft
    - adding new spwaterfall family of objects for generating waterfall plot
      and automatically down-size as input sample size grows
  * sequence
    - fixed issue with order of operations causing inconsistent behavior across
      different platforms

## 1.3.0 - 2016-12-30

  * New MIT/X11 license (https://opensource.org/licenses/MIT)
  * agc (automatic gain control)
    - major code scrub, improved reliability, simplified interface
    - block-level operation
  * buffer
    - new cbuffer objects for dynamic-sized circular buffering
  * channel
    - new module to define certain channel emulation objects
    - new functionality includes carrier frequency/phase offsets,
      (time-varying) multi-path, shadowing, and additive noise
  * dotprod
    - adding method to compute x^T * x of a vector (sum of squares)
  * equalization
    - improved interface for LMS and RLS equalizers
    - simplified methods to support blind operation
  * fec (forward error correctino)
    - interleaver and packetizer moved from the framing to the fec module
    - packetizer now includes data whitening
  * fft (fast Fourier transform)
    - general speed improvements for one-dimensional FFTs
    - completely reimplemented spgram (spectral periodogram) objects to
      include both complex and real-values amples with simpler interface
    - reimplemented asgram (ASCII spectral periodogram) objects
  * filter
    - additional prototype create methods, block execution
    - added new fftfilt family of objects to realize linear filter
      with fast Fourier transforms
    - interp family renamed to firinterp, new iirinterp family
    - decim family renamed to firdecim, new iirdecim family
    - add linear interpolation for arbitrary resamp output
    - new multi-stage half-band resampler family
    - symsync: improved stability, added rate adjustment to help pull in
      sample rate offsets
    - added autotests for validating performance of both the
      resamp and msresamp objects
  * framing
    - added `framedatastats` object for counting statistics across different
      framing objects (e.g. number of total bytes received)
    - adding generic callback function definition for all framing
      structures
    - qpacketmodem: new object to easily combine modulating and encoding;
      buffer of data in, modulated and encoded samples out
    - qpilotgen/qpilotsync: new objects to add and synchronize pilot symbols
      to modulated symbols to recover carrier frequency/phase, and gain
    - framing objects: frame64, flexframe now use qpacketmodem, qpilotgen, and
      qpilotsync objects for unified interface and vastly improved performance
    - flexframe: vastly simplified interface
    - qdetector: new family for pre-demodulator synchronizion and detection
    - moved interleaver and packetizer objects to `fec` module
    - symstream: new family for generating random stream of modulated samples
    - msource: new family for generating multiple signals for a single source,
      including tones, noise, modulated symbols
    - symtrack: new family for tracking a stream of symbols and recovering
      signal level, timing, carrier frequency/phase without pilots
  * math
    - new windowing methods (e.g. 7-term Blackman-harris window)
  * matrix
    - adding smatrix family of objects (sparse matrices)
    - improving linear solver methods (roughly doubled speed)
  * modem
    - re-organizing internal linear modem code (no interface change)
    - freqmod/freqdem: new interface, block-level execution for analog FM
    - cpfskmod/cpfskdem: new family for generic non-linear continuous-phase
      frequency-shift modulation (e.g. minimum-shift keying)
    - fskmod/fskdem: new family for non-coherent frequency-shift keying
      modulation, often with many samples per symbol (e.g. 256-FSK)
  * multicarrier
    - adding OFDM framing option for window tapering
    - simplfying OFDM framing for generating preamble symbols (all
      generated OFDM symbols are the same length)
    - adding run-time option for debugging ofdmframesync
    - adding method for initializing subcarriers with frequency range
  * nco (numerically-controlled oscillator)
    - general performance improvements, adding block-level execution
  * optim
    - gradsearch (gradient search) uses internal linesearch for
      significant speed increase and better reliability
    - gradsearch interface greatly simplified
  * utility
    - build no longer auto-generates tables at compile time (helps with cross
      compilation)
  * vector
    - new module to simplify basic vector operations
  * miscellany
    - documentation moved from core repository to website
    - global header file (`liquid.h`) include more structured source
    - consistent naming of reset() methods for most objects

## 1.2.0 - 2012-04-26

  * dotprod
    - leveraging SIMD extensions for vector operations (SSE2, SSE3)
  * fft
    - completely re-structured internal transform strategy including
      Cooley-Tukey mixed-radix algorithm, Rader algorithm for FFTs of
      prime length, and specific codelets for small-size transforms.
  * math
    - new modular arithmetic methods (prime factor, totient, etc.)
  * modem
    - new API creates linear modem objects with one argument, e.g.
      LIQUID_MODEM_QAM16
    - new type definitions for analog modems

## 1.1.0 - 2011-12-23

  * build
    - simplifying build environment by explicitly defining object
      dependencies (no longer auto-generated; much faster now)
  * documentation
    - new tutorials (ofdmflexframe)
    - sections on new objects/methods (msresamp, gmsk filter design,
      soft-decision demodulation)
    - adding useful figures (polyfit-lagrange, symsync)
    - adding BER performance plots for new FEC schemes
    - adding BER performance plots for sqam32 and sqam128
  * agc
    - fixing scaling issues
    - improving computation speed
    - simplifying interface with a single, unified design model
  * equalization
    - adding support for decision-directed equalizers
  * fec
    - adding soft decoding (when available) for forward error-correction
      schemes; generally improves performance by about 2 dB Eb/N0
    - adding half-rate Golay(24,12) code
    - adding SEC-DED codes: (22,16), (39,32), (72,64)
  * filter
    - firdes: adding Nyquist prototyping design method
    - firdes: adding new GMSK receive filter design method
    - interp: porting to efficient polyphase filterbank implementation,
      adding prototype create() method
    - adding multi-stage resampler for efficient decimation and
      interpolation
  * framing
    - adding ofdmflexframe family of objects capable of defining which
      subcarriers are nulls/pilots/data, and easily loading data into
      frames. Very similar to 'flexframe' in usage.
    - supporting soft packet decoding (interleaving, etc.)
    - adding gmskframe generator and synchronizer object; simple,
      reliable
  * matrix
    - adding Cholesky factorization A = R^T * R (for positive definite
      matrices)
    - adding conjugate gradient solver (for positive definite matrices)
  * modem
    - adding simple on/off keying (OOK) type
    - adding 256-APSK type (6,18,32,36,46,54,64)
    - adding 'square' (cross) 32-, 128-QAM types
    - adding 'optimal' 64-, 128-, and 256-QAM constellations
    - improved speed of most schemes' modulate/demodulate
      implementations
    - adding soft-decision (log-likelihood ratio) demodulation
    - adding GMSK modulation/demodulation with improved filter design
  * multicarrier
    - ofdmframe: improving synchronization and reliability in
      interference environments, enabling squelch, improving
      equalization
  * optim
    - simplified interface to gradient search

## [1.0.0] - 2011-04-29

_First release._

