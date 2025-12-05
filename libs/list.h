#ifndef LIST_H
#define LIST_H

#include<stdlib.h>

typedef int (*Destructor)(void *data);
typedef int (*Comparator)(void *current, void *comparing);

typedef struct queue
{
    struct list *first;
    struct list *last;
}QUEUE;

typedef struct list
{
    void *data;             
    struct list *next;
    float weight;   
}LIST;

LIST *createElement(void *data, float weight);
QUEUE *createWrap();
int insertList(LIST **l, void *data, float weight);
int append(QUEUE **r, void *data, float weight);
int orderedInsert(LIST **l, void *data, float weight);
LIST *pop(LIST **l);
void *popData(LIST **l);
LIST *dequeue(QUEUE *q);
void *dequeueData(QUEUE *q);
int freeList(LIST **l, Destructor destroy);
void *extract(LIST **l, void *search, Comparator compare);

#endif