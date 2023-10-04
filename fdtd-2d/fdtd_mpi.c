#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

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


int main(int argc, char** argv) {
    int process_Rank, size_Of_Cluster, message_Item;

    struct timeval inicio, fim;
    long t_exec;

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
      // ./fdtd -d <DATASET> -t <NUMBER_PROCESS>
    } else if (argc == 3){
      // <DATASET> 
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

      //printf("\n---------EX----------\n");
      //print_array(ey, NX, NY);
      //printf("\n---------EY----------\n");
      //print_array(ex, NX, NY);
      //printf("\n---------HZ----------\n");
      //print_array(hz, NX, NY);
      //printf("\nsize of cluster: %d\n\n", size_Of_Cluster);

    }

    double *scattered_Data_ey = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *scattered_Data_ex = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *scattered_Data_hz = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *extra_ey =          (double*)malloc(NX * sizeof(double));
    double *extra_hz =          (double*)malloc(NX * sizeof(double));

    
    if (process_Rank == 0){
      gettimeofday(&inicio, NULL);
    }
    
    int start = 0;
    int end = NY/size_Of_Cluster;
    
    MPI_Scatter(ey, (end-start)*NY, MPI_DOUBLE, scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(ex, (end-start)*NY, MPI_DOUBLE, scattered_Data_ex, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(hz, (end-start)*NY, MPI_DOUBLE, scattered_Data_hz, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int t = 0; t < TMAX; t++){

      if (process_Rank == 0){
        for (int f = 0; f < NY; f++){
          scattered_Data_ey[f] = _fict_[t];  
        }

        MPI_Send(&scattered_Data_hz[(end-start-1)*NX], NX, MPI_DOUBLE, process_Rank+1, 0, MPI_COMM_WORLD);

        for (int i=start+1 ; i<end ; i++){
          for (int j=0 ; j<NX ; j++){
            
            scattered_Data_ey[(i*NX)+j] = scattered_Data_ey[(i*NX)+j] - 0.5*(scattered_Data_hz[(i*NX)+j]-scattered_Data_hz[((i-1)*NX)+j]);
            
          }
        }

        for (int i=start ; i<end ; i++){
          for (int j= 1 ; j < NX ; j++){
            scattered_Data_ex[(i*NX)+j] = scattered_Data_ex[(i*NX)+j] - 0.5*(scattered_Data_hz[(i*NX)+j]-scattered_Data_hz[(i*NX)+j-1]);
            
          }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Recv(extra_ey, NX, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 0; i < end - 1; i++){    
          for (int j = 0; j < NX - 1; j++){
            scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + scattered_Data_ey[((i+1)*NX)+j] - scattered_Data_ey[(i*NX)+j]);
          }
        }
        int i = end - 1;
        for (int j = 0; j < NX - 1; j++){
          scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + extra_ey[j] - scattered_Data_ey[(i*NX)+j]);
        }
        
        MPI_Barrier(MPI_COMM_WORLD);

      }
      else{
        if (process_Rank != size_Of_Cluster-1){
          MPI_Send(&scattered_Data_hz[(end-start-1)*NX], NX, MPI_DOUBLE, process_Rank+1, 0, MPI_COMM_WORLD);
        }
        MPI_Recv(extra_hz, NX, MPI_DOUBLE, process_Rank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        int i = start;
        for (int j = 0; j<NX; j++){
          scattered_Data_ey[(i*nx)+j] = scattered_Data_ey[(i*nx)+j] - 0.5*(scattered_Data_hz[(i*nx)+j]-extra_hz[j]);
        }
        for (int i=start+1 ; i<end ; i++){  
          for (int j=0 ; j<NX ; j++){
            scattered_Data_ey[(i*nx)+j] = scattered_Data_ey[(i*nx)+j] - 0.5*(scattered_Data_hz[(i*nx)+j]-scattered_Data_hz[((i-1)*NX)+j]);
          }
        }

        for (int i=start ; i<end ; i++){
          for (int j=1 ; j < NX ; j++){
            scattered_Data_ex[(i*nx)+j] = scattered_Data_ex[(i*nx)+j] - 0.5*(scattered_Data_hz[(i*nx)+j]-scattered_Data_hz[(i*nx)+j-1]);
          }
        }

        MPI_Barrier(MPI_COMM_WORLD);

        if (process_Rank == size_Of_Cluster-1){
          MPI_Send(scattered_Data_ey, NX, MPI_DOUBLE, process_Rank-1, 0, MPI_COMM_WORLD);
        } 
        else{
          MPI_Send(scattered_Data_ey, NX, MPI_DOUBLE, process_Rank-1, 0, MPI_COMM_WORLD);
          MPI_Recv(extra_ey, NX, MPI_DOUBLE, process_Rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        for (int i = 0; i < end - 1; i++){
          for (int j = 0; j < NX - 1; j++){
            scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + scattered_Data_ey[((i+1)*NX)+j] - scattered_Data_ey[(i*NX)+j]);
            
          }
        }
        if (process_Rank != size_Of_Cluster-1){
          i = end - 1;
          for (int j = 0; j < NX - 1; j++){
          scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + extra_ey[j] - scattered_Data_ey[(i*NX)+j]);
          }
        }

        MPI_Barrier(MPI_COMM_WORLD);
      }
    }



    MPI_Gather(scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(scattered_Data_ex, (end-start)*NY, MPI_DOUBLE, ex, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(scattered_Data_hz, (end-start)*NY, MPI_DOUBLE, hz, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (process_Rank == 0){
      //print_array(hz, NX, NY);
      gettimeofday(&fim, NULL);
      t_exec = (fim.tv_sec - inicio.tv_sec) * 1000000L + (fim.tv_usec - inicio.tv_usec);
      float t_exec_segundos = t_exec/(float)1000000;
      printf("%f\n", t_exec_segundos);
      
      // free matrixes and array
      free(ex);
      free(ey);
      free(hz);
      free(_fict_);
    }
    free(scattered_Data_ey);
    free(scattered_Data_ex);
    free(scattered_Data_hz);
    free(extra_ey);
    free(extra_hz);


    MPI_Finalize();
    return 0;
}