#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sort.h"

void sort( void *data, void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b)) {
	unsigned char modified;
	void *head = data;

	if (data == NULL) return; // empty list
	do {
		void *a = head;
		void *b;
		modified = 0;
		while ((b = (nextfunc(a))) != NULL) {
			if (comparefunc(a,b) < 0) {
				swapfunc(a,b);
				modified = 1;
			}
			else a=b;
		}
	} while (modified);
}

