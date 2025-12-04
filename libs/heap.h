#ifndef HEAP_H
#define HEAP_H

#include<stdio.h>
#include<stdlib.h>

typedef int (*Comparator)(void *left, void *right); // como cualquier operador de comparación:  left < right
                                                    //                                          left > right

typedef struct heap
{
    void **elements;
    int size;
    int capacity;
    Comparator compare;
}HEAP;

HEAP *initHeap(int capacity, Comparator compare);
int resizeHeap(HEAP **h);
int hPush(HEAP *h, void *data);
void *hPop(HEAP *h);
int swap(void **a, void **b);

#endif