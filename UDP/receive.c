#include "file.h"
#include "socket.h"
#include "data.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#define RECEIVEPORT "9999"
#define SENDPORT "9998"
#define PACKETSIZE 512

void receiveFile(char * destination,char * filename){
//Initialize
	struct InputSocket * in = getInputSocket(RECEIVEPORT);
	struct OutputSocket * out = getOutputSocket(destination,SENDPORT);

	struct Data * recData = malloc(sizeof(struct Data));
	struct Data * sendData = malloc(sizeof(struct Data));
	recData->data = malloc(PACKETSIZE*sizeof(char));
	sendData->data = malloc(PACKETSIZE*sizeof(char));

	char recBuf[PACKETSIZE+10];
	char sendBuf[PACKETSIZE+10];

//Receive first packet with filesize information
	int status;
	uint32_t sequence = 0;
	receive: status = recvfrom(in->socketfd,recBuf,PACKETSIZE+10,1,NULL,0);
	buffer_to_data(recData,recBuf,status-10);
	if((status == -1) || (recData->sequence != 0)){
		fprintf(stderr,"Receive failure\n");
		goto receive;
	}
	sequence++;

//Allocate a buffer for input
	int filesize = strtol(recData->data,NULL,10);
	void * fileBuffer = malloc(filesize*sizeof(char));

	int count = 0;
	while(count <= filesize/PACKETSIZE){
		status = recvfrom(in->socketfd,recBuf,PACKETSIZE+10,1,NULL,0);
	//If there are errors receive again
		if(status == -1){
			continue;
		}	
	//Extract data
		buffer_to_data(recData,recBuf,status-10);
		if(sequence == recData->sequence){
			sequence++;
			if(count == filesize/PACKETSIZE){
				memcpy(fileBuffer+(PACKETSIZE*count),recData->data,filesize-(PACKETSIZE*count));
			}else{
				memcpy(fileBuffer+(PACKETSIZE*count),recData->data,PACKETSIZE);				
			}
			count++;
		}
	//Acknowledge receipt
		sendData->sequence = recData->sequence;
		data_to_buffer(sendData,sendBuf,0);
		sendto(out->socketfd,sendBuf,10,0,out->address->ai_addr,out->address->ai_addrlen);
	}
//Write to disk
	struct fileHolder * file = malloc(sizeof(struct fileHolder));
	file->buffer = fileBuffer;
	file->size = filesize;
	putFile(filename,file);
//Free
	free(recData->data);
	free(sendData->data);
	free(recData);
	free(sendData);		
	freeaddrinfo(out->address);
	free(fileBuffer);
}
	

int main(){
	receiveFile("127.0.0.1","received_file");
	return 0;
}
