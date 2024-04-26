#ifndef KEE_CHUNK_H_
#define KEE_CHUNK_H_

#include <stddef.h>

#ifndef KEE_TRANSPORT_CHUNK_MAX_SIZE
#define KEE_TRANSPORT_CHUNK_MAX_SIZE 768
#endif

struct kee_chunk_t {
	char *data;
	size_t data_len;
	size_t crsr;
	short cardinality;
	short number;
};

#endif // KEE_CHUNK_H_
