#include "hash.h"

int primeNumbers[] = {97,997,5003,10007,15013,20011,25013,30011,35023,40009,45007,49999};


HASH *initHash(int maxSpace)
{
    HASH *newH = (HASH*)malloc(sizeof(HASH));
    if (!newH)
        return NULL;
    if (maxSpace > 5)
        return NULL;

    newH->maxSpace = maxSpace;
    newH->numberOfKeys = 0;
    newH->elements = (EHASH**)malloc(sizeof(EHASH*) * primeNumbers[maxSpace]);

    for (int i = 0; i < primeNumbers[maxSpace]; i++)
        newH->elements[i] = NULL;

    return newH;
}

EHASH *newElem(char *key, void *pair)
{
    EHASH *newE = (EHASH*)malloc(sizeof(EHASH));
    if (!newE)
        return NULL;

    newE->key = strdup(key);
    newE->pair = pair;
    newE->next = NULL;
    return newE;
}

int hashFunction(char *key, int maxSpace) // JDB2
{
    int hash = 5381;
    char *c = key;
    while (*c != '\0')
    {
        hash = hash * 33 + *c;
        c++;
    }
    return hash % primeNumbers[maxSpace];
}

int saveKey(HASH **hT, char *key, void *pair)
{
    int EC_1 = 0, EC_2 = 0;

    if ((*hT)->numberOfKeys >= primeNumbers[(*hT)->maxSpace])
        EC_1 = reHashing(hT);

    if (EC_1 < 0)
        return -1;

    int index = hashFunction(key, (*hT)->maxSpace);
    EC_2 = insertEhash(&(*hT)->elements[index], key, pair);

    if (EC_2 < 0)
        return -1;

    (*hT)->numberOfKeys++;
    return 0;
}

int insertEhash(EHASH **chainingList, char *key, void *pair)
{
    EHASH *newE = newElem(key, pair);
    if (!newE)
        return -1;

    newE->next = *chainingList;
    *chainingList = newE;
    return 0;
}

EHASH *hashing(HASH *hT, char *key)
{
    int index = hashFunction(key, hT->maxSpace);
    EHASH *element = hT->elements[index];

    while (element)
    {
        if (!strcmp(key, element->key))
            return element;
        element = element->next;
    }
    return NULL;
}

int reHashing(HASH **hT)
{
    HASH *newHash = initHash((*hT)->maxSpace + 1);
    if (!newHash)
        return -1;

    EHASH *element = NULL;
    for (int i = 0; i < primeNumbers[(*hT)->maxSpace]; i++)
    {
        element = (*hT)->elements[i];
        while (element)
        {
            saveKey(&newHash, element->key, element->pair);
            element = element->next;
        }
    }

    freeHash(hT, NULL);
    *hT = newHash;
    return 0;
}

int freeHash(HASH **hT, Destructor freeData)
{
    if (!hT || !*hT)
        return -1;

    EHASH *element = NULL;

    for (int i = 0; i < primeNumbers[(*hT)->maxSpace]; i++)
    {
        element = (*hT)->elements[i];
        if (!element)
            continue;

        if (element->next)
            freeChainingList(&(*hT)->elements[i], freeData);
        else
        {
            if (freeData)
                freeData(element->pair);
            free(element->key);
            free(element);
        }
    }

    free((*hT)->elements);
    free(*hT);
    *hT = NULL;
    return 0;
}

int freeChainingList(EHASH **element, Destructor freeData)
{
    if (!element || !*element)
        return -1;

    EHASH *aux = NULL, *next = NULL;

    while (*element)
    {
        aux = *element;
        next = aux->next;
        if (freeData)
            freeData(aux->pair);
        free(aux->key);
        *element = next;
        free(aux);
    }

    *element = NULL;
    return 0;
}
