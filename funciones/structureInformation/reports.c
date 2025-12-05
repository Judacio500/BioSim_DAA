int reportPerson(char *cityName, char *personName)
{
    // Hashing de la ciudad en el mapa
    NODE *cityNode = hashNode(map, cityName);
    if(!cityNode)
    {
        printf("\n [ERROR] La ciudad '%s' no existe en el mapa.\n", cityName);
        return -1;
    }

    CITY *c = (CITY*)cityNode->data;
    
    // Hashing a la persona en la ciudad especificada
    // c->people es un grafo
    NODE *personNode = hashNode(c->people, personName);
    if(!personNode)
    {
        printf("\n [ERROR] '%s' no es residente de %s.\n", personName, c->name);
        return -1;
    }

    PERSON *p = (PERSON*)personNode->data;

    // Impresion Bonita
    printf("\n========================================\n");
    printf(" FICHA MEDICA INDIVIDUAL\n");
    printf("========================================\n");
    printf(" ID: \t\t%d\n", p->id);
    printf(" Nombre: \t%s\n", p->name);
    printf(" Residencia: \t%s\n", c->name);
    
    char stateStr[25];
    
    healthStateToString(p->state,stateStr);

    printf(" Estado: \t%s\n", stateStr);
    
    if(p->v)
        printf(" Portador de: \t%s (Letalidad: %.2f)\n", p->v->name, p->v->lethality);
    
    printf(" Riesgo Global: %.4f (Potencial de contagio)\n", p->globalRisk);
    printf(" Estatus: \t%s\n", p->quarantine ? "[ EN CUARENTENA ]" : "LIBRE");
    printf("========================================\n");
}

int reportCity(char *cityName)
{
    NODE *cityNode = hashNode(map, cityName);
    if(!cityNode)
    {
        printf("\n [ERROR] La ciudad '%s' no existe.\n", cityName);
        return -1;
    }

    CITY *c = (CITY*)cityNode->data;
    MD *meta = (MD*)c->people->metadata;

    printf("\n========================================\n");
    printf(" REPORTE DE ZONA: %s\n", c->name);
    printf("========================================\n");
    printf(" Poblacion Total: \t%d\n", meta->nPopulation);
    
    printf("Personas en Cuarentena: %d\n", meta->quarantined);
    
    // REPORTE O(1) DEL HISTORIAL (Queue)
    QUEUE *q = meta->contagionHistory;
    
    printf("\n --- HISTORIAL DE CONTAGIOS (O(1)) ---\n");
    if(q && q->first)
    {
        // Caso mas antiguo
        CONTAGION *firstCase = (CONTAGION*)q->first->data;
        printf(" [PRIMER REGISTRO]\n");
        printf("   Dia: %d | %s contagio a %s\n", firstCase->day, firstCase->sourceName, firstCase->targetName);
        printf("   Cepa: %s\n", firstCase->virusName);

        // Caso mas reciente
        if(q->last && q->last != q->first)
        {
            CONTAGION *lastCase = (CONTAGION*)q->last->data;
            printf("\n [ULTIMO REGISTRO]\n");
            printf("   Dia: %d | %s contagio a %s\n", 
                   lastCase->day, lastCase->sourceName, lastCase->targetName);
            printf("   Cepa: %s\n", lastCase->virusName);
        }
    }
    else
    {
        printf(" [!] No se han registrado contagios en esta zona.\n");
    }
    printf("========================================\n");
}