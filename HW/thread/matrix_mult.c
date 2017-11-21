#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define thread_size 40
#define NUM 4000

long long int sum_result = 0;

//use double pointers for multi array
int **matrix_A;
int **matrix_B;
long long int **matrix_C; //result may be long

struct timespec beginning, end;
double elapsed_time = 0;

pthread_t threads[thread_size];

//initialize 
void *multi(void * matrix);
void error(char * message); //error handling
void get_sum(int row_beg, int row_end);

int main(int argc, char ** argv){
	int i, j; //for row and columns, respectively
	int fl; //thread flag

	struct condition *condition;

	FILE *fp1;
	FILE *fp2;

	//dynamic allocation of arrays
	matrix_A = (int **)malloc(sizeof(int *) * NUM);
	matrix_B = (int **)malloc(sizeof(int *) * NUM);
	matrix_C = (long long int **)malloc(sizeof(long long int *) * NUM);

	for(i = 0; i < NUM; i++){
		matrix_A[i] = (int *)malloc(sizeof(int)*NUM);
		matrix_B[i] = (int *)malloc(sizeof(int)*NUM);
		matrix_C[i] = (long long int *)malloc(sizeof(long long int)*NUM);
	}

	if(argc!=3){
		fprintf(stderr, "not enough arguments!");
	}

	if((fp1 = fopen(argv[1], "r"))==NULL){
		fprintf(stderr, "no file 1\n");
	}
	if((fp2 = fopen(argv[2], "r"))==NULL){
		fprintf(stderr, "no file 2\n");
	}

	//insert values 
	for(i = 0; i < NUM; i++){
		for(j = 0; j < NUM; j++){
			fscanf(fp1, "%d", &matrix_A[i][j]);
		}
	}

	for(i = 0; i < NUM; i++){
		for(j = 0; j < NUM; j++){
			fscanf(fp2, "%d", &matrix_B[i][j]);
		}
	}
	printf("input done\n");

	printf("Thread: %d\n\n", thread_size);

	//create thread
	for(i = 0; i < thread_size; i++){
		if((fl = pthread_create(&threads[i], NULL, multi,NULL))!=0){
			fprintf(stderr, "error creating thread\n");
		}
	}

	for(i = 0; i < thread_size; i++){
		if((fl = pthread_join(threads[i], NULL)) != 0){
			fprintf(stderr, "error joining thread\n");
		}
	}

	//get_sum();

	printf("sum: %lld\n", sum_result);

	for(i = 0; i< NUM; i++){
		free(matrix_A[i]);
		free(matrix_B[i]);
		free(matrix_C[i]);
	}
	free(matrix_A);
	free(matrix_B);
	free(matrix_C);

	fclose(fp1);
	fclose(fp2);

	return 0;
}

//multiply
void * multi (void * arg){
	int i, j, k;

	long long int result;

	pthread_t tid = pthread_self();

	for(i = 0; i < thread_size; i++){
		if(tid == threads[i]){
			//if current thread
			break;
		}
	}

	int row_beg = (NUM/thread_size) *i;
	int row_end = (NUM/thread_size)*(i+1);
	printf("%d thread calculates row %d to %d\n", i+1, row_beg, row_end);

	//set starting time of mult
	clock_gettime(CLOCK_MONOTONIC, &beginning);

	
	//multiplication
	for(i = row_beg; i < row_end; i++){
		for(j = 0; j < NUM; j++){
			result = 0;
			for(k = 0; k < NUM; k++){
				result += matrix_A[i][k] * matrix_B[k][j];
			}
			matrix_C[i][j] = result;
		}
	}

	get_sum(row_beg, row_end);
}


void get_sum(int row_beg, int row_end){
	//num 4 receive threaded multi and use as argument
	int i, j;

	for(i = row_beg; i < row_end; i++){
		for(j = 0; j < NUM; j++){
			sum_result += matrix_C[i][j];
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &end);

	//get time
	elapsed_time = (end.tv_sec - beginning.tv_sec);
	elapsed_time += (end.tv_nsec - beginning.tv_nsec) / 1000000000.0;//nano second

	printf("time: %f seconds\n", elapsed_time);
}

void error(char * error_mes){
	fprintf(stderr, "%s\n", error_mes);
	exit(1);
}
