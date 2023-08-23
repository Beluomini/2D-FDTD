#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>

int TAMANHO_MATRIZ;
int NUMERO_THREADS;
float* matriz_a = NULL;
float* matriz_b = NULL;
float* matriz_rs = NULL;
float* matriz_rp = NULL;

void sequencial(){
	float sum = 0;
	for (int i=0;i<TAMANHO_MATRIZ;i++){
		for (int j=0;j<TAMANHO_MATRIZ;j++){
			sum = 0;
			for (int k=0;k<TAMANHO_MATRIZ;k++){
				sum += matriz_a[i*TAMANHO_MATRIZ + k] * matriz_b[k*TAMANHO_MATRIZ + j];
			}
			matriz_rs[i*TAMANHO_MATRIZ + j] = sum;
		}
	}
}

void* mm(void* arg){
	int start = *(int*)arg;
	float sum = 0;
	for (int i=start;i<start+TAMANHO_MATRIZ/NUMERO_THREADS;i++){
		for (int j=0;j<TAMANHO_MATRIZ;j++){
			sum = 0;
			for (int k=0;k<TAMANHO_MATRIZ;k++){
				sum += matriz_a[i*TAMANHO_MATRIZ + k] * matriz_b[k*TAMANHO_MATRIZ + j];
			}
			matriz_rp[i*TAMANHO_MATRIZ + j] = sum;
			
		}
	}
}

void fill_matrix(float* mat, int size){
	for(int i=0;i<size;i++){
		for (int j=0;j<size;j++){
			mat[i*size + j] = (float)rand()/RAND_MAX * 10; // 16776704 
		}
	}
}
void print_matrix(float* m, int size){
	for(int i=0;i<size;i++){
		printf("[");
		for (int j=0;j<size;j++){
			printf("%.2f ", m[i*size + j]);
		}
		printf("]");
		printf("\n");
	}
}

int main (int argc, char *argv[]){

	if (argc != 3){
		printf("Uso: %s <tamanho_matriz> <numero_threads>\n", argv[0]);
		exit(1);
	}
	TAMANHO_MATRIZ = atoi(argv[1]);
	NUMERO_THREADS = atoi(argv[2]);
	if (TAMANHO_MATRIZ <= 0 || NUMERO_THREADS <= 0) {
        printf("O tamanho da matriz e o número de threads devem ser maiores que zero.\n");
        return 1;
    }
    if (TAMANHO_MATRIZ < NUMERO_THREADS){
        printf("O tamanho da matriz deve ser menor ou igual ao numero de threads.\n");
        return 1;
    }

	int i;
	pthread_t* threads;
	int* linhas;
	struct timeval inicio, fim;
	long tempo_seq, tempo_par;

	matriz_a =  (float*)malloc(sizeof(float)*TAMANHO_MATRIZ*TAMANHO_MATRIZ);
	matriz_b =  (float*)malloc(sizeof(float)*TAMANHO_MATRIZ*TAMANHO_MATRIZ);
	matriz_rp = (float*)malloc(sizeof(float)*TAMANHO_MATRIZ*TAMANHO_MATRIZ);

	fill_matrix(matriz_a, TAMANHO_MATRIZ);
	fill_matrix(matriz_b, TAMANHO_MATRIZ);	

	gettimeofday(&inicio, NULL);

	threads = (pthread_t*)malloc(sizeof(pthread_t)*NUMERO_THREADS);
	linhas = (int*)malloc(sizeof(int)*NUMERO_THREADS);
	for (i = 0; i < NUMERO_THREADS; i++){
		linhas[i] = i*TAMANHO_MATRIZ/NUMERO_THREADS;
		pthread_create(&threads[i], NULL, mm, (void *)&linhas[i]);	
	}
	for (i = 0; i < NUMERO_THREADS; i++){
		pthread_join(threads[i], NULL);
	}
	gettimeofday(&fim, NULL);
	tempo_par = (fim.tv_sec - inicio.tv_sec) * 1000000L + (fim.tv_usec - inicio.tv_usec);

	matriz_rs = (float*)malloc(sizeof(float)*TAMANHO_MATRIZ*TAMANHO_MATRIZ);

	gettimeofday(&inicio, NULL);
	
	sequencial();
	
	gettimeofday(&fim, NULL);
	tempo_seq = (fim.tv_sec - inicio.tv_sec) * 1000000L + (fim.tv_usec - inicio.tv_usec);

	free(matriz_a);
	free(matriz_b);
	free(matriz_rs);
	free(matriz_rp);
	free(threads);
	free(linhas);

	float tempo_seq_seg = tempo_seq/(float)1000000;
	float tempo_par_seg = tempo_par/(float)1000000;
	float speedup = tempo_seq_seg/tempo_par_seg;
	printf("Tempo de execucao sequencial: %f\n", tempo_seq_seg);
	printf("Tempo de execucao paralelo:   %f\n", tempo_par_seg);
	printf("SpeedUp: %f\n", speedup);
	return 0;
}