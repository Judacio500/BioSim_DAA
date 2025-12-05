int applySmartQuarantine(CITY *c, int budget)
{
    if(!c || budget <= 0) 
        return -1;

    MD *meta = (MD*)c->people->metadata;
    int maxCapacity = (int)(meta->nPopulation * 0.3); // Capacidad total del array (30%)

    // Si ya esta lleno el bunker, no cabe nadie mas
    if(meta->quarantined >= maxCapacity)
        return 0;

    // Inicializamos el heap temporal
    HEAP *riskHeap = initHeap(meta->nPopulation, compareRisk);
    if(!riskHeap) 
        return -1;

    // Llenado del Heap
    // Recorremos el array de toda la poblacion linealmente
    for(int i = 0; i < meta->nPopulation; i++)
    {
        PERSON *p = meta->population[i];

        // Solo consideramos candidatos validos:
        // - Vivos
        // - Que no esten en cuarentena
        // - Con riesgo > 0 (si alguien no tiene riesgo no sirve de nada aislarlo)
        if(p->state != FALLECIDO && !p->quarantine && p->globalRisk > 0)
        {
            hPush(riskHeap, p); // Empujamos el nuevo propenso 
        }
    }

    printf("Analisis completado. Candidatos a cuarentena: %d\n", riskHeap->size);

    // Metemos en cuarentena a cuantos hayan sido designados
    int isolatedCount = 0;
    
    // Mientras tengamos presupuesto, candidatos y ESPACIO en el array
    while(isolatedCount < budget && riskHeap->size > 0 && meta->quarantined < maxCapacity)
    {
        PERSON *target = (PERSON*)hPop(riskHeap);
        
        // Aplicar aislamiento
        target->quarantine = true; // Un individuo solo puede salir de cuarentena si...
                                   // se recupero la persona (esto se gestiona en otra función)

        // Lo movemos al array de cuarentena para gestion rapida
        // Usamos el contador actual como indice y LUEGO incrementamos
        meta->quarantineArray[meta->quarantined] = target;
        meta->quarantined++; 

        // Mostramos el dato de registro, esto no se guarda
        printf("\t[!] CUARENTENA: %s (Riesgo: %.2f) -> Aislado. (Total: %d/%d)\n", target->name, target->globalRisk, meta->quarantined, maxCapacity);

        isolatedCount++;
    }

    // Liberamos el heap temporal
    // Las personas siguen en el grafo/array original
    free(riskHeap->elements);
    free(riskHeap);
    
    return isolatedCount; // Retornamos cuantos logramos aislar
}

// Comparador para el Heap de Cuarentena (MaxHeap por GlobalRisk)
int compareRisk(void *a, void *b)
{
    PERSON *p1 = (PERSON*)a;
    PERSON *p2 = (PERSON*)b;
    
    // Queremos que flote el de MAYOR riesgo
    return (p1->globalRisk > p2->globalRisk);
}