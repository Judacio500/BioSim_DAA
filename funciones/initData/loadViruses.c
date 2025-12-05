
int loadViruses()
{
    FILE *fp = fopen("viruses.dat", "rb");
    if(!fp)
    {
        return -1;
    }
    
    fread(viruses, sizeof(VIRUS), 50, fp);
        
    fclose(fp);
    printf("Sistema cargado: 50 Cepas virales listas.\n");
    return 0;
}