CONTAGION *createContagion(int day, int sourceId, char *sourceName, int targetId, char *targetName, char *virusName)
{
    CONTAGION *newC = (CONTAGION*)malloc(sizeof(CONTAGION));

    if(!newC)
        return NULL;

    // El dia de contagio
    newC->day = day;
    
    // El que contagia
    newC->sourceId = sourceId;
    strcpy(newC->sourceName, sourceName);

    // El contagiado
    newC->targetId = targetId;
    strcpy(newC->targetName, targetName);

    // La cepa
    strcpy(newC->virusName, virusName);

    return newC;
}