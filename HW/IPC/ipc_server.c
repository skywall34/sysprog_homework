#include "ipc.h"
#include <signal.h>

#define USAGE_STRING "Invalid arguments.\n Use: ./ipc_server new_server_key\n"
#define INVALID_SERVER_KEY "Invalid server key. Please use a positive int\n"
#define MAX_CLIENTS 3 //max number of clients

//create message queue
int create_message_queue(int key){
	int queueID;
	if((queueID = msgget(key, IPC_CREAT | PERMISSIONS)) == -1){
		perror("mssget: Error creating msg queue\n");
		exit(EXIT_FAILURE);
	}

	return queueID;
}

//receive message and put in buffer
void receive_message(int msgqid, msgbuf * msgp, long mtype){
	int bytesRead = msgrcv(msgqid, msgp, sizeof(struct data_st), mtype, 0);
	if (bytesRead == -1){
		if(errno == EIDRM){ //timeout exception
			fprintf(stderr, "Message queue removed while waiting\n");
			exit(0);
		}
	}
}


//send message to client through message queue
void send_message(char message[MSGSTR_LEN], int msgqid, long to, long from){
	//to: send key to
	//from: send key from
	msgbuf new_msg;
	new_msg.mtype = to; //receiving server
	data_st ds;
	ds.source = from;
	ds.dest = to; //receving client
	char * null = "\0";
	int length  = strlen(message);
	if(MSGSTR_LEN < length){
		length = MSGSTR_LEN;
	}
	int i;
	//sending character at a time
	for(i = 0; i < length; i++){
		strncpy(ds.msgstr, &(message[i]), 1);
		new_msg.data = ds;
		//block the send to prevent errors
		int ret = msgsnd(msgqid, (void *)&new_msg, sizeof(data_st), 0);
		if(ret== -1){
			perror("msgsnd: error sending message\n");
			exit(EXIT_FAILURE);
		}
	}
	strncpy(ds.msgstr, null, 1);
	new_msg.data = ds;

	int ret = msgsnd(msgqid, (void *)&new_msg, sizeof(data_st), 0);
	if(ret == -1){
		perror("msgsnd: error sending message second error!\n");
		exit(EXIT_FAILURE);
	}
}


int main(int argc, char *argv[]){
	int queueID;
	int key;
	int clients[MAX_CLIENTS];
	int current_num_clients = 0;
	int i;

	msgbuf client_buffer_message[MAX_CLIENTS];

	if(argc==2){
		key = atoi(argv[1]);
		if(key <=0){
			printf(INVALID_SERVER_KEY);
			exit(-1);
		}
		queueID = create_message_queue(key);
	}
	else{
		printf(USAGE_STRING);
		exit(-1);
	}

	if(queueID < 0){
		printf("Failed to create queue.\n");
		exit(-1);
	}

	printf("Server connected! Waiting for clients....\n");
	printf("Connect client by running ./ipc_client %d new_client_key", key);

	while(1){
		//read message and print
		int to;
		int from;
		char message[MSGSTR_LEN];
		msgbuf tempbuf;
		receive_message(queueID, &tempbuf, key);

		to = tempbuf.data.dest;
		from = tempbuf.data.source;
		data_st * ds_pointer;
		strncpy(message, tempbuf.data.msgstr,1);
		if(current_num_clients == 0){
			ds_pointer = &(client_buffer_message[0].data);
			ds_pointer->dest = to;
			ds_pointer->source = from;
			strncpy(ds_pointer->msgstr, message, 1);
			clients[current_num_clients] = from;
			current_num_clients++;
		}
		else{
			int i = 0;
			int buff = -1;
			while(i < current_num_clients){
				if(clients[i]==from){
					buff = i;
					i++;
				}
			}
			if(buff != -1){
				ds_pointer=&(client_buffer_message[buff].data);
				ds_pointer->dest = to;
				ds_pointer->source = from;
				strcat(ds_pointer->msgstr,message);
				if(strcmp(message,"\0")==0){
					printf("message: &%s\n", ds_pointer->msgstr);
					if(ds_pointer->dest == key){
						//if exit
						if(strcmp(ds_pointer->msgstr,EXIT_STR)==0){
							printf("Exiting..\n");
							if(msgctl(queueID, IPC_RMID, NULL)==-1){
								if(errno == EIDRM){
									fprintf(stderr, "Message queue already removed\n");
								}
								else{
									perror("Error while removing message queue\n");
								}
							}
							exit(0);
						}
						if(strcmp(ds_pointer->msgstr, CONNECT_MSG)==0){
							printf("%ld client connected\n", ds_pointer->source);
						}
						else if(strcmp(ds_pointer->msgstr, DISCONNECT_MSG)==0){
							printf("%ld client disconnected\n", ds_pointer->source);
							while(i < current_num_clients){
								if(clients[i] == ds_pointer->source) clients[i] = -1;
							}
						}
						else{
							printf("Client %ld: \"%s\"\n", ds_pointer->source, ds_pointer->msgstr);
						}
					}
					else if(ds_pointer->dest == 0){
						printf("Client %ld: \"%s\"\n", ds_pointer->source, ds_pointer->msgstr);
						int j = 0;
						while (j < current_num_clients){
							if(clients[j] != -1){
								printf("sending message to %d from %ld: \"%s\"\n", clients[j], ds_pointer->source, ds_pointer->msgstr);
								send_message(ds_pointer->msgstr, queueID, clients[j], ds_pointer->source);
							}
							j++;
						}
					}
					else{
						printf("sending message to %ld from %ld: \"%s\"\n", ds_pointer->dest, ds_pointer->source, ds_pointer->msgstr);
						send_message(ds_pointer->msgstr, queueID, ds_pointer->dest, ds_pointer->source);
					}
					strncpy(ds_pointer->msgstr, "", MSGSTR_LEN);
				}
			}
			else{
				ds_pointer=&(client_buffer_message[current_num_clients].data);
				ds_pointer->dest = to;
				ds_pointer->source=from;
				strncpy(ds_pointer->msgstr, message, 1);
				clients[current_num_clients] = from;
				current_num_clients++;
			}
		}
	}

	//assume that msqid has been obtained beforehand

	if(msgctl(queueID, IPC_RMID, NULL)==-1){
		if(errno == EIDRM){
			fprintf(stderr, "Message queue already removed\n");
		}
		else{
			perror("Error removing message queue\n");
		}
	}
	return 0;
}
