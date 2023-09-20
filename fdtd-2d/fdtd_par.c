// TODO3: check compilation includes and libs, maybe not everything is needed

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// threads array
pthread_t* threads;

// barrier initialization
pthread_barrier_t barrier;
pthread_barrierattr_t attr;

// function used to calculate ey
void* eyParallel(void* arg){
  int id = *(int*)arg;


  int start = NX/(NUMBER_THREADS/2) * id;
  if (id == 0){
    start += 1;
  }

  //printf("thread %d ey working from %d to %d\n", id, start, NY/(NUMBER_THREADS/2)*(id+1));


  for (int t=0 ; t<TMAX ; t++){
    for (int f = 0; f < NY; f++)
	      ey_p[0][f] = _fict_p[t];
    
    // i = NX/(NUMBER_THREADS/2) * id + 1;
    // i < NX/(NUMBER_THREADS/2)*(id+1)
    // i++
    // TODO: the +1 at the start here is because we dont start at 0, but at 1
    // so for the other threads we need to start at the next line not jump 1 extra
    for (int i=start ; i<NX/(NUMBER_THREADS/2)*(id+1) ; i++){
      for (int j=0 ; j<NY ; j++){
        ey_p[i][j] = ey_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i-1][j]);
      }
    }

    pthread_barrier_wait(&barrier);

    //printf("thread %d ey working at hz from %d to %d\n", id, (NX/2)/(NUMBER_THREADS/2)*id, (NX/2)/(NUMBER_THREADS/2)*(id+1));

    // CALCULATE HALF HZ (0,0 -> nx/2,ny/2)
    int end_hz = (NX/2)/(NUMBER_THREADS/2)*(id+1);
    if (id == NUMBER_THREADS/2 - 1){
      end_hz = NX/2;
    }
    // here we dont have "i < end_hz-1" <-- this is because -1 is only is the last
    // line, not in the middle
    for (int i= (NX/2)/(NUMBER_THREADS/2)*id; i< end_hz; i++){
      for (int j=0 ; j<NY-1 ; j++){
        hz_p[i][j] = hz_p[i][j] - 0.7*(ex_p[i][j+1] - ex_p[i][j] + ey_p[i+1][j] - ey_p[i][j]);
      }
    }
    pthread_barrier_wait(&barrier);
  }

}

// function used to calculate ex
void* exParallel(void* arg){
  int id = *(int*)arg;


  int start = NY/(NUMBER_THREADS/2) * id;
  if (id == 0){
    start += 1;
  }
  //printf("thread %d ex working from %d to %d\n", id, start, NY/(NUMBER_THREADS/2)*(id+1));


  for (int t=0 ; t<TMAX ; t++){
    for (int i=0 ; i<NX ; i++){
      // TODO: the +1 at the start here is because we dont start at 0, but at 1
      // so for the other threads we need to start at the next line not jump 1 extra
      for (int j= start ; j<NY/(NUMBER_THREADS/2)*(id+1) ; j++){
        ex_p[i][j] = ex_p[i][j] - 0.5*(hz_p[i][j]-hz_p[i][j-1]);
      }
    }
    pthread_barrier_wait(&barrier);

    //printf("thread %d ex working at hz from %d to %d\n", id, (NX/2)/(NUMBER_THREADS/2)*id + (NX/2), (NX/2)/(NUMBER_THREADS/2)*(id+1) + (NX/2));

    int end_hz = (NX/2)/(NUMBER_THREADS/2)*(id+1) + (NX/2);
    if (id == NUMBER_THREADS/2 - 1){
      end_hz = NX-1;
    }
    // CALCULATE HALF HZ (nx/2,ny/2 -> nx,ny)
    // here we have i < end_hz-1 because we dont need to caculate the last line
    // also we have the same issue as the for above, only the last line has -1
    // not all threads
    for (int i= (NX/2)/(NUMBER_THREADS/2)*id + (NX/2); i< end_hz; i++){
      for (int j=0 ; j<NY-1 ; j++){
        hz_p[i][j] = hz_p[i][j] - 0.7*(ex_p[i][j+1] - ex_p[i][j] + ey_p[i+1][j] - ey_p[i][j]);
      }
    }
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
  //printf("ex\n");
  //print_array(ex, NX, NY);
  //printf("\ney\n");
  //print_array(ey, NX, NY);
  //printf("\nhz\n");
  //print_array(hz, NX, NY);
  //printf("\n\n");
    

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
    // ./fdtd -d <DATASET> -t <NUMBER_THREADS>
  } else if (argc == 5){
    // <DATASET> 
    if (!strcmp(argv[1],"-d")){
      if (!strcmp(argv[2],"small")){
        TMAX = 2;
        NX = 100;
        NY = 100;
      } else if (!strcmp(argv[2],"medium")){
        TMAX = 5;
        NX = 100;
        NY = 100;
      } else if (!strcmp(argv[2],"large")){
        TMAX = 10;
        NX = 100;
        NY = 100;
      } else {
        printf("Invalid dataset\n");
        return 0;
      }
    } else {
      printf("Invalid arguments\n");
      return 0;
    }
    // -t <NUMBER_THREADS>
    if(!strcmp(argv[3],"-t")){
      if (atoi(argv[4]) <= 0){
        printf("Invalid number of threads\n");
        return 0;
      }
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
  pthread_barrier_init(&barrier, &attr, NUMBER_THREADS);


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
