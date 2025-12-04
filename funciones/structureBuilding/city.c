CITY *createCity(int id, char *name, int population)
{
    CITY *newC = (CITY*)malloc(sizeof(CITY));

    if(!newC)
        return NULL;

    newC->id = id;
    strcpy(newC->name, name);
    newC->population = population;
    newC->people = createGraph(name, createMetadata(population)); 

    return newC;
}