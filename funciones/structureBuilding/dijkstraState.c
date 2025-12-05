D_STATE *createDstate(NODE *state, float cost)
{
    D_STATE *newD = (D_STATE*)malloc(sizeof(D_STATE));

    if(!newD)
        return NULL;

    newD->currentNode = state;
    newD->cost = cost;

    return newD;
}