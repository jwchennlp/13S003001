#ifndef __SOCKET_H__
#define __SOCKET_H__

struct InputSocket{
	int socketfd;
};

struct OutputSocket{
	int socketfd;
	struct addrinfo * address;
};

struct InputSocket *  getInputSocket(char * port);
struct OutputSocket * getOutputSocket(char * destination, char * port);

#endif
