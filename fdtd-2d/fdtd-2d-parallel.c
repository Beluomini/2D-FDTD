/**
 * fdtd-2d.c: This file is part of the PolyBench/C 3.2 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <pthread.h>


/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 50x1000x1000. */
#include "fdtd-2d.h"

/* Array initialization. */
static void init_array (int tmax, int nx, int ny, double** ex, double** ey, double** hz, double* _fict_) {
  int i, j;

  for (i = 0; i < tmax; i++)
    _fict_[i] = (DATA_TYPE) i;
  
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
      {
        ex[i][j] = ((DATA_TYPE) i*(j+1)) / nx;
        ey[i][j] = ((DATA_TYPE) i*(j+2)) / ny;
        hz[i][j] = ((DATA_TYPE) i*(j+3)) / nx;
      }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int nx,
		 int ny,
		 double** ex,
     double** ey,
     double** hz)
{
  int i, j;

  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++) {
      fprintf(stderr, DATA_PRINTF_MODIFIER, ex[i][j]);
      fprintf(stderr, DATA_PRINTF_MODIFIER, ey[i][j]);
      fprintf(stderr, DATA_PRINTF_MODIFIER, hz[i][j]);
      if ((i * nx + j) % 20 == 0) fprintf(stderr, "\n");
    }
  fprintf(stderr, "\n");
}

//função ṕara alocar a matriz
void* matrix_alloc_data(int x, int y){

  // aloca um vetor de LIN ponteiros para linhas
  double** vec = malloc (NX * sizeof (double*)) ;

  // aloca cada uma das linhas (vetores de COL inteiros)
  for (int i=0; i < NX; i++)
    vec[i] = malloc (NY * sizeof (double)) ;

  // double** vec = (double**)malloc(x * sizeof(double*) * y * sizeof(double));
  // for (int i=0;i<x;i++){
  //   for (int j=0;j<y;j++){
  //     vec[i][j] = 0.0;
  //   }
  // }
  return vec;
}

//função ṕara alocar o vetor
void* vector_alloc_data(int x){
  double* vec = (double*)malloc(x * sizeof(double));
  for (int i=0;i<x;i++){
    vec[i] = i;
  }
  return vec;
}

double** ex_p;
double** ey_p;
double** hz_p;
double* _fict_p;

void printMatrix(double** m, int x, int y){
  for (int i=0;i<x;i++){
    for (int j=0;j<y;j++){
      printf("%.2f ", m[i][j]);
    }
    printf("\n");
  }
}

pthread_t* threads;

void* eyParallel(void* arg){
  int start = *(int*)arg;
  start = NX/(NUMBER_THREADS/2) * start;
  float sum = 0;
    
    for (int i=start+1 ; i<NX/(NUMBER_THREADS/2)*(start+1) ; i++){
      for (int j=0 ; j<NY ; j++){
        ey_p[i][j] = ey_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i-1][j]);
      }
    }

}

