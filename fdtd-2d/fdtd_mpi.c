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
    
    printf("aa\n");
    printf("process %d\n", process_Rank);

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

    }
    // TODO7?: apenas o 0 esta passando, seg fault nos outros
    printf("Processo %d passou aq", process_Rank);

    // ideia de logica para mpi:
    // scatter de ambos ex e ey para todos os processos
    // considerando 8x8, 4 processos, 2 iteracoes
    // temos que cada processo recebe 2 linhas de ex e ey
    // sendo 0 o pai, ele recebera ey 0 e 1, e ex 0 e 1
    // assim ele consegue fazer a conta de ey que depende de _fict_
    // apos cada processo realizar as operacoes nos seus vetores ex/ey
    // teremos uma barreira para garantir que todos terminem
    // seguindo a conta hz[i][j] = hz[i][j] - 0.7*  (ex[i][j+1] - ex[i][j] + ey[i+1][j] - ey[i][j]);
    // hz depende de hz i j, ex i j+1, ex i j, ey i+1 j, ey i j
    // ou seja, depende de i, j, i+1 e j+1, logo, precisamos de uma linha a mais
    // e que o for duplo va de 0 a nx-1 e ny-1
    // ja que cada processo tem seu pedaco de nx e ny precisamos apenas que o processo subsequente envie
    // a linha extra** para o processo anterior, isto é, o processo 1 envia a linha 0 e por ai vai
    // ideias para lidar com a linha/coluna extra: um send e receive p cada linha por processo
    // considerando o problema acima resolvido, teriamos no final cada processo com seu pedaco de hz
    // teriamos entao, outra barreira, para garantir que o hz termine de ser calculado
    // necessitamos das barreiras pois precisamos enviar uma linha entre os processos durante a execucao
    // após tudo isso, temos o hz separado em cada processo, podemos junto tudo no 0 para retornar ao usuario
    // ou continuar dividido entre os processos e comecar a proxima iteracao
    // ** -> a linha se refere apenas a ey que depende de i+1 linha, ex depende j+1 coluna, que ja foi passado

    //TODO1: declara e aloca aqui?? todos processos estao fazendo isso
    //TODO2: alterar nome dos vetores, muito grande, confuso no meio do codigo
    double *scattered_Data_ey = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *scattered_Data_ex = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *scattered_Data_hz = (double*)malloc(NX * (NY/size_Of_Cluster) * sizeof(double));
    double *extra_ey =          (double*)malloc(NX * sizeof(double));

    int start = 0;
    int end = NY/size_Of_Cluster;
    int last = (end-start)*NY;
    
    // Envia os dados de Ey para os processos com excessão dos dados que o processo 0 vai usar
    printf("processo 0 enviando %f para todos os processos com start %d e end %d\n", ey[end], start, end);
    // enviando e recebendo corretamente para cada processo
    // cada processo guarda os elementos em sua matrix 'scattered_data_XX', de 0 a 'last'
    MPI_Scatter(ey, (end-start)*NY, MPI_DOUBLE, scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(ex, (end-start)*NY, MPI_DOUBLE, scattered_Data_ex, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Scatter(hz, (end-start)*NY, MPI_DOUBLE, scattered_Data_hz, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    printf("processo %d recebeu 1 elemento %f e ultimo elemento %f  (ey)\n", process_Rank, scattered_Data_ey[0], scattered_Data_ey[last-1]);
    printf("processo %d recebeu 1 elemento %f e ultimo elemento %f  (ex)\n", process_Rank, scattered_Data_ex[0], scattered_Data_ex[last-1]);
    printf("processo %d recebeu 1 elemento %f e ultimo elemento %f  (hz)\n", process_Rank, scattered_Data_hz[0], scattered_Data_hz[last-1]);

    // if process 0, calculate first line of ey, depending on _fict_
    // TODO3: separar o if/else, deixar apenas o calculo do _fict_ dentro do if
    // pois so o prank 0 faz esse calculo, o resto é repetido entre todos processos
    if (process_Rank == 0){
      printf("Processo 0 comecou trabalho\n");
      for (int f = 0; f < NY; f++){
        // como esta com 1 iteracao, t pode ser 0 fixo, mudar depois 
        
        scattered_Data_ey[f] = _fict_[0];  
        
        //printf("sdy %f, fict %f\n", scattered_Data_ey[f], _fict_[0]);
      }

      for (int i=start+1 ; i<end ; i++){
        for (int j=0 ; j<NY ; j++){
          //printf("1Acessando [%d][%d] ou %d, valor em ey, hz: %f, %f\n", i, j, (i*NX)+j, scattered_Data_ey[(i*NX)+j], scattered_Data_hz[(i-1*NX)+j]);
          //printf("1Acessado %d %d -> %d %d, %d %d\n", i, j, i*NX, j, (i-1)*NX, j);
          
          //scattered_Data_ey[(i*NX)+j] = scattered_Data_ey[(i*NX)+j] - 0.5*(scattered_Data_hz[(i*NX)+j]-scattered_Data_hz[((i-1)*NX)+j]);
          
        }
      }

      // for loops condition inverted?
      for (int i=start ; i<end ; i++){
        for (int j= 1 ; j < NX ; j++){
          //printf("2Acessando [%d][%d] ou %d, valor em ex, hz: %f, %f\n", i, j, (i*NX)+j, scattered_Data_ex[(i*NX)+j], scattered_Data_hz[(i*NX)+j-1]);
          
          //scattered_Data_ex[(i*NX)+j] = scattered_Data_ex[(i*NX)+j] - 0.5*(scattered_Data_hz[(i*NX)+j]-scattered_Data_hz[(i*NX)+j-1]);
          
        }
      }

      //MPI_Barrier(MPI_COMM_WORLD);

      // precisa receber a ultima linha do proximo processo, 0 nao envia para ninguem
      // processar ate a penultima linha, a ultima faz separado do for
      // pois ira guardar a ultima linha do proximo processo em um array separado**
      // ** -> acredito que nao tenha como guardar no proprio vetor scattered, uma vez que o 
      // tamanho ja foi definido e alocado, porem, pode se alocar contando-se com 1 linha a mais

      // apenas recebe pois é o primeiro processo. Sempre recebe do prank 1
      
      //MPI_Recv(extra_ey, NX, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      // agora é feito o calculo do hz

      // considerando que temos o extra_ey, fazemos a conta com for até NY-2 ao inves de NY-1 ** outdated
      // e damos hard code na ultima linha
      for (int i = 0; i < end - 1; i++){    // trabalho vai ded 0 ate end(linhas que cada processo possui)
        for (int j = 0; j < NY - 1; j++){
          //printf("Acessando [%d][%d] ou %d, valor em ex, ey: %f, %f\n", i, j, (i*NX)+j, scattered_Data_ex[(i*NX)+j+1], scattered_Data_ex[(i*NX)+j]);
          
          //scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + scattered_Data_ey[((i+1)*NX)+j] - scattered_Data_ey[(i*NX)+j]);
          
        }
      }
      int i = end - 1;
      for (int j = 0; j < NY - 1; j++){
        // pelo fato de extra_ey ser apenas 1 linha apenas indexamos o j, o scattered_ey continua ali
        //printf("Acessando [%d][%d] ou %d, valor em ex, ey: %f, %f\n", i, j, (i*NX)+j, scattered_Data_ex[(i*NX)+j+1], scattered_Data_ex[(i*NX)+j]);
        
        //scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + extra_ey[j] - scattered_Data_ey[(i*NX)+j]);
        
      }
      // temos outra barreira para garantir que todos estao na mesma 'pagina'
      
      //MPI_Barrier(MPI_COMM_WORLD);

      // na teoria, nao precisariamos fazer mais nada, apenas continuar com a operacao, 
      // talvez, quem sabe, incerto, por uma chance, podemos tirar esta ultima barreira, uma vez 
      // que a unica dependencia é da linha extra do ey, porem a 1 barreira garante que todos cheguem
      // lá e esperem para prosseguir, garantindo que todos tenham a linha extra do ey para trabalhar

    }
    else{
      // erro pois para calcular ex e ey precisamos de hz i-1, que nao esta no processo
      // alem disso verificar pois nao executa a ultima linha o processo que recebeu o ultimo
      // subsets de linha, o resto calcula normal
      for (int i=start+1 ; i<end ; i++){  
        for (int j=0 ; j<NX ; j++){
          // aqui precisamos de uma linha extra, exemplo: para calculo das linhas 2 e 3
          // precisamos de hz[1], hz[2] e hz[3]

          //printf("acessado: %d %d -> %d %d, %d %d\n", i, j, i*NX, j, (i-1)*NX, j);
          
          //scattered_Data_ey[(i*nx)+j] = scattered_Data_ey[(i*nx)+j] - 0.5*(scattered_Data_hz[(i*nx)+j]-scattered_Data_hz[((i-1)*NX)+j]);
          
        }
      }

      for (int i=start ; i<end ; i++){
        for (int j=1 ; j < NX ; j++){
          //printf("Acessando [%d][%d] ou %d, valor em ex, hz: %f, %f\n", i, j, (i*NX)+j, scattered_Data_ex[(i*NX)+j], scattered_Data_hz[(i*NX)+j-1]);
          
          //scattered_Data_ex[(i*nx)+j] = scattered_Data_ex[(i*nx)+j] - 0.5*(scattered_Data_hz[(i*nx)+j]-scattered_Data_hz[(i*nx)+j-1]);
          
        }
      }

      //MPI_Barrier(MPI_COMM_WORLD);


      // caso for o ultimo processo, nao recebe, apenas envia
      // a linha enviada é a 1 de cada processo, que será usada como a ultima no processo anterior
      if (process_Rank == size_Of_Cluster-1){
        //MPI_Send(scattered_Data_ey, NX, MPI_DOUBLE, process_Rank-1, 0, MPI_COMM_WORLD);
      } 
      else{
        // todos exceto prank 0 e max-1 enviam e recebem
        // sem index no scattered_ey pois o 1 elemento é o que queremos enviar
        
        //MPI_Send(scattered_Data_ey, NX, MPI_DOUBLE, process_Rank-1, 0, MPI_COMM_WORLD);
        //MPI_Recv(extra_ey, NX, MPI_DOUBLE, process_Rank+1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }

      //TODO5: verificar se o calculo do hz esta correto, provavelmente nao
      for (int i = 0; i < end - 1; i++){
        for (int j = 0; j < NY - 1; j++){
          //scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + scattered_Data_ey[((i+1)*NX)+j] - scattered_Data_ey[(i*NX)+j]);
          
        }
      }
      int i = end - 1;
      for (int j = 0; j < NY - 1; j++){
        // pelo fato de extra_ey ser apenas 1 linha apenas indexamos o j, o scattered_ey continua ali
        
        //scattered_Data_hz[(i*NX)+j] = scattered_Data_hz[(i*NX)+j] - 0.7*  (scattered_Data_ex[(i*NX)+j+1] - scattered_Data_ex[(i*NX)+j] + extra_ey[j] - scattered_Data_ey[(i*NX)+j]);
      }
      //MPI_Barrier(MPI_COMM_WORLD);

    }







    MPI_Gather(scattered_Data_ey, (end-start)*NY, MPI_DOUBLE, ey, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(scattered_Data_ex, (end-start)*NY, MPI_DOUBLE, ex, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(scattered_Data_hz, (end-start)*NY, MPI_DOUBLE, hz, (end-start)*NY, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    if (process_Rank == 0){
      //print_array(ey, NX, NY);
      
      // TODO6?: nao esta limpando a memoria, verificar onde colocar e nao esquecer!
      // // free matrixes and array
      //free_matrix(ex, NX);
      //free_matrix(ey, NX);
      //free_matrix(hz, NX);
      //free(_fict_);
    }
    

    MPI_Finalize();
    return 0;
}