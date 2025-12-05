
PERSON **quickSort(PERSON **arr, int low, int high)
{
   // Esta funcion trabaja como un manager de quickSort
   // Al ser recursivo necesitamos que pase por aqui para hacer el partition in situ
   // en un array nuevo, no es un deep copy, es una vista distinta de las mismas estructuras
   
    PERSON **mockUp = (PERSON**)malloc(sizeof(PERSON*)*(high+1)); // Ultimo indice + 1 = Tamaño
    copy(arr,mockUp,high+1);

    qSort(mockUp,0,high);

    return mockUp;
}

int qSort(PERSON **arr, int low, int high)
{
    if(low>=high)
        return 0;

    int p = partition(arr, low, high);
    qSort(arr,low,p-1);
    qSort(arr,p+1,high);
    return 0;
}

int partition(PERSON **arr, int low, int high)
{
    int i = low, j = high-1;
    PERSON *pivot = arr[high];

    while(i<=j)
    {
        while(i<high && criteria(arr[i],pivot,eCrit))
            i++;
        
        while(j>=low && !criteria(arr[j],pivot,eCrit))
            j--;

        if(i <= j)
        {
            swap(&arr[i], &arr[j]);
            i++;
            j--;
        }
    }

    swap(&arr[i],&arr[high]);
    return i;
}

