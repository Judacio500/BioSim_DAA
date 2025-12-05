#include <math.h>
#include <float.h>


//MinHeap para dijkstra
int compareDijkstra(void *a, void *b)
{
    D_STATE *s1 = (D_STATE*)a;
    D_STATE *s2 = (D_STATE*)b;

    return (s1->cost < s2->cost);
}


LIST *findCriticalPath(NODE *start, NODE *target)
{
    if(!start || !target) 
        return NULL;

    printf("[DIJKSTRA] Buscando camino critico de %s a %s...\n",((PERSON*)start->data)->name, ((PERSON*)target->data)->name);

    // Hash para arreglo de distancia y camino
    // el acceso con clave lo hace mas atractivo
    // en terminos de practicidad
    HASH *distMap = initHash(0);
    HASH *parentMap = initHash(0);
    
    // Heap de Prioridad 
    // El tamaño inicial resulta un poco irrelevante ya que se redimensiona solo al llenarse
    HEAP *priorityQueue = initHeap(100, compareDijkstra);

    // Aqui se nota como brilla el hash NOMBRE:VALOR
    // no necesitamos trackear el id de nadie, a nivel logico
    // es mas obvio decirle a la computadora "Fulanito"
    // y sin perder ese valioso tiempo O(1)
    float *startDist = (float*)malloc(sizeof(float));
    *startDist = 0.0f;
    saveKey(&distMap, start->nodeKey, startDist);

    // Push inicial al heap
    D_STATE *initialState = createDstate(start,0);
    
    hPush(priorityQueue, initialState);

    NODE *current = NULL;
    int found = 0;

    while(priorityQueue->size > 0) // Siempre que haya nodos a explorar
    {
        D_STATE *state = (D_STATE*)hPop(priorityQueue);
        current = state->currentNode;
        float currentCost = state->cost;
        free(state); // Liberamos el wrapper, no el nodo

        // Si llegamos al destino, terminamos
        if(current == target) // Comparacion de punteros directa
        {
            found = 1;
            break;
        }

        // Revisar si encontramos un camino mas corto a 'current'
        EHASH *dElem = hashing(distMap, current->nodeKey);
        if(dElem)
        {
            float bestKnown = *(float*)dElem->pair;
            if(currentCost > bestKnown) 
                continue;
        }

        // Exploramos vecinos
        EDGE *edge = current->firstEdge;
        while(edge)
        {
            NODE *neighbor = edge->destinationNode;
            
            // Calculo del Costo de la Arista
            // Peso original (probabilidad): 0.1 a 1.0
            // Costo Dijkstra: -log(peso)
            // Evitamos log(0)

            float prob = edge->weight;
            if(prob <= 0.0001f) prob = 0.0001f; 
            
            float edgeCost = -logf(prob); 
            float newDist = currentCost + edgeCost;

            // Revisar distancia actual conocida
            float oldDist = FLT_MAX; // Distancia mas grande que cualquier otra
                                     // basicamente el centinela para intercambiar la distancia la primera vez
            EHASH *nElem = hashing(distMap, neighbor->nodeKey);
            if(nElem) 
                oldDist = *(float*)nElem->pair;

            // Comprobamos si la distancia es correcta
            if(newDist < oldDist)
            {
                // Actualizamos distancia
                float *newDistPtr = (float*)malloc(sizeof(float));
                *newDistPtr = newDist;
                // saveKey simula actualizacion si ya existe
                // como el metodo de colisiones es chaining
                // el primer elemento del hash con la misma llave es el ultimo insertado
                saveKey(&distMap, neighbor->nodeKey, newDistPtr);

                // Actualizamos padre
                saveKey(&parentMap, neighbor->nodeKey, current);

                // Push al heap
                D_STATE *nextState = createDstate(neighbor,newDist);
                hPush(priorityQueue, nextState);
            }

            edge = edge->nextNode;
        }
    }

    // Reconstruimos el camino
    LIST *pathList = NULL;
    
    if(found)
    {
        NODE *path = target;
        while(path)
        {
            // Insertamos al inicio de la lista para tener orden Origen -> Destino
            insertList(&pathList, path, 0);
            
            if(path == start) 
                break;

            // Buscar al padre de 'path'
            // en nuestro mapa hash, decimos "Quien fue el que tuvo la menor distancia a este nodo?"
            // importante aclarar aqui que el nodo inicial no genero un padre para su llave, por lo que la respuesta sera NULL
            EHASH *pElem = hashing(parentMap, path->nodeKey);
            if(pElem)
                path = (NODE*)pElem->pair; // Cuando lo encontramos actualizamos path y volvemos a intentar
            else
                path = NULL; // Camino roto (no deberia pasar si found=1)
        }
    }

    free(priorityQueue->elements); 
    free(priorityQueue);
    
   // Liberamos el mapa de distancias (Borramos la tabla Y los floats)
    freeHash(&distMap, destroyFloat); 
    
    // Liberamos el mapa de padres (Borramos la tabla pero NO los nodos, porque son del grafo)
    freeHash(&parentMap, NULL);

    return pathList;
}

void destroyFloat(void *data)
{
    if(data) 
        free(data); // ¡Totalmente válido y seguro!
}