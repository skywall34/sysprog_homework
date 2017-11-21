#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM 4000


int main(){
	int row, col;
	static int matrix[NUM][NUM];
	int random_num;
	FILE* fp;
	fp = fopen("matrixB.txt", "w+");
	if(fp == NULL){
		perror("could not open input file\n");
		exit(1);
	}
	srand(time(NULL));
	for(int i = 0; i < NUM; i++){
		for(int j = 0; j < NUM; j++){
			random_num = 1000+(rand()%9000);
			matrix[i][j] = random_num;
		}
	}		
	for(row = 0; row < NUM; row++){
		for(col = 0; col < NUM; col++){
			fprintf(fp, "%d ", matrix[row][col]);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
	
	return 0;
}
