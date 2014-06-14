#ifndef __buffer_h__
#define __buffer_h__
#include <sys/types.h>

struct fileHolder{
	ssize_t size;
	void * buffer;
};

struct fileHolder * getFile(char * name);
void putFile(char * name,struct fileHolder *);

#endif
