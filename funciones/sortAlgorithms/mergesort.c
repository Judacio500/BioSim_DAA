
PERSON **mergeSort(PERSON **arr, int low, int high)
{
    PERSON **mockUp = (PERSON**)malloc(sizeof(PERSON*)*(high+1));
    copy(arr,mockUp,high+1);

    mSort(mockUp,low,high);

    return mockUp;
}

int mSort(PERSON **arr, int low, int high)
{
    if(low < high)
    {
        int m = low + (high - low) / 2;

        mSort(arr, low, m);
        mSort(arr, m + 1, high);
        merge(arr, low, m, high);
    }
    return 0;
}

int merge(PERSON **arr, int low, int m, int high)
{
    int i, j, k;
    int n1 = m - low + 1;
    int n2 = high - m;

    PERSON **L = (PERSON**)malloc(sizeof(PERSON*)*n1);
    PERSON **R = (PERSON**)malloc(sizeof(PERSON*)*n2);

    for(i=0; i < n1; i++)
        L[i] = arr[low + i];
    for(j=0; j < n2; j++)
        R[j] = arr[m + 1 + j];

    i = 0; 
    j = 0; 
    k = low; 

    while(i < n1 && j < n2) 
    {
        if(criteria(L[i], R[j], eCrit)) 
        {
            arr[k] = L[i];
            i++;
        }
        else 
        {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while(i<n1) 
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    while(j<n2) 
    {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);

    return 0;
}