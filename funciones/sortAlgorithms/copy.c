int copy(PERSON **src, PERSON *dest, int size)
{
    if(!src || !dest)
        return NULL;

    for(int i=0; i<size; i++)
        dest[i] = src[i]; // Apuntamos al mismo struct persona pero el arreglo es distinto

    return 0;
}