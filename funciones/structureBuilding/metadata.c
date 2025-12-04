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
    
    // Auxiliares utilizados bajo demanda
    newMd->utilities = NULL;
    newMd->infectedList = NULL;
    newMd->quarantineList = NULL;
    newMd->contagionHistory = createWrap(); // Este es una cola asi
                                            // Que basada en mi libreria necesita un wrapper

    return newMd;
}