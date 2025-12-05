int swap(PERSON **a, PERSON **b)
{
    PERSON *temp = *a;

    if(!*a && !*b)
        return -1;

    *a = *b;
    *b = temp;

    return 0;
}