int printTable(PERSON **arr, int n, char *title)
{
    if(!arr || n <= 0) 
        return -1;

    printf("\n=== %s ===\n", title);
    printf("%-5s | %-25s | %-12s | %-15s | %-10s | %-6s\n", 
           "ID", "Nombre", "Estado", "Cepa", "Riesgo(G)", "Dias");
    printf("--------------------------------------------------------------------------------------\n");

    for(int i = 0; i < n; i++)
    {
        PERSON *p = arr[i];
        if(!p) 
            continue;

        char stateStr[25];
        
        healthStateToString(p->state,stateStr);

        char virusName[20] = "Ninguna";
        if(p->v) 
            strncpy(virusName, p->v->name, 19);

        printf("%-5d | %-25s | %-12s | %-15s | %-10.4f | %-6d\n",p->id, p->name, stateStr, virusName, p->globalRisk, p->daysInfected);
    }
    printf("--------------------------------------------------------------------------------------\n");
    printf("Total listados: %d\n\n", n);
}