#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "sort.h"

void *quicksort_partition( void *p, void *r, void *(*prevfunc) (void *node), void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b)) {
	void *x = p; // Pivot is the first piece of data.

	void *i = NULL;
	void *j = NULL;
	unsigned char done = 0;

	while (1) {
		do {
			if (j==NULL) j = r;
			else j = prevfunc(j);
			if (i==j) done=1;
		} while (comparefunc(j,x) > 0);
		do {
			if (i==NULL) i = p;
			else i = nextfunc(i);
			if (i==j) done=1;
		} while (comparefunc(i,x) < 0);
		if (done) return j;
		swapfunc (i, j);
	}
}

void quicksort_internal( void *p, void *r, void *(*prevfunc) (void *node), void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b)) {
	// fprintf (stderr, "quicksort_internal...\n");
	if (p != r) {
		void *q = quicksort_partition(p, r, prevfunc, nextfunc, swapfunc, comparefunc);
		if (q==NULL) return;
		if (p != q) quicksort_internal (p, q, prevfunc, nextfunc, swapfunc, comparefunc);
		if (q != r) {
			void *qnext = nextfunc(q);
			if (qnext != r) quicksort_internal (qnext, r, prevfunc, nextfunc, swapfunc, comparefunc);
		}
	}
}

void quicksort( void *data, void *(*prevfunc) (void *node), void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b)) {
	void *tail = data;
	void *next = NULL;
	if (tail == NULL) return;
	while ((next = nextfunc(tail)) != NULL) tail = next;
	quicksort_internal (data, tail, prevfunc, nextfunc, swapfunc, comparefunc);
}

