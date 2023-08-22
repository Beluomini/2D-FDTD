/**
 * fdtd-2d.h: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#ifndef FDTD_2D_H
# define FDTD_2D_H

/* Default to STANDARD_DATASET. */
# if !defined(SMALL_DATASET) && !defined(MEDIUM_DATASET) && !defined(LARGE_DATASET)
#  define MEDIUM_DATASET
# endif

/* Do not define anything if the user manually defines the size. */
# if !defined(NX) && ! defined(NY) && !defined(TMAX)
/* Define the possible dataset sizes. */


#  ifdef SMALL_DATASET
#   define TMAX 400
#   define NX 13000
#   define NY 13000
#  endif

#  ifdef MEDIUM_DATASET /* Default if unspecified. */
#   define TMAX 800
#   define NX 13000
#   define NY 13000
#  endif

#  ifdef LARGE_DATASET
#   define TMAX 1200
#   define NX 13000
#   define NY 13000
#  endif

# endif /* !N */

# define _PB_TMAX POLYBENCH_LOOP_BOUND(TMAX,tmax)
# define _PB_NX POLYBENCH_LOOP_BOUND(NX,nx)
# define _PB_NY POLYBENCH_LOOP_BOUND(NY,ny)

# ifndef DATA_TYPE
#  define DATA_TYPE double
#  define DATA_PRINTF_MODIFIER "%0.2lf "
# endif


#endif /* !FDTD_2D */
