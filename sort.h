#ifndef SORT_H
#define SORT_H

void sort( void *data, void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b));

#endif
