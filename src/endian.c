#include "endian.h"

int is_le() {
	unsigned short s = 42;
	return *((unsigned char*)&s) == 42;
}

// convert unsigned integer to little-endian representation
int to_endian(char direction, int l, void *n) {
	union le un;

	if (l == 1 || is_le() == direction) {
		return 0;
	}
	switch(l) {
		case sizeof(long long):
			un.ll = (long long*)n;	
			break;
		case sizeof(int):
			un.i = (int*)n;	
			break;
		case sizeof(short):
			un.s = (short*)n;	
			break;
		default:
			return 1;
	}
	flip_endian(l, &un);

	return 0;
}

void flip_endian(int l, union le *n) {
	int i;
	char t;
	unsigned char *ne;

	ne = (n->c)+(l-1);
	for (i = 0; i < l/2; i++) {
		t = *(n->c+i);
		*((n->c)+i) = *(ne-i);
		*(ne-i) = t;
	}
}
