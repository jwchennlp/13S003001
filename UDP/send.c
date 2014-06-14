#include "file.h"
#include "socket.h"
#include "data.h"
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define RECEIVEPORT "9998"
#define SENDPORT "9999" 
#define PACKETSIZE  512

void sendFile(char * destination,char * filename){
//Initialize
	struct fileHolder * file = getFile(filename);

	struct InputSocket * in = getInputSocket(RECEIVEPORT);
	struct OutputSocket * out = getOutputSocket(destination,SENDPORT);

	struct Data * recData = malloc(sizeof(struct Data));
	struct Data * sendData = malloc(sizeof(struct Data));
	recData->data = malloc(PACKETSIZE*sizeof(char));
	sendData->data = malloc(PACKETSIZE*sizeof(char));

	char recBuf[PACKETSIZE+10];
	char sendBuf[PACKETSIZE+10];

	uint32_t sequence = 0;
	sendData->sequence = sequence;
	sprintf(sendData->data,"%d",(int)file->size);
	data_to_buffer(sendData,sendBuf,strlen((char *)sendData->data));

	int  counter = 0;
	fcntl(in->socketfd,F_SETFL,O_NONBLOCK);

	while(counter <= (file->size/PACKETSIZE)){
		int lastIteration = file->size/PACKETSIZE;
		int status = recvfrom(in->socketfd,recBuf,PACKETSIZE,1,NULL,0);
	//Resend if recieving socket would block or there are other errors
		if(status == -1){
			if(counter == lastIteration){
				status = sendto(out->socketfd,sendBuf,file->size-(counter*PACKETSIZE)+10,0,out->address->ai_addr,out->address->ai_addrlen);
			}else{
				status = sendto(out->socketfd,sendBuf,PACKETSIZE+10,0,out->address->ai_addr,out->address->ai_addrlen);
			}
			continue;
		}
	//Resend if the ACK response contains a different sequence number than the current
		buffer_to_data(recData,recBuf,status-10);
		if(sequence != recData->sequence){
			if(counter == lastIteration){
				status = sendto(out->socketfd,sendBuf,file->size-(counter*PACKETSIZE)+10,0,out->address->ai_addr,out->address->ai_addrlen);
			}else{
				status = sendto(out->socketfd,sendBuf,PACKETSIZE+10,0,out->address->ai_addr,out->address->ai_addrlen);
			}
			continue;
		}
	//Update the sequence number and payload and send
		sequence++;
		counter++;
		sendData->sequence = sequence;
		int offset = PACKETSIZE*(counter-1);
		if(counter > lastIteration){
			memcpy(sendData->data,file->buffer+offset,file->size-offset);
			data_to_buffer(sendData,sendBuf,file->size-offset);
			status = sendto(out->socketfd,sendBuf,file->size-offset+10,0,out->address->ai_addr,out->address->ai_addrlen);
		}else{
			memcpy(sendData->data,file->buffer+offset,PACKETSIZE);
			data_to_buffer(sendData,sendBuf,PACKETSIZE);
			status = sendto(out->socketfd,sendBuf,PACKETSIZE+10,0,out->address->ai_addr,out->address->ai_addrlen);
		}
	}
//Free
	free(recData->data);
	free(sendData->data);
	free(recData);
	free(sendData);
	freeaddrinfo(out->address);	
}


int main(int argc, char * argv[]){
	if( argc != 3){
		printf("Usage sender destination-ip filename");
		exit(1);
	}
	sendFile(argv[1],argv[2]);
	return 0;
}
