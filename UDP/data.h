//File: data.h
#include <stdint.h>

struct Data{
        uint32_t sequence;
        void * data;
};

void data_to_buffer(struct Data * data,void * buf, int dataLength);

void buffer_to_data(struct Data * data,void * buf, int dataLength);

