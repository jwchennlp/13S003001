#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include "socket.h"


struct InputSocket * getInputSocket(char * port){
        struct addrinfo hints;
	struct addrinfo * results = malloc(sizeof(struct addrinfo));
        int socketfd, status;
        memset(&hints,0,sizeof(hints));
       hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_flags = AI_PASSIVE;
        status = getaddrinfo(NULL,port,&hints,&results);
        if(status != 0){
                fprintf(stderr,"getaddrinfo: %s",gai_strerror(status));
                exit(1);
        }
        socketfd = socket(results->ai_family,results->ai_socktype,results->ai_protocol);
        if(socketfd == -1){
                fprintf(stderr,"failed to create socket");
                exit(1);
        }
        status = bind(socketfd,results->ai_addr,results->ai_addrlen);
        if(status == -1){
                fprintf(stderr,"failed to bind input socket");
                exit(1);
        }
        freeaddrinfo(results);
	struct InputSocket * ret = malloc(sizeof(struct InputSocket));
	ret->socketfd = socketfd;
        return ret;
}


struct OutputSocket * getOutputSocket(char * destination, char * port){
        struct addrinfo hints;
	struct addrinfo * results; //= malloc(sizeof(struct addrinfo));
        int socketfd, status;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_DGRAM;
	
        status = getaddrinfo(destination,port,&hints,&results);
        if(status != 0){
                fprintf(stderr,"getaddrinfo: socket %s",gai_strerror(status));
                exit(1);
        }
        socketfd = socket(results->ai_family,results->ai_socktype,results->ai_protocol);
        if(socketfd == -1){
                fprintf(stderr,"failed to create socket");
                exit(1);
        }
/*       status = bind(socketfd,results->ai_addr,results->ai_addrlen);
        if(status == -1){
               fprintf(stderr,"failed to bind socket");
                exit(1);
        } */
	struct OutputSocket * ret = malloc(sizeof(struct OutputSocket));
	ret->socketfd = socketfd;
	ret->address = results;
	return ret;
}


