
// Comentarios adicionales:
// El BFS es precioso

int bfsCluster(GRAPH *g, PERSON *start, HASH *visited)
{
    int count = 0;
    QUEUE *toVisit = createWrap();
    
    // Iniciamos BFS
    append(&toVisit, start, 0);
    saveKey(&visited, start->personKey, start); // En lugar de una lista de visitamos utilizamos un hash
                                                // Porque es mas rapido

    // En pocas palabras esto lo que hace es decir
    // Contando desde x Infectado todos estos a su alrededor estan enfermos 
    while(toVisit->first)
    {
        PERSON *current = (PERSON*)dequeueData(toVisit);
        count++;    // Contamos la persona como parte del cluster

        // Sacamos el nodo y vemos sus adyacencias para anexarlas al cluster
        NODE *uNode = hashNode(g, current->personKey);
        if(uNode)
        {
            EDGE *e = uNode->firstEdge;
            while(e)
            {
                PERSON *target = (PERSON*)e->destinationNode->data;
                
                // CONDICIÓN DEL BROTE:
                // Solo nos propagamos si el vecino está INFECTADO
                // y si NO lo hemos contado ya en este barrido.
                if(target->state == INFECTADO)
                {
                    // Ya fue visitado???
                    if(!hashing(visited, target->personKey))
                    {
                        // Si no esta lo marcamos y lo pegamos a la cola
                        saveKey(&visited, target->personKey, target);  
                        append(&toVisit, target, 0);                 
                    }
                }
                e = e->nextNode;
            }
        }
    }
    
    // Liberamos el wrapper de la cola de nodos por visitar
    free(toVisit); 
    return count;
}

// Recibe el grafo de una ciudad y para cada infectado genera un cluster
// Importante aclarar que un brote se puede extender hasta otra ciudad 
// Si A y C son personas de Mexico y NewYork ambos infectados estos dos ya generan un cluster
int detectOutbreak(GRAPH *peopleGraph)
{
    MD *meta = (MD*)peopleGraph->metadata;
    if (!meta || !meta->infectedList) 
        return 0;

    int maxClusterSize = 0;
    int currentClusterSize = 0;

    // Hash en lugar de una lista para no procesar al mismo infectado dos veces
    // (Si A contagió a B, el cluster de A es el mismo que el de B)
    // Tarde o temprano se unen los clusters y se forma una masa
    HASH *visited = initHash(0); 

    // Iteramos sobre los infectados
    LIST *current = meta->infectedList;
    while(current)
    {
        PERSON *p = (PERSON*)current->data;

        // Si no hemos procesado a este infectado en un cluster anterior:
        if (hashing(visited, p->personKey) == NULL)
        {
            // Contamos su cluster
            currentClusterSize = bfsCluster(peopleGraph, p, visited);
            
            if (currentClusterSize > maxClusterSize)
            {
                maxClusterSize = currentClusterSize;
            }
        }
        current = current->next;
    }

    // Limpieza del hash temporal 
    // al pasar NULL como segundo parametro le indicamos al callback
    // que no queremos deshacernos de los datos solo de la estructura
    // que los mostraba
    freeHash(&visited, NULL); 

    return maxClusterSize; // Retorna el tamaño del brote más grande encontrado
}