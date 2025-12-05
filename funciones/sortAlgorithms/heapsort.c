PERSON **heapSort(PERSON **population, int n)
{
    // Heapsort es sencillo

    // 1.- Creo mi heap
    HEAP *h = initHeap(n, compare);
    if (!h) return NULL;

    // 2.- Inserto los datos en el heap segun mi criterio
    for (int i = 0; i < n; i++)
    {
        if (population[i] != NULL)
            hPush(h, population[i]);
    }

    // 3.- Creo un array exactamente igual
    PERSON **sortedArr = (PERSON**)malloc(sizeof(PERSON*) * n);
    if (!sortedArr) return NULL;

    // 4.- Hago pop sobre el nuevo array en orden numerico
    //     el heap los acomoda automaticamente
    for (int i = 0; i < n; i++)
    {
        sortedArr[i] = (PERSON*)hPop(h);
    }

    free(h->elements);
    free(h);


    /* HEAP SORT!!!!
    (\(\
    ( -.-)
    o_(")(") 
    */

    return sortedArr;
}


int compare(void *person1, void *person2) // Callback necesario en el heap, al ser generico las comparaciones deben ser definidas por el usuario
{
    return criteria((PERSON*)person1, (PERSON*)person2, eCrit);
}

/*
    La funcion criteria realiza las evaluaciones de persona 1 y persona 2 respetando
    la jerarquia de la libreria: Izquierda Comparacion Derecha

    Y gracias a esto permite en un solo heapSort generar 4 acomodos distintos
    Lo que se traduce a 4 heaps con criterios de ordenamiento distinto

    Max/Min
*/

