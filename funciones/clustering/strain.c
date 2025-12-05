int clusterViruses()
{
    printf("\n=== CLUSTERING DE CEPAS (Analisis de Variantes) ===\n");
    
    // Las tablas hash resolvieron todo en este proyecto
    // La L viene de la longitud de la cadena que el hash procesa
    HASH *clusterTable = initHash(0);
    
    if(!clusterTable) 
        return -1;

    char familyKey[50];
    
    for(int i = 0; i < 50; i++)
    {
        VIRUS *v = &viruses[i];
        
        // Generar Clave de Familia (Primera palabra antes de espacio o guion)
        // Si ya de plano se insertaron virus con nombres similares
        // esto tambien permite agruparlos directamente por esas similitudes
        int j = 0;
        while(v->name[j] != '\0' && v->name[j] != ' ' && v->name[j] != '-')
        {
            familyKey[j] = v->name[j];
            j++;
        }
        familyKey[j] = '\0'; // Caracter Nulo al final
        
        // Insertar en el Hash
        // al encontrar 2 variantes las encadena
        // y eso basicamente genera una familia, todo lo que no pertenezca a la misma familia
        // deberia hasheara en otro lado la mayor parte de las veces
        saveKey(&clusterTable, familyKey, v);
    }

    // Vamos a contar cuantas estructuras del  hash se llenaron para ver los clusters de familias
    int foundClusters = 0;
    
    // La tabla hash tiene como primer tamaño 97
    // Usa jdb2 como funcion hash, por eso necesita numeros primoss
    for(int i = 0; i < 97; i++) 
    {
        EHASH *node = clusterTable->elements[i];
        
        if(node)
        {
            foundClusters++;

            printf("\n [Familia Viral: %s]\n", node->key);
            
            while(node)
            {
                VIRUS *v = (VIRUS*)node->pair;
                printf("   -> Cepa: %-15s | R0: %.2f | Letalidad: %.2f\n", 
                       v->name, v->contagiousness, v->lethality);
                
                node = node->next;
            }
        }
    }
    
    printf("\nTotal de familias detectadas: %d\n", foundClusters);

    freeHash(&clusterTable, NULL); 

    return 0;
}