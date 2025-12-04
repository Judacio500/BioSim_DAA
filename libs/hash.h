#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int primeNumbers[];

typedef void(*Destructor)(void *data);

typedef struct hashElem
{
    char *key;
    void *pair;
    struct hashElem *next;
} EHASH;

typedef struct hash
{
    struct hashElem **elements;
    int maxSpace;
    int numberOfKeys;
} HASH;

HASH *initHash(int maxSpace);
EHASH *newElem(char *key, void *pair);
int hashFunction(char *key, int maxSpace);
int saveKey(HASH **hT, char *key, void *pair);
int insertEhash(EHASH **chainingList, char *key, void *pair);
EHASH *hashing(HASH *hT, char *key);
int reHashing(HASH **hT);
int freeHash(HASH **hT, Destructor freeData);
int freeChainingList(EHASH **element, Destructor freeData);

#endif
