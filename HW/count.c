#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define BUFFER_SIZE 4096


int read_data(int fd, char *buffer, int buf_size);
int write_data(int fd, const char *buffer, int buf_size);
void increment(char *buffer);
int start_rw(int rfd, int wfd, char *buffer);
void child_handler(pid_t current_proc, pid_t new_proc);


int main(int argc, char **argv){
	
	//fork to create new process P1 P2 P3
	//kill() to send signal to any new process
	//get from file sample.txt, write to file using fprintf
	//problem: current pid is the last forked pid, no other child does work
	if(argc==1){
		fprintf(stderr, "error! too few arguments");
		return 1;
	}
	else if(atoi(argv[1]) <= 0){
		fprintf(stderr, "first argument needs to be an integer larger than 0!\n");
		return 1;
	}
	
	
	int rfd;
	int wfd;
	int len;
	char buffer[BUFFER_SIZE];
	pid_t pid;
	pid_t ppid;
	int iter = atoi(argv[1]);
	int i;
	pid_t proc[3]; //processes
	int status = 0;
	int num = 0;

	//read file open
	if((rfd = open(argv[2], O_RDONLY))==-1){
		fprintf(stderr, "file read open error: %s\n", strerror(errno));
		return 1;
	}

	//write file open, if none, create
	if((wfd = open(argv[2], O_WRONLY | O_CREAT, 0644)) == -1){
		fprintf(stderr, "file write open error: %s\n", strerror(errno));
		return 1;
	}
	printf("iter: %d\n", iter);

	for(i=0; i < 3; i++){
		pid = fork();
		if(pid < 0){
			fprintf(stderr, "fork error\n");
			exit(1);
		} else if (pid == 0){
			printf("Current Process: %d\n", getpid());
			proc[i] = getpid();
			printf("proc[%d]: %d\n", i, proc[i]);
		}else{
			while((ppid = wait(&status))>0);
			//break;
		}
	}
	
	printf("working process: %d\n", getpid());
	//kill(proc[0], SIGCONT);
	while(num < iter/3){
		int i = 1;
		int return_num = start_rw(rfd, wfd, buffer);
		if(return_num ==1){
			fprintf(stderr, "Problem with read or write");
			break;
		}
		num++; //sums up to iter
		
		if(num==iter){
			i = 3;
		}
		pause();
		kill(proc[i], SIGCONT);
		if(i < 2){
			continue;
		} else{
			break;
		}
		
	}

		
	close(rfd);
	close(wfd);
	
	return 0;
}

int read_data(int fd, char *buffer, int buf_size)
{
	int size = 0;
	int len;

	while(1){
		if((len = read(fd, &buffer[size], buf_size-size)) > 0){
			size += len;
			if(size == buf_size){
				return size;
			}
		} else if (len == 0){
			return size;
		} else{
			if(errno == EINTR){
				continue;
			} else {
				return -1;
			}
		}
	}
}

int write_data(int fd, const char *buffer, int buf_size)
{
	int size = 0;
	int len;

	while(1){
		if((len = write(fd, &buffer[size], buf_size-size))>0){
			size+=len;
			if(size == buf_size){
				return size;
			}
		} else if(len == 0){
			return size;
		} else{
			if(errno == EINTR){
				continue;
			} else{
				return -1;
			}
		}
	}
}

void increment(char *buffer)
{
	int number = atoi(buffer); //change to int
	printf("Incremented Value: %d\n", ++number); //printf the incremented value
	sprintf(buffer, "%d", number); //change back to char
}

void child_handler(pid_t current_proc, pid_t new_proc)
{
	kill(current_proc, SIGSTOP);
	kill(new_proc, SIGCONT);
}

int start_rw(int rfd, int wfd, char *buffer){
	//conduct read_data and write_data
        //read data
	int len;
        if((len = read_data(rfd, buffer, 4096))<0){
		fprintf(stderr, "read error: %s\n", strerror(errno));
                return 1;
        }
        if(len == 0){
		printf("EOF!");
		return 0;
        }
        //increment data
        increment(buffer);
        //go to beginning and write data
        lseek(wfd,0,SEEK_SET);
        if((len=write_data(wfd, buffer,len))<0){
                fprintf(stderr, "write error: %s\n", strerror(errno));
                return 1;
        }
        //set back to beginning for read
        lseek(rfd, 0, SEEK_SET);
}

