#include"list.h"

LIST *createElement(void *data, float weight)
{
    LIST *newE = (LIST*)malloc(sizeof(LIST));

    if(!newE)
        return NULL;

    newE->data = data;
    newE->next = NULL;
    newE->weight = weight; // Aqui tenia  un 0

    return newE;
}

QUEUE *createWrap()
{
    QUEUE *newQ = (QUEUE*)malloc(sizeof(QUEUE));

    if(!newQ)
        return NULL;

    newQ->first = NULL;
    newQ->last = NULL;

    return newQ;
}

int insertList(LIST **l, void *data, float weight)
{
    LIST *newE = createElement(data,weight);

    if(!newE)
        return -1;

    newE->next = *l;
    *l = newE;
    
    return 0;
}

int append(QUEUE **q, void *data, float weight)
{
    if(!*q)
    {
        *q = createWrap();
        if(!*q)
            return -1;
    }

    LIST *newE = createElement(data,weight);

    if(!newE)
        return NULL;

    if(!(*q)->first && !(*q)->last)
    {
        (*q)->first = newE;
        (*q)->last  = newE;
        return 0;
    } 

    (*q)->last->next = newE;
    (*q)->last = newE;

    return 0;
}

int orderedInsert(LIST **l, void *data, float weight)
{
    if(!l)
        return -1;
    
    while(*l && (*l)->weight < weight)
        l = &(*l)->next;

    insertList(l,data,weight);
    return 0;
}

LIST *pop(LIST **l)
{
    if(!l || !*l)
        return NULL;

    LIST *pElem = *l;
    *l = pElem->next;
    pElem->next = NULL;

    return pElem;
}

void *popData(LIST **l)
{
    if(!l || !*l)
        return NULL;

    LIST *pElem = *l;
    void *data = pElem->data;

    *l = pElem->next;

    free(pElem);

    return data;
}

LIST *dequeue(QUEUE *q)
{
    if(!q)
        return NULL;

    LIST *pElem = pop(&(q->first));

    if(!pElem)
        return NULL;

    if(!q->first)
        q->last = NULL;

    return pElem;
}

void *dequeueData(QUEUE *q)
{
    if(!q)
        return NULL;

    void *data = popData(&(q->first));

    if(!data)
        return NULL;

    if(!q->first)
        q->last = NULL;

    return data;
}

int freeList(LIST **l, Destructor destroy)
{
    while(*l)
    {
        void *data = popData(l); // Eventualmente l->NULL

        if(destroy)              // Si no pasamos destructor los datos dentro de la lista no se destruyen
            destroy(data);       // solo la envoltura, esto es relevante porque hay datos que viven en otra
                                 // estructura y que pueden necesitarse en una lista solo temporalmente
    }

    return 0;
}

int freeQueue(QUEUE *q, Destructor destroy)
{
    if(!q)
        return -1;

    freeList(&q->first, destroy); //Liberamos su lista
    free(q);

    return 0;
}