// TODO: error when using 4 threads. Seg fault
// TODO: not working when nx or ny are odd

#include <stdio.h>
#include <stdlib.h>
// unused???
//#include <unistd.h>
#include <string.h>
// unused???
//#include <math.h>
#include <pthread.h>

// only being used for the time function
#include <polybench.h>

#include "fdtd-2d.h"

// matrices and array initialization
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

//função para alocar o vetor
void* vector_alloc_data(int x){
  double* vec = (double*)malloc(x * sizeof(double));
  for (int i=0;i<x;i++){
    vec[i] = i;
  }
  return vec;
}

// global pointers to use inside threads
double** ex_p;
double** ey_p;
double** hz_p;
double* _fict_p;

pthread_t* threads;

pthread_barrier_t barrier;
pthread_barrierattr_t attr;
int ret;
// function used to calculate ey
void* eyParallel(void* arg){
  int id = *(int*)arg;
  int start = NX/(NUMBER_THREADS/2) * id;
    
  int start_hz, end_hz;
  start_hz = (NX/2)/(NUMBER_THREADS/2)*id;
  end_hz = (NX/2)/(NUMBER_THREADS/2)*(id+1);
  if (id == NUMBER_THREADS/2){
    end_hz = NX/2;
  }
  printf("Thread ey(%d), start:%d, next:%d, start_hz: %d, end_hz: %d\n", id, start, NX/(NUMBER_THREADS/2) * (id+1), start_hz, end_hz);

  for (int t=0 ; t<TMAX ; t++){

    


    for (int f = 0; f < NY; f++)
	      ey_p[0][f] = _fict_p[t];
    
    //printf("%d, %d\n", start+1, NX/(NUMBER_THREADS/2)*(start+1));

    for (int i=start+1 ; i<NX/(NUMBER_THREADS/2)*(id+1) ; i++){
      for (int j=0 ; j<NY ; j++){
        //printf("ey[%d][%d] = %f\n, hz_p = %f, hz_p = %f", i, j, ey_p[i][j], hz_p[i][j], hz_p[i-1][j]);
        ey_p[i][j] = ey_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i-1][j]);
        //printf("ey[%d][%d] = %f\n", i, j, ey_p[i][j]);
      }
    }

    //printf("pre barrier y\n");

    // BARRIER 1 HERE
    pthread_barrier_wait(&barrier);

    // CALCULATE HALF HZ (0,0 -> nx/2,ny/2)
    


    for (int i= start_hz; i< end_hz-1; i++){
      for (int j=0 ; j<NY-1 ; j++){
        hz_p[i][j] = hz_p[i][j] - 0.7*(ex_p[i][j+1] - ex_p[i][j] + ey_p[i+1][j] - ey_p[i][j]);
      }
    }
    //printf("pre barrier2 y\n");
    // BARRIER 2 HERE
    pthread_barrier_wait(&barrier);
  }

}

// function used to calculate ex
void* exParallel(void* arg){
  int id = *(int*)arg;
  int start = NY/(NUMBER_THREADS/2) * id;
  float sum = 0;

  int start_hz, end_hz;
  start_hz = (NX/2)/(NUMBER_THREADS/2)*id + (NX/2);
  end_hz = (NX/2)/(NUMBER_THREADS/2)*(id+1) + (NX/2);
  if (id == NUMBER_THREADS/2){
    end_hz = NX;
  }

    
  printf("Thread ex(%d), start:%d, next:%d, start_hz: %d, end_hz: %d\n", id, start, NX/(NUMBER_THREADS/2) * (id+1), start_hz, end_hz);


  for (int t=0 ; t<TMAX ; t++){
    
  
    for (int i=0 ; i<NX ; i++){
      for (int j=start+1 ; j<NY/(NUMBER_THREADS/2)*(id+1) ; j++){
        //printf("ex[%d][%d] = %f, hz_p = %f, hz_p = %f\n", i, j, ex_p[i][j], hz_p[i][j], hz_p[i][j-1]);
        ex_p[i][j] = ex_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i][j-1]);
        //printf("after ex[%d][%d] = %f\n", i, j, ex_p[i][j]);
      }
    }
    //printf("pre barrier x\n");
    // BARRIER 1 HERE
    pthread_barrier_wait(&barrier);

    // CALCULATE HALF HZ (nx/2,ny/2 -> nx,ny)

    for (int i= start_hz; i< end_hz-1; i++){
      for (int j=0 ; j<NY-1 ; j++){
        //printf("ex[%d][%d] = %f\n", i, j, ex_p[i][j]);
        //printf("hz_p = %f, hz_p = %f\n", hz_p[i][j], hz_p[i][j-1]);
        hz_p[i][j] = hz_p[i][j] - 0.7*(ex_p[i][j+1] - ex_p[i][j] + ey_p[i+1][j] - ey_p[i][j]);
      }
    }
    //printf("pre barrier2 x\n");
    // BARRIER 2 HERE
    pthread_barrier_wait(&barrier);

  }  

}

void free_matrix(double** mat, int x){
  for (int i=0;i<x;i++){
    free(mat[i]);
  }
  free(mat);
}

// main computation function
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
  

  // remove this loop, need to be inside ex, ey parallel code
  //for(t = 0; t < TMAX; t++)
  //  {
      // remove this loop, put inside ey parallel code
      //for (j = 0; j < NY; j++)
	    //  ey[0][j] = _fict_[t];
      
      // this loop is right being here, i think
      for (i = 0; i < NUMBER_THREADS/2; i++){
        threads_id[i] = i;
        pthread_create(&threads[i], NULL, eyParallel, &threads_id[i]);
      }

      // also this one
      for (i = 0; i < NUMBER_THREADS/2; i++){
        pthread_create(&threads[i+(NUMBER_THREADS/2)], NULL, exParallel, &threads_id[i]);
      }

    // here we have the first barrier, but inside both ex and ey threads
    // after all threads stop here, they should start calculating hz, then 
    // another barrier is placed, and then they calculate ex and ey again
      
    for (i = 0; i < NUMBER_THREADS; i++){ // -> so this join should be removed  
      pthread_join(threads[i], NULL);
    }
    
      // BARREIRA 1
    //  for (i = 0; i < NX - 1; i++)   // -> this should also be inside ex and ey
	  //    for (j = 0; j < NY - 1; j++) // and divide the task in half 
	  //      hz[i][j] = hz[i][j] - 0.7*  (ex[i][j+1] - ex[i][j] +
		//		                                ey[i+1][j] - ey[i][j]);
      // BARREIRA 2

    print_array(hz, NX, NY);
    printf("\n\n");
      
  //  }

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
        TMAX = 1;
        NX = 6;
        NY = 6;
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

  // initialize barrier
  ret = pthread_barrier_init(&barrier, &attr, NUMBER_THREADS);


  // inicializa todos os arrays cm os devidos valores iniciais
  init_array (tmax, nx, ny, ex, ey, hz, _fict_);
  
  // Usar a função matrix_alloc_data para alocar as matrizes  
  ex_p = ex;
  ey_p = ey;
  hz_p = hz;
  _fict_p= _fict_;

  // time function from polybench 
  polybench_start_instruments;

  // 'main' function, where work is being done
  kernel_fdtd_2d (tmax, nx, ny, ex, ey, hz, _fict_);


  // stop timer and print results
  polybench_stop_instruments;
  polybench_print_instruments;


  // free matrixes and array
  free_matrix(ex, NX);
  free_matrix(ey, NX);
  free_matrix(hz, NX);
  free(_fict_);


  return 0;
}
