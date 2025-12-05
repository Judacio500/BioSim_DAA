// Estructura auxiliar para pasar multiples parametros al callback
typedef struct SimulationContext
{
    int currentDay;
    LIST *newInfected;   // Lista temporal para guardar a los que se infectan hoy
    LIST *newRecovered;  // Lista temporal para los que se recuperan hoy
} SC;

// Callback utilizado en traverseGraph
// para hacer todos los calculos de la población 
void simulationCallback(void *data, void *param)
{
    NODE *currentNode = (NODE*)data;
    if(!currentNode) 
        return;

    PERSON *p = (PERSON*)currentNode->data;
    SC *ctx = (SC*)param;

    float infWeight = 0; // Peso combinado de adyacencias contagiadas
    float myConnectivity = 0; // Riesgo global calculado puramente de cantidad de adyacencias

    EDGE *edge = currentNode->firstEdge;
    
    NODE *virusSources[100]; // Array simple para candidatos a contagiarme
    float sourceProbabilities[100];
    int candidatesCount = 0;

    while(edge)
    {
        NODE *neighborNode = edge->destinationNode;
        PERSON *neighbor = (PERSON*)neighborNode->data;

        // Sumamos el peso de cada adyacencia para determinar la conectividad de una persona
        // Que tan propenso es a contagiar
        // Aqui se balancean un poco las cosas con las conexiones
        // Aqui contagia mas quien tiene ConexionFuerte + MuchasConexiones
        myConnectivity += edge->weight;

        // Revisamos si el vecino esta infectado para proceder
        if(neighbor->state == INFECTADO && neighbor->v != NULL)
        {
            // Probabilidad de que ESTE vecino me contagie
            float weight = neighbor->v->contagiousness * edge->weight;
            
            // Factor de reducción la persona ya se habia contagiado
            // genero anticuerpos
            if(p->state == RECUPERADO) 
                weight *= 0.1f; // (Corregido a 0.1f para dar resistencia, no inmunidad total)
            else if(p->state == VACUNADO) 
                weight = 0.0f;

            if(weight > 0 && candidatesCount < 100)
            {
                virusSources[candidatesCount] = neighborNode;
                sourceProbabilities[candidatesCount] = weight;
                candidatesCount++;
                infWeight += weight;
            }
        }
        edge = edge->nextNode;
    }

    // Actualizamos metricas
    p->infRisk = infWeight; 
    if(p->state == INFECTADO && p->v)
        p->globalRisk = myConnectivity * p->v->contagiousness;
    else
        p->globalRisk = myConnectivity * 0.5f; // Potencial (con virus promedio)


    if(p->state == INFECTADO)
    {
        p->daysInfected++; // Aumentamos los dias infectado de esta persona

        if(p->v)
        {
            // La letalidad es la probabilidad de morir CADA DÍA que estás enfermo
            // distribuida linealmente en el periodo de enfermedad

            // Para suavizar, usamos letalidad / recoveryTime que distribuye la probabilidad de morir en los dias de enfermedad
            float dailyLethality = p->v->lethality / p->v->recovery; 

            float deathRoll = (float)(rand() % 1000) / 1000.0f;
            if(deathRoll < dailyLethality)
            {
                p->state = FALLECIDO;
                p->v = NULL; // El virus muere con el huesped
                return; // Terminamos este nodo porque ya no puede hacer nada
            }

            float mutationRoll = (float)(rand() % 1000) / 1000.0f;
            if(mutationRoll < p->v->mProbablity)
            {
                // ¡MUTA!
                // Elegimos un nuevo virus aleatorio del catálogo global
                int newVirusID = rand() % 50; // 50 cepas
                p->v = &viruses[newVirusID];
            }
        }

        if(p->v && p->daysInfected >= p->v->recovery) // Revisamos si de acuerdo a su virus ya se mejoro
        {
            // Lo agendamos para recuperacion al final del dia para que sus adyacencias no lo consideren como recuperado en el estado de hoy
            insertList(&ctx->newRecovered, p, 0);
        }
        return; // Un infectado no se vuelve a infectar (ese dia)
    }

    if((p->state == SUSCEPTIBLE || p->state == RECUPERADO) && candidatesCount > 0)
    {
        // ¿Me contagio?
        // Usamos una probabilidad compuesta simple basada en el peso total de infección
        float chance = infWeight; 
        if(chance > 1.0f) chance = 1.0f;

        // Tirada
        float roll = (float)(rand() % 1000) / 1000.0f;

        if(roll < chance)
        {
            // ¡CONTAGIADO!
            
            // ¿Quién fue? 
            // Elegimos cual de los vecinos fue el culpable basado en su aporte al peso de infección
            float roulette = (float)(rand() % 1000) / 1000.0f * infWeight;
            float cumsum = 0.0f;
            PERSON *sourcePatient = NULL;

            for(int i=0; i < candidatesCount; i++)
            {
                cumsum += sourceProbabilities[i];
                if(roulette <= cumsum)
                {
                    sourcePatient = (PERSON*)virusSources[i]->data;
                    break;
                }
            }
            
            if(!sourcePatient) 
                sourcePatient = (PERSON*)virusSources[0]->data;

            // Preparamos el Log de Contagio
            CONTAGION *log = createContagion(ctx->currentDay, sourcePatient->id, sourcePatient->name, p->id, p->name, sourcePatient->v->name);
            
            p->v = sourcePatient->v;    // Hereda cepa no hay problema con los calculos porque
                                        // aun no esta marcado como infectado
            
            // Todos los nodos apuntan a su grafo que guarda su metaData
            if(currentNode->graph && currentNode->graph->metadata)
            {
                MD *cityMeta = (MD*)currentNode->graph->metadata;
                append(cityMeta->contagionHistory, log, 0); // Encolamos el log del contagio a los contagios por ciudad
            }

            // Agendamos el cambio de estado al finalizar
            insertList(&ctx->newInfected, p, 0);
        }
    }
}

