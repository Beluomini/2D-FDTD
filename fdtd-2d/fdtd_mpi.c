#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// only being used for the time function
//#include <polybench.h>

#include <mpi.h>

#include "fdtd-2d.h"

// matrices and array initialization
static void init_array (int tmax, int nx, int ny, double* ex, double* ey, double* hz, double* _fict_) {
  int i, j;

  for (i = 0; i < tmax; i++)
    _fict_[i] = (double) i;
  
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
      {
        ex[i*nx+j] = ((double) i*(j+1)) / nx;
        ey[i*nx+j] = ((double) i*(j+2)) / ny;
        hz[i*nx+j] = ((double) i*(j+3)) / nx;
      }
}

void print_array(double* array, int x, int y){
    for (int i=0;i<x;i++){
        for (int j=0;j<y;j++){
        printf("%.2f ", array[i*x+j]);
        }
        printf("\n");
    }
}

//função para alocar a matriz
double* matrix_alloc_data(int x, int y){

  double* vec = (double*)malloc(x * y * sizeof(double));
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

void free_matrix(double* mat, int x){
  free(mat);
}

int main(int argc, char** argv) {
    int process_Rank, size_Of_Cluster, message_Item;

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
          TMAX = 1;
          NX = 8;
          NY = 8;
        } else if (!strcmp(argv[2],"medium")){
          TMAX = 1;
          NX = 8;
          NY = 8;
        } else if (!strcmp(argv[2],"large")){
          TMAX = 1;
          NX = 8;
          NY = 8;
        } else {
          printf("Invalid dataset\n");
          return 0;
        }
      } else {
        printf("Invalid arguments\n");
        return 0;
      }
      // -t <size_Of_Cluster>
      if(!strcmp(argv[3],"-t")){
        if (atoi(argv[4]) <= 0){
          printf("Invalid number of threads\n");
          return 0;
        }
        size_Of_Cluster = atoi(argv[4]);
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

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    double* ex;
    double* ey;
    double* hz;
    double* _fict_;

    if(process_Rank == 0){

      ex = matrix_alloc_data(NX, NY);
      ey = matrix_alloc_data(NX, NY);
      hz = matrix_alloc_data(NX, NY);
      _fict_ = vector_alloc_data(NY);

      // Inicializa as matrizes e o vetor
      init_array (tmax, nx, ny, ex, ey, hz, _fict_);

      print_array(ey, NX, NY);
      printf("\n---------------------\n");
      print_array(ex, NX, NY);
      printf("\n---------------------\n");
      print_array(hz, NX, NY);
      printf("\nsize of cluster: %d\n\n", size_Of_Cluster);

      // time function from polybench 
      //polybench_start_instruments;

      //stop timer and print results
      //polybench_stop_instruments;
      //polybench_print_instruments;

     

    }

    double *scattered_Data_ey = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));

    int start = 0;
    int end = NY/size_Of_Cluster;

    // Envia os dados de Ey para os processos com excessão dos dados que o processo 0 vai usar
    printf("processo 0 enviando %f para todos os processos com start %d e end %d\n", ey[end], start, end);
    MPI_Scatter(ey, (end-start)*NY, MPI_DOUBLE, scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //CONTA

    // for (int f = 0; f < NY; f++)
    //   ey[0][f] = _fict_[t];

    // for (int i=start ; i<end ; i++){
    //   for (int j=0 ; j<NY ; j++){
    //     ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
    //   }
    // }

    // for (int i=start ; i<end ; i++){
    //   for (int j=0 ; j<NY ; j++){
    //     ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
    //   }
    // }

    // for (int i=0 ; i<NX ; i++){
    //   for (int j= start ; end ; j++){
    //     ex[i][j] = ex[i][j] - 0.5*(hz[i][j]-hz[i][j-1]);
    //   }
    // }

    MPI_Gather(scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);


    // // free matrixes and array
    // free_matrix(ex, NX);
    // free_matrix(ey, NX);
    // free_matrix(hz, NX);
    // free(_fict_);

    MPI_Finalize();
    return 0;
}