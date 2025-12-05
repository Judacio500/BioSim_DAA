// Criterio para ordenar la cuarentena:
// Prioridad a los que ya se recuperaron (Estado RECUPERADO va primero)
// O por dias restantes para sanar.

int compareRecovery(void *a, void *b)
{
    PERSON *p1 = (PERSON*)a;
    PERSON *p2 = (PERSON*)b;

    // Si p1 es RECUPERADO y p2 no, p1 va primero
    if (p1->state == RECUPERADO && p2->state != RECUPERADO) 
        return 1;
    if (p1->state != RECUPERADO && p2->state == RECUPERADO) 
        return 0;

    // Si ambos estan igual, desempatamos por dias infectado (quien lleva mas tiempo)
    return (p1->daysInfected > p2->daysInfected);
}

int releaseRecovered(CITY *c)
{
    if(!c) 
        return -1;
    MD *meta = (MD*)c->people->metadata;
    if(meta->quarantined == 0) 
        return -1;

    // Borrado seguro al reves
    for(int i = meta->quarantined - 1; i >= 0; i--)
    {
        PERSON *p = meta->quarantineArray[i];

        // Checamos si ya se recuperó o falleció
        if(p->state == RECUPERADO || p->state == FALLECIDO)
        {
            p->quarantine = false; 

            // Eliminar del Array (Swap con el último, esto deja el espacio libre como si no hubiera pasado nada)
            meta->quarantineArray[i] = meta->quarantineArray[meta->quarantined - 1];
            meta->quarantineArray[meta->quarantined - 1] = NULL;
            meta->quarantined--;
        }
    }
    
    if(meta->quarantined > 1)
    {
        // Configuramos el criterio global antes de ordenar
        eCrit = 4; 
        
        // Llamamos a tu mSort recursivo directamente sobre el array de cuarentena 
        // mergeSort devuelve una copia asi que no lo necesitamos aqui
        mSort(meta->quarantineArray, 0, meta->quarantined - 1);
    }

    return 0;
}