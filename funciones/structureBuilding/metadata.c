MD *createMetadata(int populationSize)
{
    MD *newMd = (MD*)malloc(sizeof(MD));

    if(!newMd)
        return NULL;

    newMd->population = (PERSON**)malloc(sizeof(PERSON*) * populationSize);
    
    if (!newMd->population)
    {
        free(newMd);
        return NULL;
    }

    newMd->nPopulation = populationSize;
    newMd->quarantined = 0;
    
    // Auxiliares utilizados bajo demanda
    newMd->infectedList = NULL;
    newMd->quarantineList = (PERSON**)malloc(sizeof(PERSON*) * (int)(populationSize*0.3)); // La cuarentena solo puede albergar al 30% de las personas
    newMd->contagionHistory = createWrap(); // Este es una cola asi
                                            // Que basada en mi libreria necesita un wrapper

    return newMd;
}