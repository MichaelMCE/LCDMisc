#include <malloc.h>
#include "malloc.h"

#ifndef X64
int _srealloc (void **v, size_t bsize)
{
	void *temp = realloc(*v, bsize);
	if (!temp && bsize) {
		//*v = 0;
		return 0;
	}
	*v = temp;
	return 1;
}
#endif

//void malloc(int len) {
//}

