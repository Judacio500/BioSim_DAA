// Copia la representación en texto del estado al buffer proporcionado
int healthStateToString(HealthState state, char *buffer)
{
    switch(state) {
        case SUSCEPTIBLE: 
            strcpy(buffer, "SANO"); 
            break;
        case INFECTADO:   
            strcpy(buffer, "INFECTADO"); 
            break;
        case VACUNADO:    
            strcpy(buffer, "VACUNADO"); 
            break;
        case RECUPERADO:  
            strcpy(buffer, "RECUPERADO"); 
            break;
        case FALLECIDO:   
            strcpy(buffer, "FALLECIDO"); 
            break;
        default:          
            strcpy(buffer, "DESCONOCIDO"); 
            break;
    }

    return 0;
}