void* exParallel(void* arg){
  int start = *(int*)arg;
  start = NY/(NUMBER_THREADS/2) * start;
  float sum = 0;
    
    for (int i=0 ; i<NX ; i++){
      for (int j=start+1 ; j<NY/(NUMBER_THREADS/2)*(start+1) ; j++){
        ex_p[i][j] = ex_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i][j-1]);
      }
    }

}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static void kernel_fdtd_2d(int tmax,
		    int nx,
		    int ny,
		    double ** ex,
        double ** ey,
        double ** hz,
        double * _fict_)
{

  int t, i, j;

#pragma scop

  int* threads_id = malloc(NUMBER_THREADS * sizeof(int));
  threads = malloc(NUMBER_THREADS * sizeof(pthread_t));

  for(t = 0; t < _PB_TMAX; t++)
    {
      for (j = 0; j < _PB_NY; j++)
	      ey[0][j] = _fict_[t];
      
      for (i = 0; i < NUMBER_THREADS/2; i++){
        threads_id[i] = i;
        pthread_create(&threads[i], NULL, eyParallel, &threads_id[i]);
      }

      for (i = 0; i < NUMBER_THREADS/2; i++){
        pthread_create(&threads[i+(NUMBER_THREADS/2)], NULL, exParallel, &threads_id[i]);
      }

    for (i = 0; i < NUMBER_THREADS; i++){
      pthread_join(threads[i], NULL);
    }
    

      // for (i = 1; i < _PB_NX; i++)
	    //   for (j = 0; j < _PB_NY; j++)
	    //     ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
      
      // for (i = 0; i < _PB_NX; i++)
	    //   for (j = 1; j < _PB_NY; j++)
	    //     ex[i][j] = ex[i][j] - 0.5*(hz[i][j]-hz[i][j-1]);
      
      // BARREIRA 1
      for (i = 0; i < _PB_NX - 1; i++)
	      for (j = 0; j < _PB_NY - 1; j++)
	        hz[i][j] = hz[i][j] - 0.7*  (ex[i][j+1] - ex[i][j] +
				                                ey[i+1][j] - ey[i][j]);
      // BARREIRA 2

    // printMatrix(ex, NX, NY);
    // printf("\n");
    // printMatrix(ey, NX, NY);
    // printf("\n");
    printMatrix(hz, NX, NY);
    printf("\n\n");
      
    }

#pragma endscop
}

int main(int argc, char** argv)
{

  if (argc == 2){
    // ./fdtd -h   or ./fdtd -help
    if (!strcmp(argv[1],"-h") || !strcmp(argv[1],"-help")){
      printf("Usage: ./fdtd -d <DATASET>\n");
      printf("Default values:\n");
      printf("DATASET: [small, medium, large]\n");
      printf("small = 400 iterations");
      printf("medium = 800 iterations");
      printf("large = 1200 iterations");
      return 0;
    }
  } else if (argc == 5){
    // ./fdtd -d <DATASET> 
    if (!strcmp(argv[1],"-d")){
      if (!strcmp(argv[2],"small")){
        TMAX = 2;
        NX = 5;
        NY = 5;
      } else if (!strcmp(argv[2],"medium")){
        TMAX = 80;
        NX = 13000;
        NY = 13000;
      } else if (!strcmp(argv[2],"large")){
        TMAX = 1200;
        NX = 13000;
        NY = 13000;
      } else {
        printf("Invalid dataset\n");
        return 0;
      }
    } else {
      printf("Invalid arguments\n");
      return 0;
    }

    if(!strcmp(argv[3],"-t")){
      NUMBER_THREADS = atoi(argv[4]);
    }else{
      printf("Invalid arguments\n");
      return 0;
    }

  } else {
    printf("Invalid arguments\n");
    return 0;
    
  }
  
  /* Retrieve problem size. */
  int tmax = TMAX;
  int nx = NX;
  int ny = NY;


  double** ex = matrix_alloc_data(NX, NY);
  double** ey = matrix_alloc_data(NX, NY);
  double** hz = matrix_alloc_data(NX, NY);
  double* _fict_ = vector_alloc_data(NY);

  /* Initialize array(s). */
  init_array (tmax, nx, ny, ex, ey, hz, _fict_);
  
  // Usar a função matrix_alloc_data para alocar as matrizes  
  ex_p = ex;
  ey_p = ey;
  hz_p = hz;
  _fict_p= _fict_;

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_fdtd_2d (tmax, nx, ny, ex, ey, hz, _fict_);


  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  // /* Prevent dead-code elimination. All live-out data must be printed
  //    by the function call in argument. */
  // polybench_prevent_dce(print_array(nx, ny, POLYBENCH_ARRAY(ex),
	// 			    POLYBENCH_ARRAY(ey),
	// 			    POLYBENCH_ARRAY(hz)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(ex);
  POLYBENCH_FREE_ARRAY(ey);
  POLYBENCH_FREE_ARRAY(hz);
  POLYBENCH_FREE_ARRAY(_fict_);

  return 0;
}
