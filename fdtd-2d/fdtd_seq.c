// TODO1: polish this
// TODO2: check for compilation includes and libs


#include <stdio.h>
#include <string.h>
#include <pthread.h>

/* Include polybench common header. */
#include <polybench.h>

#include "fdtd-2d.h"

/* Array initialization. */
static void init_array (int tmax, int nx, int ny, double** ex, double** ey, double** hz, double* _fict_) {
  int i, j;

  for (i = 0; i < tmax; i++)
    _fict_[i] = (double) i;
  
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
      {
        ex[i][j] = ((double) i*(j+1)) / nx;
        ey[i][j] = ((double) i*(j+2)) / ny;
        hz[i][j] = ((double) i*(j+3)) / nx;
      }
}


void print_array(double** array, int x, int y){
    for (int i=0;i<x;i++){
        for (int j=0;j<y;j++){
        printf("%.2f ", array[i][j]);
        }
        printf("\n");
    }
}

//função para alocar a matriz
void* matrix_alloc_data(int x, int y){

    // aloca um vetor de LIN ponteiros para linhas
    double** vec = malloc (NX * sizeof (double*)) ;

    // aloca cada uma das linhas (vetores de COL inteiros)
    for (int i=0; i < NX; i++)
        vec[i] = malloc (NY * sizeof (double)) ;

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


void free_matrix(double** mat, int x){
  for (int i=0;i<x;i++){
    free(mat[i]);
  }
  free(mat);
}

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_fdtd_2d(int tmax,
		    int nx,
		    int ny,
		    double ** ex,
        double ** ey,
        double ** hz,
        double * _fict_)
{
  int t, i, j;

  for(t = 0; t < TMAX; t++)
    {
      // printf("t: %d\n", t);
      for (j = 0; j < NY; j++)
	      ey[0][j] = _fict_[t];

      for (i = 1; i < NX; i++)
	      for (j = 0; j < NY; j++)
	        ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
      
      for (i = 0; i < NX; i++)
	      for (j = 1; j < NY; j++)
	        ex[i][j] = ex[i][j] - 0.5*(hz[i][j]-hz[i][j-1]);
      
      for (i = 0; i < NX - 1; i++)
	      for (j = 0; j < NY - 1; j++)
	        hz[i][j] = hz[i][j] - 0.7*  (ex[i][j+1] - ex[i][j] +
				                                ey[i+1][j] - ey[i][j]);

      //print_array(ex, NX, NY);
      //printf("\n\n");
      //print_array(ey, NX, NY);
      //printf("\n\n");
      //print_array(hz, NX, NY);
    }

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
  } else if (argc == 3){
    // ./fdtd -d <DATASET> 
    if (!strcmp(argv[1],"-d")){
      if (!strcmp(argv[2],"small")){
        TMAX = 120;
        NX = 10240;
        NY = 10240;
      } else if (!strcmp(argv[2],"medium")){
        TMAX = 240;
        NX = 10240; 
        NY = 10240;
      } else if (!strcmp(argv[2],"large")){
        TMAX = 360;
        NX = 10240;
        NY = 10240;
      } else {
        printf("Invalid dataset\n");
        return 0;
      }
    } else {
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

  // inicializa todos os arrays cm os devidos valores iniciais
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

  //print_array(ey, NX, NY);

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

    free_matrix(ex, NX);
    free_matrix(ey, NX);
    free_matrix(hz, NX);
    free(_fict_);


  return 0;
}
