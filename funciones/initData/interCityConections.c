

/*

    Veo prudente explicar aqui que utilizamos hashNode + addEdgeThrough 
    porque los nodos pertenecen a distintos grafos y addEdge utiliza el hash
    del grafo para conectarlos, por eso las diferencias:

    addEdge(Grafo, LlaveNodo1, LlaveNodo2, Peso, Tipo_De_Conexion) 

    addEdgeThrough(Nodo1, Nodo2, Peso, Tipo_De_Conexion)

    Una forma eficiente de generar un grafo multicapa

    Y como cada nodo  tiene un puntero a su grafo 
    esto permite hacer exploraciones por todo el grafo o por
    el grafo local
*/

int generateInterCityConnections()
{
    printf("Generando conexiones internacionales...\n");

    // Doble bucle para conectar Ciudad A con Ciudad B esto es O(n^2) 
    for(int i = 0; i < nCities; i++)
    {

        NODE *nodeA = hashNode(map, cityNames[i]);
        if(!nodeA) 
            continue;
        CITY *cityA = (CITY*)nodeA->data;           // Obtenemos la ciudad A
        MD *metaA = (MD*)cityA->people->metadata;

        for(int j = 0; j < nCities; j++)
        {
            if(i == j) 
                continue; // No conectar ciudad consigo misma (pues las conexiones locales ya se generaron anteriormente)

            NODE *nodeB = hashNode(map, cityNames[j]);  // Obtenemos la ciudad B
            if(!nodeB) 
                continue;
            CITY *cityB = (CITY*)nodeB->data;
            MD *metaB = (MD*)cityB->people->metadata;

            // Garantizamos la primera conexion entre ciudades
            // lo que se traduce a que todo el grafo siempre
            // SIEMPRE, esta conectado

            int rA = rand() % metaA->nPopulation;
            int rB = rand() % metaB->nPopulation;
            
            NODE *nA = hashNode(cityA->people, metaA->population[rA]->personKey);
            NODE *nB = hashNode(cityB->people, metaB->population[rB]->personKey);
            
            if(nA && nB)
            {
                 float w = (float)((rand() % 3) + 1) / 10.0f;
                 addEdgeThrough(nA, nB, w, 2);
            }

            // Dejamos fijo el numero de conexiones a un 2%
            int nConnections = (int)ceil(0.002 * cityB->population);
            if(nConnections < 1) nConnections = 1;

            // Para cada habitante de ciudad A
            for(int k = 0; k < metaA->nPopulation; k++)
            {
                // Un 30% de tener conexiones en esa ciudad
                if((rand() % 100) < 30) 
                {
                    PERSON *pA = metaA->population[k];

                    // Buscamos su NODO real en el grafo A (para poder crear la arista)
                    NODE *n1 = hashNode(cityA->people, pA->personKey); // obtenemos el nodo

                    // Generamos las n conexiones calculadas 
                    for(int c = 0; c < nConnections; c++)
                    {
                        // Elegimos un random en Ciudad B
                        int rIdx = rand() % metaB->nPopulation;
                        PERSON *pB = metaB->population[rIdx];
                        
                        // Buscamos su NODO real en el grafo B
                        NODE *n2 = hashNode(cityB->people, pB->personKey);

                        if(n1 && n2) // Seguridad de haber hasheado correctamente los 2 nodos
                        {
                            // PESO DÉBIL (0.1 a 0.3)
                            // Simula contacto esporádico o digital 
                            float weight = (float)((rand() % 3) + 1) / 10.0f;

                            // Usamos addEdgeThrough como explique anteriormente pues vienen de grafos distintos
                            // De hecho esta es la conexion normal de un grafo, regularmente no se conectan por hash X D
                            addEdgeThrough(n1, n2, weight, 2);
                        }
                    }
                }
            }
        }
    }
    printf("Mundo interconectado.\n");
}