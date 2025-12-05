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
    int TotalPopulation = 0;

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
            if(pIndex > 0)
            {
                PERSON *prev = meta->population[pIndex - 1];
                // Peso 1.0, Grafo Bidireccional
                addEdge(c->people, p->personKey, prev->personKey, 1.0f, 2);
            }
            
            // Si ya tenemos el 10% de los elementos de la poblacion:
            if(pIndex >= (int)(0.1*cityPop))
            {
                // Agregamos el 4% de conexiones extra con personas aleatorias que ya existen en el grafo
                int extraConnections = (int)(0.04*cityPop); 

                for(int k = 0; k < extraConnections; k++)
                {

                    int rIdx = rand() % (pIndex - 1); 

                    if(rIdx == pIndex)
                    {
                        k--;
                        continue;
                    }

                    PERSON *randomNeighbor = meta->population[rIdx];

                    // Generamos un peso aleatorio entre 0.1 y 1.0 para simular
                    // cercania que sera importante para el calculo de probabilidades de contagio
                    // Es mas probable contagiarte con alguien que ves a diario que 
                    // con alguien que saludas en la calle cada 7 dias
                    float weight = (float)((rand() % 10) + 1) / 10.0f; 

                    // Conexion bidireccional de nuevo ergo ambos se conocen
                    addEdge(c->people, p->personKey, randomNeighbor->personKey, weight, 2);
                }
            }

            pIndex++;
        }

        fclose(fp);
        // printf("Ciudad cargada: %s (%d habitantes)\n", c->name, pIndex);
    }

    return 0;
}