#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "file.h"



struct fileHolder * getFile(char * fileName){
	int fd, status;
	struct fileHolder * file = malloc(sizeof(struct fileHolder));
	
	fd = open(fileName,O_RDONLY);
	if(fd == -1){
		fprintf(stderr,"Error opening file");
		exit(1);
	}
	
	struct stat * ret = malloc(sizeof(struct stat));
	status = fstat(fd,ret);
	if(status == -1){
		fprintf(stderr,"Error retrieving file status");
		exit(1);
	}
 	int length = ret->st_size;
 	file->size = length;
	free(ret);
	
	void  * buf = malloc(length*sizeof(char));
	ssize_t numRead = 0;
	while(length != 0){
		numRead = read(fd,(char *) buf+numRead,length);
		if(numRead == -1){
			if(errno == EINTR){
				continue;
			}
			fprintf(stderr,"read");
			exit(1);
		}
		if(numRead != length){
			continue;
		}	
		length -= numRead;
	}

	status = close(fd);
	if(status == -1){
		fprintf(stderr,"Error closing file");
		exit(1);
	}

	file->buffer = buf;
	return file;
}


void  putFile(char * fileName,struct fileHolder * file){
	int fd;
	fd = creat(fileName,0644);
	if(fd == -1){
		fprintf(stderr,"Error creating file for write");
		exit(1);
	}
	ssize_t numWritten = write(fd,file->buffer,file->size);
	if(numWritten != file->size){
		fprintf(stderr,"Partial write");
		exit(1);
	}
}

