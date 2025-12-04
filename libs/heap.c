#include"heap.h"

HEAP *initHeap(int capacity, Comparator compare)
{
    HEAP *newH = (HEAP*)malloc(sizeof(HEAP));
    
    if(!newH)
        return NULL;

    newH->elements = (void**)malloc(sizeof(void*)*capacity);
    newH->size = 0;
    newH->capacity = capacity;
    newH->compare = compare;

    return newH;
}

int resizeHeap(HEAP *h)
{
    void **temp = h->elements;
    int newCap = (int)(h->capacity*1.5);

    h->elements = (void**)realloc(h->elements,sizeof(void*)*newCap);
    
    if(!h->elements)
    {
        h->elements = temp;
        return -1;
    }

    h->capacity = newCap;

    return 0;
}

int hPush(HEAP *h, void *data)
{
    if(!h || !data)
        return -1;

    if(h->size>=h->capacity)
        resizeHeap(h);

    int i = h->size;        // Posicion del nuevo elemento / Inicio del flotado
    h->elements[i] = data;  
    h->size++;

    for(;i>0;i=(i-1)/2)
    {
        int parent = (i-1)/2;
        if(h->compare(h->elements[i], h->elements[parent]))    // Callback que puede cambiar el orden del heap (Min/Max)
            swap(&h->elements[parent],&h->elements[i]);          // Al flotar la pregunta es: el hijo es mas/menos que el padre?
                                                               // si lo es flota, tomando PRIORIDAD si no se queda 
    }

    return 0;
}

void *hPop(HEAP *h)
{
    if(!h || h->size == 0)
        return NULL;

    int i = 0;                      // el inicio para el hundimiento
    void *temp = h->elements[i];    // Guardamos el dato

    h->elements[i] = h->elements[h->size-1];
    h->size--;

    int hSelect,l,r;
    
    while(i < h->size/2)
    {
        l = 2*i+1;
        r = 2*i+2;

        if(r<h->size)
            hSelect = h->compare(h->elements[l],h->elements[r]) ? l : r;    // Obtenemos el hijo con prioridad 
                                                                            // izquierda "pregunta" derecha ? "si" -> prioridad = izquierda : "si no" -> prioridad = derecha
        else
            hSelect = l;

        if(h->compare(h->elements[hSelect],h->elements[i])) // Callback que puede cambiar el orden del heap (Max/Min)
        {                                                   // Al hundir la pregunta es: es el hijo mas/menos que el padre?
            swap(&h->elements[hSelect],&h->elements[i]);    // Si lo es, se hunde el padre dandole PRIORIDAD al hijo
            i = hSelect;
            continue;
        }
        break;
    }

    return temp;
}

int swap(void **a, void **b)
{
    void *temp = *a;

    if(!a && !b)
        return -1;
    
    *a = *b;
    *b = temp;

    return 0;
}