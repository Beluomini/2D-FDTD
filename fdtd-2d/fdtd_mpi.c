#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// only being used for the time function
#include <polybench.h>

#include <mpi.h>

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

int main(int argc, char** argv) {
    int process_Rank, size_Of_Cluster, message_Item;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size_Of_Cluster);
    MPI_Comm_rank(MPI_COMM_WORLD, &process_Rank);

    if(process_Rank == 0){
      for(int i = 0; i < size_Of_Cluster; i++){

        double** ex = matrix_alloc_data(NX, NY);
        double** ey = matrix_alloc_data(NX, NY);
        double** hz = matrix_alloc_data(NX, NY);
        double* _fict_ = vector_alloc_data(NY);

        init_array (tmax, nx, ny, ex, ey, hz, _fict_);

        // time function from polybench 
        polybench_start_instruments;

        for(int i = 0; i < size_Of_Cluster; i++){
            
            MPI_Send(ex, 1, MPI_INT, 1, 1, MPI_COMM_WORLD);
            printf("Message Sent: %d\n", message_Item);

        }



        // stop timer and print results
        polybench_stop_instruments;
        polybench_print_instruments;

        // free matrixes and array
        free_matrix(ex, NX);
        free_matrix(ey, NX);
        free_matrix(hz, NX);
        free(_fict_);

      }
    }

    else if(process_Rank == 1){
        MPI_Recv(&message_Item, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Message Received: %d\n", message_Item);
    }

    MPI_Finalize();
    return 0;
}