HealthState stringToState(char *str)
{
    if(strcmp(str, "SUSCEPTIBLE") == 0) return SUSCEPTIBLE;
    if(strcmp(str, "INFECTADO") == 0) return INFECTADO;
    if(strcmp(str, "VACUNADO") == 0) return VACUNADO;
    if(strcmp(str, "RECUPERADO") == 0) return RECUPERADO;
    return FALLECIDO;
}

int loadData()
{
    // cargamos las 50 cepas
    loadViruses();

    map = createGraph("Map", NULL); 

    FILE *fp = NULL;

    char filepath[100];
    char line[256];

    for(int i = 0; i<nCities; i++)
    {
     
        sprintf(filepath, "%s%s%s", path, cityNames[i], ext);
        fp = fopen(filepath, "r");
        if(!fp)
        {
            printf("Error abriendo: %s\n", filepath);
            continue;
        }

        if(!fgets(line, sizeof(line), fp)) 
        {
            fclose(fp);
            continue; 
        }

        // Cada archivo de ciudad guarda el numero de habitantes primero
        int cityPop = atoi(line);

        // Creamos la ciudad con sus metadatos y su grafo de personas
        CITY *c = createCity(i,&cityNames[i],cityPop);
l
        // Añadimos al grafo global la nueva ciudad
        addNode(map, c->name, c);  // Esta funcion incluye hasheo

        int pIndex = 0;
        
        while(fgets(line, sizeof(line), fp))
        {
            // Eliminar salto de linea si existe
            line[strcspn(line, "\r\n")] = 0;

            // Formato: ID,Nombre,Apellido,Estado,VirusID
            
            // 1. ID
            char *token = strtok(line, ",");
            if(!token) continue;
            int id = atoi(token);

            // 2. Nombre
            token = strtok(NULL, ",");
            char firstName[50];
            strcpy(firstName, token);

            // 3. Apellido, concatenado porque el nombre se guarda de manera simple
            token = strtok(NULL, ",");
            char fullName[100];
            sprintf(fullName, "%s_%s", firstName, token);

            // 4. Estado
            token = strtok(NULL, ",");
            HealthState state = stringToState(token);

            // 5. Cepa con la que inicia si corresponde (-1 si no esta infectado)
            token = strtok(NULL, ",");
            int vID = atoi(token);

            // Llave de la persona para el grafo
            char pKey[20];
            sprintf(pKey, "%.3s%d", firstName, id);

            PERSON *p = createPerson(id, fullName, c->name, pKey, state, vID);

            MD *meta = (MD*)c->people->metadata;

            // Nuestros datos se guardaran en 2 niveles
            // En el grafo para operaciones complejas de relacion
            // y en un array para sorting
            meta->population[pIndex] = p;
            addNode(c->people, p->personKey, p);
            
            // Conectamos a cada persona con la anterior para asegurar que el grafo no son nodos sueltos
            // solo para el primer 10% de la poblacion
            // esto nos asegura que el grafo este conectado al 100%
            // el decimo individuo de hecho se conecta ademas aleatoriamente con otros nodos
            if(pIndex > 0 && pIndex <= (int)(0.1*cityPop))
            {
                PERSON *prev = meta->population[pIndex - 1];
                addEdge(c->people, p->personKey, prev->personKey, 1.0f, 2);
            }
            
            // Si ya tenemos el 10% de los elementos de la poblacion:
            if(pIndex >= (int)(0.1 * cityPop))
            {
                // Del 1 al 100 para simular porcentajes de poblacion
                int roll = (rand() % 100) + 1; 
                float connectionPercent; 

                // Asignacion basada en la aproximacion de una distribucion normal
                // Media: 5% | Extremos: 2% y 8%
                
                if (roll <= 5)        // 5% de la poblacion 
                    connectionPercent = 0.02f; 
                else if (roll <= 20)  // 15% de la poblacion
                    connectionPercent = 0.03f; 
                else if (roll <= 40)  // 20% de la poblacion 
                    connectionPercent = 0.04f; 
                else if (roll <= 60)  // 20% de la poblacion 
                    connectionPercent = 0.05f; 
                else if (roll <= 80)  // 20% de la poblacion
                    connectionPercent = 0.06f; 
                else if (roll <= 95)  // 15% de la poblacion
                    connectionPercent = 0.07f; 
                else                  // 5% de la poblacion
                    connectionPercent = 0.08f; 

                int extraConnections = (int)(connectionPercent * cityPop); 
                
                if (extraConnections == 0 && cityPop > 10) 
                    extraConnections = 1;
                
                if (extraConnections > pIndex) 
                    extraConnections = pIndex - 1;

                for(int k = 0; k < extraConnections; k++)
                {
                    int rIdx = rand() % pIndex; 
                    
                    if (rIdx == pIndex) // No nos conectamos a nosotros mismos 
                    { 
                        k--;            // Aseguramos las n conexiones calculadas
                        continue; 
                    } 

                    PERSON *randomNeighbor = meta->population[rIdx];

                    // PESO DINÁMICO
                    // En este modelo las personas mas sociables
                    // son mas propensas a tener lazos mas debiles
                    // esto provoca que aunque tienen mas conexiones (foco de infeccion)
                    // tambien son menos propensos a contagiar
                    float weight;
                    if(connectionPercent > 0.06f)
                        weight = (float)((rand() % 5) + 1) / 10.0f; // 0.1 a 0.5
                    else
                        weight = (float)((rand() % 6) + 5) / 10.0f; // 0.5 a 1.0

                    addEdge(c->people, p->personKey, randomNeighbor->personKey, weight, 2);
                }
            }

            pIndex++;
        }

        fclose(fp);
        // printf("Ciudad cargada: %s (%d habitantes)\n", c->name, pIndex);
    }

    generateInterCityConnections() // Generar conexiones con personas en otras ciudades 
                                   // Para dejar que una enfermedad se propague

    return 0;
}