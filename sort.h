#ifndef SORT_H
#define SORT_H

void quicksort( void *data, void *(*prevfunc) (void *node), void *(*nextfunc) (void *node), void (*swapfunc) (void *a, void *b), int (*comparefunc) (void *a, void *b));

#endif
