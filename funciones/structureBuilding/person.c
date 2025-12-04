PERSON *createPerson(int id, char *name, char *cityKey, HealthState init, int virusID)
{
    PERSON *newP = (PERSON*)malloc(sizeof(PERSON));

    if(!newP)
        return NULL;

    newP->id = id;
    strcpy(newP->name, name);
    strcpy(newP->cityKey, cityKey);
    newP->state = init;

    if(virusID != -1) // ID -1 significa que la persona no esta infectada al iniciar
        newP->v = &viruses[virusID];
    else
        newP->v = NULL;

    // Calculados al entrar al grafo
    newP->daysInfected = 0;
    newP->globalRisk = 0;
    newP->infRisk = 0;
    newP->quarantine = false;

    return newP;
}