int stepSimulation(int day)
{
    printf("--- Simulando Dia %d ---\n", day);

    SC ctx;
    ctx.currentDay = day;
    ctx.newInfected = NULL;
    ctx.newRecovered = NULL;

    // Solo necesitamos un punto de entrada válido al Mapa de personas.
    // Usamos el primer nodo de la primera ciudad disponible.
    // Gracias a las conexiones inter-ciudades, traverseGraph saltará de país en país.
    // Recordemos que garantizamos la primera conexion asi que el grafo es
    // valgame la redundancia, conexo
    
    NODE *entryPoint = NULL;
    
    // Buscamos una entrada válida (por seguridad, si la ciudad 0 estuviera vacía)
    for(int i=0; i < nCities; i++)
    {
        NODE *cityNode = hashNode(map, cityNames[i]);
        if(cityNode)
        {
            CITY *c = (CITY*)cityNode->data;
            if(c->people && c->people->currentNode)
            {
                entryPoint = c->people->currentNode;
                break; // ¡Encontramos la entrada a la Red!
            }
        }
    }

    if(entryPoint)
    {
        LIST *visited = NULL; // Control de ciclos global
        traverseGraphWParameter(&visited, entryPoint, &ctx, simulationCallback);
        freeList(&visited, NULL); 
    }
    else
    {
        printf("El mundo esta vacio.\n"); // Debugging por si los archivos no cargaran
                                          // esta funcion seria lo primero en saltar a la vista 
    }
    
    // Despues de los calculos finaliza el dia y cambiamos el estado de infección
    while(ctx.newInfected)
    {
        PERSON *p = (PERSON*)popData(&ctx.newInfected);
        p->state = INFECTADO;
        p->daysInfected = 0;
        
        // Agregar a la lista de infectados de su ciudad
        NODE *cNode = hashNode(map, p->cityKey);
        if(cNode) {
            CITY *c = (CITY*)cNode->data;
            MD *m = (MD*)c->people->metadata;
            insertList(&m->infectedList, p, 0);
        }
    }

    // Luego anexamos a los recien recuperados
    while(ctx.newRecovered)
    {
        PERSON *p = (PERSON*)popData(&ctx.newRecovered);
        p->state = RECUPERADO;
        p->v = NULL; 
        p->daysInfected = 0;
        
        // Para removerlo de lista de infectados
        NODE *cNode = hashNode(map, p->cityKey);
        CITY *c = (CITY*)cNode->data;
        MD *m = (MD*)c->people->metadata;
        extract(&m->infectedList,p,comparePerson);
    }

    // Gestión de Cuarentena (Liberar y Aislar)
    for(int i=0; i < nCities; i++)
    {
        NODE *node = hashNode(map, cityNames[i]);
        if(node)
        {
            CITY *c = (CITY*)node->data;
            releaseRecovered(c);          // Liberamos a los que ya sanaron
            applySmartQuarantine(c, 5);   // Aislamos a los más peligrosos (Presupuesto diario fijo: 5)
        }
    }

    return 0;
}

int comparePerson(void *person1, void *person2)
{
    PERSON *p1 = (PERSON*)person1;
    PERSON *p2 = (PERSON*)person2;

    return p1->id == p2->id ? 1 : 0;
}