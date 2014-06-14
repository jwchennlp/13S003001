#include "data.h"
#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void data_to_buffer(struct Data * data,void * buf, int dataLength){
	sprintf((char *) buf,"%010d",data->sequence);
	memcpy(buf+10,data->data,dataLength);
}	

void buffer_to_data(struct Data * data,void * buf, int dataLength){
	char dest[10];
	strncpy(dest,(char *) buf,10);
	data->sequence = strtol(dest,NULL,10);
	memcpy(data->data,buf+10,dataLength); 
}
