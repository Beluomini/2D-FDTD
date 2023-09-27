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
        printf("%.2f ", array[i*nx+j]);
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
          NX = 6;
          NY = 6;
        } else if (!strcmp(argv[2],"medium")){
          TMAX = 1;
          NX = 6;
          NY = 6;
        } else if (!strcmp(argv[2],"large")){
          TMAX = 1;
          NX = 6;
          NY = 6;
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

    double* ex = matrix_alloc_data(NX, NY);
    double* ey = matrix_alloc_data(NX, NY);
    double* hz = matrix_alloc_data(NX, NY);
    double* _fict_ = vector_alloc_data(NY);

    double scattered_Data_ex;
    double scattered_Data_ey;

    if(process_Rank == 0){

      init_array (tmax, nx, ny, ex, ey, hz, _fict_);

      print_array(ey, NX, NY);
      printf("\n---------------------\n");
      print_array(ex, NX, NY);
      printf("\n---------------------\n");
      print_array(hz, NX, NY);
      printf("\nsize of cluster: %d\n", size_Of_Cluster);

      // time function from polybench 
      //polybench_start_instruments;

      for(int t = 0; t < tmax; t++){
        
        int start = (NY/(size_Of_Cluster/2)) - (NY/(size_Of_Cluster/2))/size_Of_Cluster;

        int end = NY/(size_Of_Cluster/2);

        printf("id: %d\n", id);

        MPI_Scatter(&ey[end], (end-start)*NY, MPI_DOUBLE, &scattered_Data_ey, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        start = 0;

        end = NX/(size_Of_Cluster/2);

        MPI_Scatter(&ex, (end-start)*NX, MPI_DOUBLE, &scattered_Data_ex, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        for (int f = 0; f < NY; f++)
          ey[0][f] = _fict_[t];

        for (int i=start ; i<end ; i++){
          for (int j=0 ; j<NY ; j++){
            ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
          }
        }

        for (int i = 1; i < size_Of_Cluster; i++){

          if(i < size_Of_Cluster/2){

            // RECEBE Dos processos

          }else{

            // RECEBE Dos processos

          }

        }

        //barreira
        //MPI_Barrier(MPI_COMM_WORLD);

        print_array(ey, NX, NY);
        printf("\n---------------------\n");
        print_array(ex, NX, NY);
        printf("\n---------------------\n");
        print_array(hz, NX, NY);
        printf("\nsize of cluster: %d\n", size_Of_Cluster);

      }

      //stop timer and print results
      //polybench_stop_instruments;
      //polybench_print_instruments;

      // free matrixes and array
      free_matrix(ex, NX);
      free_matrix(ey, NX);
      free_matrix(hz, NX);
      free(_fict_);

    }else{

      int start, end;

      for(int t = 0; t < tmax; t++){

        if(process_Rank < size_Of_Cluster/2){

          // recebe do process rank 0

          for (int i=start ; i<end ; i++){
            for (int j=0 ; j<NY ; j++){
              ey[i][j] = ey[i][j] - 0.5*(hz[i][j]-hz[i-1][j]);
            }
          }
              
          // envia pro process rank 0

          //barreira
          //MPI_Barrier(MPI_COMM_WORLD);


        }else{

          // recebe do process rank 0

          for (int i=0 ; i<NX ; i++){
            for (int j= start ; end ; j++){
              ex[i][j] = ex[i][j] - 0.5*(hz[i][j]-hz[i][j-1]);
            }
          }

          // envia pro process rank 0

          //barreira
          //MPI_Barrier(MPI_COMM_WORLD);
          
        }

        

      }

    }

    
    MPI_Finalize();
    return 0;
}