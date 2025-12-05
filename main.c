// Librerias Independientes
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h> 
#include<math.h>
#include<cfloat>


// Librerias Personales
#include"graph.h"
#include"list.h"
#include"hash.h"
#include"heap.h"

// Para la ruta y extensión
char *path = "Ciudades/";
char *ext = ".txt";

// Solo los nombres de las ciudades
char *cityNames[] = 
{
    "Anorlondo",
    "Arabasta",
    "Ashina",
    "Caelid",
    "CityOfTears",
    "Deepnest",
    "Dirtmouth",
    "Konoha",
    "Limgrave",
    "Liurnia",
    "London",
    "Lothric",
    "Majula",
    "Mexico",
    "NewYork",
    "Paris",
    "Skypiea",
    "Tokyo",
    "Wano",
    "Water7"
};

int nCities = 20;
char keyBuffer[20];
int eCrit = 0; // Criterio de evaluacion de personas
int currentSortAlg = 1; // 1: Heap, 2: Quick, 3: Merge
int currentCityIdx = 0; // Ciudad seleccionada para visualizar
int dayCount = 0;       // Contador de dias simulados

typedef enum 
{
    SUSCEPTIBLE,
    INFECTADO,
    VACUNADO,
    RECUPERADO,
    FALLECIDO
} HealthState;

// Wrapper necesario para el Heap en Dijkstra y Prim 
typedef struct DijkstraState 
{
    struct node *currentNode;
    float cost; 
} D_STATE;

typedef struct virus
{
    char name[50];          // AÑADIDO: Clave para el Clustering (Punto 7) y Hash
    float lethality;        // Que tan probable es que desuscriba de la vida a una persona
    float contagiousness;   // Que tan contagioso es, este valor afecta directamente el calculo de riesgo de las adyacencias
    int recovery;           // Tiempo de recuperacion en ciclos de la simulacion
    float mProbablity;      // Probabilidad de mutar en un virus distinto, regularmente baja, pero incrementa linealmente con el recovery
                            // Si un virus se llega a volver muy duradero eventualmente se volvera igual de contagioso -> letal
}VIRUS;

typedef struct person
{
    // La clave hash se construira con estos dos
    // Sera una combinacion del ID unico de la persona para el inicio del hash
    // + Las iniciales de su nombre (extraidas con strtok ya que el formato en la "BD" AKA archivo de texto 
    // AKA txt, sera Nombre1_Nombre2_Apellido1_Apellido2)
    
    int id;             // ID y parte del hash en el grafo
    char name[100];     // Nombre de la persona / Tambien parte de su clave hash en el grafo 
                        // Se tomo esta decision porque un nombre es muy largo para ser una llave Hash
    
    char cityKey[50];      // Llave de la ciudad en la que reside este individuo utilizable en el hash
    char personKey[20];
    HealthState state;       // Estado actual (Sano, Infectado, etc.)
    int daysInfected;        // Contador para saber cuándo se recupera (vs virus->recovery)
    float globalRisk;        // Riesgo calculado globalmente para el Heap de Aislamiento
    
    float infRisk;      // Que tan probable es que este individuo se infecte, un solo valor pero incrementa 
                        // segun la contagiabilidad de los virus de sus adyacencias
    bool quarantine;    // cuando un individuo esta en cuarentena esto es True y el grafo lo ignora para calculos relevantes
    struct virus *v;    // el virus que contrajo la persona

}PERSON;

typedef struct contagion    // Log de contagios, cuando una persona da positivo:
{
    int day;                // Día de la simulación en que ocurrió
    
    // Guardamos nombres/IDs para que el registro persista 
    // incluso si el nodo Persona cambia o se borra (aunque no debería borrarse)
    int sourceId;           // ID del que contagió (Paciente Cero local)
    char sourceName[100];   
    
    int targetId;           // ID del nuevo infectado
    char targetName[100];
    
    char virusName[50];     // Cepa involucrada
}CONTAGION;

typedef struct metadata
{
    struct person **population; // Array intacto de todas las personas en esta ciudad
    int nPopulation;            // Cantidad de personas en esta ciudad
    int quarantined;
    struct list *infectedList;      // Lista rápida de infectados activos
    struct person **quarantineArray; // Array de "en cuarentena" como los espacios para cuarentena seran limitados esto es una decision de diseño perfecta 
    struct queue *contagionHistory; // Cola que guarda el historial de contagios, ordenado por defecto segun el dia en el que sucedio
}MD;

typedef struct city
{
    int id;
    char name[50]; // Tambien cityKey, en el grafo es una cadena dinamica asi que tecnicamente no estan en el mismo lugar (dirección de memoria)
    int population;
    
    GRAPH *people; // El grafo interno de personas
}CITY;

// UTILIDADES 
int clearScreen();
int printConfig();
HealthState stringToState(char *str);
int healthStateToString(HealthState state, char *buffer);

// CONSTRUCTORES
CITY *createCity(int id, char *name, int population);
CONTAGION *createContagion(int day, int sourceId, char *sourceName, int targetId, char *targetName, char *virusName);
D_STATE *createDstate(NODE *state, float cost);
MD *createMetadata(int populationSize);
PERSON *createPerson(int id, char *name, char *cityKey, char *personKey, HealthState init, int virusID);

// CARGA DE DATOS
int loadViruses();
int loadData();
int generateInterCityConnections();

// ALGORITMOS DE ORDENAMIENTO (MANAGER Y RECURSIVOS)
PERSON **heapSort(PERSON **population, int n);
PERSON **mergeSort(PERSON **arr, int low, int high);
int mSort(PERSON **arr, int low, int high);
int merge(PERSON **arr, int low, int m, int high);
PERSON **quickSort(PERSON **arr, int low, int high);
int qSort(PERSON **arr, int low, int high);
int partition(PERSON **arr, int low, int high);
int applySorting(CITY *c);

// ALGORITMOS AUXILIARES
int swap(PERSON **a, PERSON **b);
int copy(PERSON **src, PERSON **dest, int size);
int criteria(PERSON *p1, PERSON *p2, int eval);
int compare(void *person1, void *person2);
int comparePerson(void *person1, void *person2);
int compareRecovery(void *a, void *b);
int compareRisk(void *a, void *b);
int compareDijkstra(void *a, void *b);

// LÓGICA DE SIMULACIÓN Y EPIDEMIA
void simulationCallback(void *data, void *param);
int stepSimulation(int day);
int detectOutbreak(GRAPH *peopleGraph);
int bfsCluster(GRAPH *g, PERSON *start, HASH *visited);

// CUARENTENA
int applySmartQuarantine(CITY *c, int budget);
int releaseRecovered(CITY *c);

// CLUSTERING Y RUTAS
int clusterViruses();
LIST *findCriticalPath(NODE *start, NODE *target);
void destroyFloat(void *data);

// CONSULTAS Y REPORTES 
int printTable(PERSON **arr, int n, char *title);
int reportPerson(char *cityName, char *personName);
int reportCity(char *cityName);

// MENUS
int menuConfiguration();
int menuAnalysis();

// Grafo inicial
// Sus nodos son ciudades
// La clave hash de cada ciudad es su nombre
// Global porque no usamos multi hilo :D
GRAPH *map = NULL;
VIRUS viruses[50];


//                //
//                //
//      MENUS     // 
//                //
//                //


int main(int argc, char const *argv[])
{
    printf("Iniciando BioSim...\n");
    
    // 1. Carga de Datos
    loadData();
    
    // Aseguramos criterio por defecto
    eCrit = 4; // ID

    int option = 0;
    while(option != 4)
    {
        clearScreen();
        printf("========================================\n");
        printf("   BIOSIM - PANEL DE CONTROL\n");
        printf("========================================\n");
        printf("1. Avanzar Simulacion (Siguiente Dia)\n");
        printf("2. Analisis de Datos\n");
        printf("3. Configuracion (Ordenamiento/Vista)\n");
        printf("4. Salir\n");
        printf("----------------------------------------\n");
        printf("Dia Actual: %d\n", dayCount);
        printf("Ciudad Foco: %s\n", cityNames[currentCityIdx]);
        printf("----------------------------------------\n");
        printf("Seleccion: ");
        scanf("%d", &option);

        if(option == 1)
        {
            stepSimulation(dayCount);
            dayCount++;
            
            // Mostramos un resumen rapido de la ciudad foco
            reportCity(cityNames[currentCityIdx]);
            
            printf("\nSimulacion del dia completada.\nPresione Enter...");
            getchar(); getchar();
        }
        else if(option == 2)
        {
            menuAnalysis();
        }
        else if(option == 3)
        {
            menuConfiguration();
        }
    }

    printf("Cerrando sistema...\n");
    return 0;
}


int clearScreen()
{
    // Simulacion de limpieza multiplataforma simple
    printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    return 0;
}

int printConfig()
{
    char *algName = "";
    if(currentSortAlg == 1) algName = "HeapSort";
    else if(currentSortAlg == 2) algName = "QuickSort";
    else algName = "MergeSort";

    char *critName = "";
    switch(eCrit)
    {
        case 1: critName = "Riesgo Infeccion (Desc)"; break;
        case 2: critName = "Dias Infectado (Desc)"; break;
        case 3: critName = "Nombre (A-Z)"; break;
        case 4: critName = "ID (Asc)"; break;
        default: critName = "Default"; break;
    }

    printf(" [CONFIGURACION ACTUAL]\n");
    printf(" Ciudad Objetivo: %s\n", cityNames[currentCityIdx]);
    printf(" Algoritmo:       %s\n", algName);
    printf(" Criterio:        %s\n", critName);
    printf(" Dia Actual:      %d\n", dayCount);
    printf("----------------------------------------\n");
    return 0;
}

int applySorting(CITY *c)
{
    if(!c) 
        return -1;

    MD *meta = (MD*)c->people->metadata;
    PERSON **sorted = NULL;

    // Aplicamos el algoritmo seleccionado como plugin
    // Se genera una nueva vista ordenada
    if(currentSortAlg == 1)
        sorted = heapSort(meta->population, meta->nPopulation);
    else if(currentSortAlg == 2)
        sorted = quickSort(meta->population, 0, meta->nPopulation - 1);
    else
        sorted = mergeSort(meta->population, 0, meta->nPopulation - 1);

    if(sorted)
    {
        // Reemplazo in-place del arreglo principal
        // Liberamos el arreglo viejo de punteros (no los datos) y asignamos el nuevo
        free(meta->population);
        meta->population = sorted;
        return 1;
    }
    return 0;
}

int menuConfiguration()
{
    int option = 0;
    while(option != 4)
    {
        clearScreen();
        printf("=== CONFIGURACION ===\n");
        printf("1. Cambiar Algoritmo de Ordenamiento\n");
        printf("2. Cambiar Criterio de Ordenamiento\n");
        printf("3. Cambiar Ciudad Objetivo (Visualizacion)\n");
        printf("4. Volver\n");
        printConfig();
        printf("Seleccion: ");
        scanf("%d", &option);

        if(option == 1)
        {
            printf("\n1. HeapSort (O(n log n))\n2. QuickSort (O(n log n))\n3. MergeSort (O(n log n))\nElegir: ");
            scanf("%d", &currentSortAlg);
            if(currentSortAlg < 1 || currentSortAlg > 3) 
                currentSortAlg = 1;
        }
        else if(option == 2)
        {
            printf("\n1. Riesgo (Desc)\n2. Dias Enf. (Desc)\n3. Nombre (A-Z)\n4. ID (Asc)\nElegir: ");
            scanf("%d", &eCrit);
            if(eCrit < 1 || eCrit > 4)
                eCrit = 4;
        }
        else if(option == 3)
        {
            printf("\nSeleccione ID de ciudad (0-%d):\n", nCities-1);
            for(int i=0; i<nCities; i++)
                printf("%d. %s\n", i, cityNames[i]);
            
            int newIdx;
            scanf("%d", &newIdx);
            if(newIdx >= 0 && newIdx < nCities)
                currentCityIdx = newIdx;
        }
    }
    return 0;
}

int menuAnalysis()
{
    int option = 0;
    while(option != 5)
    {
        clearScreen();
        printf("=== ANALISIS DE DATOS ===\n");
        printf("1. Tabla de Habitantes (Ordenada)\n");
        printf("2. Clustering de Cepas\n");
        printf("3. Consulta Individual (Ficha Medica)\n");
        printf("4. Ruta Critica (Dijkstra)\n");
        printf("5. Volver\n");
        printConfig();
        printf("Seleccion: ");
        scanf("%d", &option);

        if(option == 1)
        {
            NODE *n = hashNode(map, cityNames[currentCityIdx]);
            if(n)
            {
                CITY *c = (CITY*)n->data;
                
                // Ordenamos in-place antes de mostrar
                applySorting(c);
                
                // Construccion del titulo dinamico
                char titleBuffer[100];
                const char *critStr = (eCrit == 1) ? "Riesgo" : (eCrit == 2) ? "Dias" : (eCrit == 3) ? "Nombre" : "ID"; 
                // Const para que acepte la cadena estatica
                sprintf(titleBuffer, "Habitantes %s Ordenado por %s", c->name, critStr);
                
                MD *meta = (MD*)c->people->metadata;
                printTable(meta->population, meta->nPopulation, titleBuffer);
                
                printf("Presione Enter para continuar...");
                getchar(); getchar();
            }
        }
        else if(option == 2)
        {
            clusterViruses();
            printf("Presione Enter para continuar...");
            getchar(); getchar();
        }
        else if(option == 3)
        {
            char pName[100];
            printf("Ingrese Nombre_Apellido (Ej. Juan_Perez): ");
            scanf("%s", pName);
            
            // Usamos la ciudad seleccionada como contexto
            reportPerson(cityNames[currentCityIdx], pName);
            
            printf("Presione Enter para continuar...");
            getchar(); getchar();
        }
        else if(option == 4)
        {
            char startName[100], targetName[100];
            printf("Origen (Nombre_Apellido): ");
            scanf("%s", startName);
            printf("Destino (Nombre_Apellido): ");
            scanf("%s", targetName);

            NODE *cityNode = hashNode(map, cityNames[currentCityIdx]);
            if(cityNode)
            {
                CITY *c = (CITY*)cityNode->data;
                NODE *nStart = hashNode(c->people, startName);
                NODE *nTarget = hashNode(c->people, targetName);

                if(nStart && nTarget)
                {
                    LIST *pathList = findCriticalPath(nStart, nTarget);
                    if(pathList)
                    {
                        printf("\n[RUTA CRITICA ENCONTRADA]\n");
                        int steps = 0;
                        while(pathList)
                        {
                            NODE *step = (NODE*)pathList->data;
                            PERSON *p = (PERSON*)step->data;
                            printf(" %d. %s (Riesgo: %.2f)\n", steps++, p->name, p->infRisk);
                            
                            if(pathList->next) printf("    |\n    v\n");
                            pathList = pathList->next;
                        }
                        // Liberamos la lista, no los nodos
                        freeList(&pathList, NULL);
                    }
                    else
                    {
                        printf("No existe un camino de riesgo entre estas personas.\n");
                    }
                }
                else
                {
                    printf("Personas no encontradas en %s.\n", c->name);
                }
            }
            printf("Presione Enter para continuar...");
            getchar(); getchar();
        }
    }
    return 0;
}


//                         //
//                         //
//   STRUCTURE INSTANCES   // 
//                         //
//                         //


CITY *createCity(int id, char *name, int population)
{
    CITY *newC = (CITY*)malloc(sizeof(CITY));

    if(!newC)
        return NULL;

    newC->id = id;
    strcpy(newC->name, name);
    newC->population = population;
    newC->people = createGraph(name, createMetadata(population)); 

    return newC;
}

CONTAGION *createContagion(int day, int sourceId, char *sourceName, int targetId, char *targetName, char *virusName)
{
    CONTAGION *newC = (CONTAGION*)malloc(sizeof(CONTAGION));

    if(!newC)
        return NULL;

    // El dia de contagio
    newC->day = day;
    
    // El que contagia
    newC->sourceId = sourceId;
    strcpy(newC->sourceName, sourceName);

    // El contagiado
    newC->targetId = targetId;
    strcpy(newC->targetName, targetName);

    // La cepa
    strcpy(newC->virusName, virusName);

    return newC;
}

D_STATE *createDstate(NODE *state, float cost)
{
    D_STATE *newD = (D_STATE*)malloc(sizeof(D_STATE));

    if(!newD)
        return NULL;

    newD->currentNode = state;
    newD->cost = cost;

    return newD;
}

MD *createMetadata(int populationSize)
{
    MD *newMd = (MD*)malloc(sizeof(MD));

    if(!newMd)
        return NULL;

    newMd->population = (PERSON**)malloc(sizeof(PERSON*) * populationSize);
    
    if (!newMd->population)
    {
        free(newMd);
        return NULL;
    }

    newMd->nPopulation = populationSize;
    newMd->quarantined = 0;
    
    // Auxiliares utilizados bajo demanda
    newMd->infectedList = NULL;
    newMd->quarantineArray = (PERSON**)malloc(sizeof(PERSON*) * (int)(populationSize*0.3)); // La cuarentena solo puede albergar al 30% de las personas
    newMd->contagionHistory = createWrap(); // Este es una cola asi
                                            // Que basada en mi libreria necesita un wrapper

    return newMd;
}

PERSON *createPerson(int id, char *name, char *cityKey, char *personKey, HealthState init, int virusID)
{
    PERSON *newP = (PERSON*)malloc(sizeof(PERSON));

    if(!newP)
        return NULL;

    newP->id = id;
    strcpy(newP->name, name);
    strcpy(newP->cityKey, cityKey);
    strcpy(newP->personKey, personKey);
    newP->state = init;

    if(virusID != -1) // ID -1 significa que la persona no esta infectada al iniciar
        newP->v = &viruses[virusID];
    else
        newP->v = NULL;

    // Calculados al entrar al grafo
    newP->daysInfected = 0;
    newP->globalRisk = 0;
    newP->infRisk = 0;
    newP->quarantine = false;

    return newP;
}


//                         //
//                         //
//   DATA INITIALIZATION   // 
//                         //
//                         //


int loadData()
{
    // cargamos las 50 cepas
    loadViruses();

    map = createGraph("Map", NULL); 

    FILE *fp = NULL;

    char filepath[100];
    char line[256];

    for(int i = 0; i<nCities; i++)
    {
     
        sprintf(filepath, "%s%s%s", path, cityNames[i], ext);
        fp = fopen(filepath, "r");
        if(!fp)
        {
            printf("Error abriendo: %s\n", filepath);
            continue;
        }

        if(!fgets(line, sizeof(line), fp)) 
        {
            fclose(fp);
            continue; 
        }

        // Cada archivo de ciudad guarda el numero de habitantes primero
        int cityPop = atoi(line);

        // Creamos la ciudad con sus metadatos y su grafo de personas
        CITY *c = createCity(i,cityNames[i],cityPop);

        // Añadimos al grafo global la nueva ciudad
        addNode(map, c->name, c);  // Esta funcion incluye hasheo

        int pIndex = 0;
        
        while(fgets(line, sizeof(line), fp))
        {
            // Eliminar salto de linea si existe
            line[strcspn(line, "\r\n")] = 0;

            // Formato: ID,Nombre,Apellido,Estado,VirusID
            
            // 1. ID
            char *token = strtok(line, ",");
            if(!token) continue;
            int id = atoi(token);

            // 2. Nombre
            token = strtok(NULL, ",");
            char firstName[50];
            strcpy(firstName, token);

            // 3. Apellido, concatenado porque el nombre se guarda de manera simple
            token = strtok(NULL, ",");
            char fullName[100];
            sprintf(fullName, "%s_%s", firstName, token);

            // 4. Estado
            token = strtok(NULL, ",");
            HealthState state = stringToState(token);

            // 5. Cepa con la que inicia si corresponde (-1 si no esta infectado)
            token = strtok(NULL, ",");
            int vID = atoi(token);

            // Llave de la persona para el grafo
            char pKey[20];
            sprintf(pKey, "%.3s%d", firstName, id);

            PERSON *p = createPerson(id, fullName, c->name, pKey, state, vID);

            MD *meta = (MD*)c->people->metadata;

            // Nuestros datos se guardaran en 2 niveles
            // En el grafo para operaciones complejas de relacion
            // y en un array para sorting
            meta->population[pIndex] = p;
            addNode(c->people, p->personKey, p);
            
            // Conectamos a cada persona con la anterior para asegurar que el grafo no son nodos sueltos
            // solo para el primer 10% de la poblacion
            // esto nos asegura que el grafo este conectado al 100%
            // el decimo individuo de hecho se conecta ademas aleatoriamente con otros nodos
            if(pIndex > 0 && pIndex <= (int)(0.1*cityPop))
            {
                PERSON *prev = meta->population[pIndex - 1];
                addEdge(c->people, p->personKey, prev->personKey, 1.0f, 2);
            }
            
            // Si ya tenemos el 10% de los elementos de la poblacion:
            if(pIndex >= (int)(0.1 * cityPop))
            {
                // Del 1 al 100 para simular porcentajes de poblacion
                int roll = (rand() % 100) + 1; 
                float connectionPercent; 

                // Asignacion basada en la aproximacion de una distribucion normal
                // Media: 5% | Extremos: 2% y 8%
                
                if (roll <= 5)        // 5% de la poblacion 
                    connectionPercent = 0.02f; 
                else if (roll <= 20)  // 15% de la poblacion
                    connectionPercent = 0.03f; 
                else if (roll <= 40)  // 20% de la poblacion 
                    connectionPercent = 0.04f; 
                else if (roll <= 60)  // 20% de la poblacion 
                    connectionPercent = 0.05f; 
                else if (roll <= 80)  // 20% de la poblacion
                    connectionPercent = 0.06f; 
                else if (roll <= 95)  // 15% de la poblacion
                    connectionPercent = 0.07f; 
                else                  // 5% de la poblacion
                    connectionPercent = 0.08f; 

                int extraConnections = (int)(connectionPercent * cityPop); 
                
                if (extraConnections == 0 && cityPop > 10) 
                    extraConnections = 1;
                
                if (extraConnections > pIndex) 
                    extraConnections = pIndex - 1;

                for(int k = 0; k < extraConnections; k++)
                {
                    int rIdx = rand() % pIndex; 
                    
                    if (rIdx == pIndex) // No nos conectamos a nosotros mismos 
                    { 
                        k--;            // Aseguramos las n conexiones calculadas
                        continue; 
                    } 

                    PERSON *randomNeighbor = meta->population[rIdx];

                    // PESO DINÁMICO
                    // En este modelo las personas mas sociables
                    // son mas propensas a tener lazos mas debiles
                    // esto provoca que aunque tienen mas conexiones (foco de infeccion)
                    // tambien son menos propensos a contagiar
                    float weight;
                    if(connectionPercent > 0.06f)
                        weight = (float)((rand() % 5) + 1) / 10.0f; // 0.1 a 0.5
                    else
                        weight = (float)((rand() % 6) + 5) / 10.0f; // 0.5 a 1.0

                    addEdge(c->people, p->personKey, randomNeighbor->personKey, weight, 2);
                }
            }

            pIndex++;
        }

        fclose(fp);
        // printf("Ciudad cargada: %s (%d habitantes)\n", c->name, pIndex);
    }

    generateInterCityConnections();// Generar conexiones con personas en otras ciudades 
                                   // Para dejar que una enfermedad se propague

    return 0;
}

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

/*

    Veo prudente explicar aqui que utilizamos hashNode + addEdgeThrough 
    porque los nodos pertenecen a distintos grafos y addEdge utiliza el hash
    del grafo para conectarlos, por eso las diferencias:

    addEdge(Grafo, LlaveNodo1, LlaveNodo2, Peso, Tipo_De_Conexion) 

    addEdgeThrough(Nodo1, Nodo2, Peso, Tipo_De_Conexion)

    Una forma eficiente de generar un grafo multicapa

    Y como cada nodo  tiene un puntero a su grafo 
    esto permite hacer exploraciones por todo el grafo o por
    el grafo local
*/

int generateInterCityConnections()
{
    printf("Generando conexiones internacionales...\n");

    // Doble bucle para conectar Ciudad A con Ciudad B esto es O(n^2) 
    for(int i = 0; i < nCities; i++)
    {

        NODE *nodeA = hashNode(map, cityNames[i]);
        if(!nodeA) 
            continue;
        CITY *cityA = (CITY*)nodeA->data;           // Obtenemos la ciudad A
        MD *metaA = (MD*)cityA->people->metadata;

        for(int j = 0; j < nCities; j++)
        {
            if(i == j) 
                continue; // No conectar ciudad consigo misma (pues las conexiones locales ya se generaron anteriormente)

            NODE *nodeB = hashNode(map, cityNames[j]);  // Obtenemos la ciudad B
            if(!nodeB) 
                continue;
            CITY *cityB = (CITY*)nodeB->data;
            MD *metaB = (MD*)cityB->people->metadata;

            // Garantizamos la primera conexion entre ciudades
            // lo que se traduce a que todo el grafo siempre
            // SIEMPRE, esta conectado

            int rA = rand() % metaA->nPopulation;
            int rB = rand() % metaB->nPopulation;
            
            NODE *nA = hashNode(cityA->people, metaA->population[rA]->personKey);
            NODE *nB = hashNode(cityB->people, metaB->population[rB]->personKey);
            
            if(nA && nB)
            {
                 float w = (float)((rand() % 3) + 1) / 10.0f;
                 addEdgeThrough(nA, nB, w, 2);
            }

            // Dejamos fijo el numero de conexiones a un 2%
            int nConnections = (int)ceil(0.002 * cityB->population);
            if(nConnections < 1) nConnections = 1;

            // Para cada habitante de ciudad A
            for(int k = 0; k < metaA->nPopulation; k++)
            {
                // Un 30% de tener conexiones en esa ciudad
                if((rand() % 100) < 30) 
                {
                    PERSON *pA = metaA->population[k];

                    // Buscamos su NODO real en el grafo A (para poder crear la arista)
                    NODE *n1 = hashNode(cityA->people, pA->personKey); // obtenemos el nodo

                    // Generamos las n conexiones calculadas 
                    for(int c = 0; c < nConnections; c++)
                    {
                        // Elegimos un random en Ciudad B
                        int rIdx = rand() % metaB->nPopulation;
                        PERSON *pB = metaB->population[rIdx];
                        
                        // Buscamos su NODO real en el grafo B
                        NODE *n2 = hashNode(cityB->people, pB->personKey);

                        if(n1 && n2) // Seguridad de haber hasheado correctamente los 2 nodos
                        {
                            // PESO DÉBIL (0.1 a 0.3)
                            // Simula contacto esporádico o digital 
                            float weight = (float)((rand() % 3) + 1) / 10.0f;

                            // Usamos addEdgeThrough como explique anteriormente pues vienen de grafos distintos
                            // De hecho esta es la conexion normal de un grafo, regularmente no se conectan por hash X D
                            addEdgeThrough(n1, n2, weight, 2);
                        }
                    }
                }
            }
        }
    }
    printf("Mundo interconectado.\n");
}


//                  //
//                  //
//   ORDENAMIENTO   // 
//                  //
//                  //

// HEAP SORT
PERSON **heapSort(PERSON **population, int n)
{
    // Heapsort es sencillo

    // 1.- Creo mi heap
    HEAP *h = initHeap(n, compare);
    if (!h) return NULL;

    // 2.- Inserto los datos en el heap segun mi criterio
    for (int i = 0; i < n; i++)
    {
        if (population[i] != NULL)
            hPush(h, population[i]);
    }

    // 3.- Creo un array exactamente igual
    PERSON **sortedArr = (PERSON**)malloc(sizeof(PERSON*) * n);
    if (!sortedArr) return NULL;

    // 4.- Hago pop sobre el nuevo array en orden numerico
    //     el heap los acomoda automaticamente
    for (int i = 0; i < n; i++)
    {
        sortedArr[i] = (PERSON*)hPop(h);
    }

    free(h->elements);
    free(h);


    /* HEAP SORT!!!!
    (\(\
    ( -.-)
    o_(")(") 
    */

    return sortedArr;
}

//MERGE SORT
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

//QUICK SORT
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



//              //
//              //
//   DIJKSTRA   // 
//              //
//              //


LIST *findCriticalPath(NODE *start, NODE *target)
{
    if(!start || !target) 
        return NULL;

    printf("[DIJKSTRA] Buscando camino critico de %s a %s...\n",((PERSON*)start->data)->name, ((PERSON*)target->data)->name);

    // Hash para arreglo de distancia y camino
    // el acceso con clave lo hace mas atractivo
    // en terminos de practicidad
    HASH *distMap = initHash(0);
    HASH *parentMap = initHash(0);
    
    // Heap de Prioridad 
    // El tamaño inicial resulta un poco irrelevante ya que se redimensiona solo al llenarse
    HEAP *priorityQueue = initHeap(100, compareDijkstra);

    // Aqui se nota como brilla el hash NOMBRE:VALOR
    // no necesitamos trackear el id de nadie, a nivel logico
    // es mas obvio decirle a la computadora "Fulanito"
    // y sin perder ese valioso tiempo O(1)
    float *startDist = (float*)malloc(sizeof(float));
    *startDist = 0.0f;
    saveKey(&distMap, start->nodeKey, startDist);

    // Push inicial al heap
    D_STATE *initialState = createDstate(start,0);
    
    hPush(priorityQueue, initialState);

    NODE *current = NULL;
    int found = 0;

    while(priorityQueue->size > 0) // Siempre que haya nodos a explorar
    {
        D_STATE *state = (D_STATE*)hPop(priorityQueue);
        current = state->currentNode;
        float currentCost = state->cost;
        free(state); // Liberamos el wrapper, no el nodo

        // Si llegamos al destino, terminamos
        if(current == target) // Comparacion de punteros directa
        {
            found = 1;
            break;
        }

        // Revisar si encontramos un camino mas corto a 'current'
        EHASH *dElem = hashing(distMap, current->nodeKey);
        if(dElem)
        {
            float bestKnown = *(float*)dElem->pair;
            if(currentCost > bestKnown) 
                continue;
        }

        // Exploramos vecinos
        EDGE *edge = current->firstEdge;
        while(edge)
        {
            NODE *neighbor = edge->destinationNode;
            
            // Calculo del Costo de la Arista
            // Peso original (probabilidad): 0.1 a 1.0
            // Costo Dijkstra: -log(peso)
            // Evitamos log(0)

            float prob = edge->weight;
            if(prob <= 0.0001f) prob = 0.0001f; 
            
            float edgeCost = -logf(prob); 
            float newDist = currentCost + edgeCost;

            // Revisar distancia actual conocida
            float oldDist = FLT_MAX; // Distancia mas grande que cualquier otra
                                     // basicamente el centinela para intercambiar la distancia la primera vez
            EHASH *nElem = hashing(distMap, neighbor->nodeKey);
            if(nElem) 
                oldDist = *(float*)nElem->pair;

            // Comprobamos si la distancia es correcta
            if(newDist < oldDist)
            {
                // Actualizamos distancia
                float *newDistPtr = (float*)malloc(sizeof(float));
                *newDistPtr = newDist;
                // saveKey simula actualizacion si ya existe
                // como el metodo de colisiones es chaining
                // el primer elemento del hash con la misma llave es el ultimo insertado
                saveKey(&distMap, neighbor->nodeKey, newDistPtr);

                // Actualizamos padre
                saveKey(&parentMap, neighbor->nodeKey, current);

                // Push al heap
                D_STATE *nextState = createDstate(neighbor,newDist);
                hPush(priorityQueue, nextState);
            }

            edge = edge->nextNode;
        }
    }

    // Reconstruimos el camino
    LIST *pathList = NULL;
    
    if(found)
    {
        NODE *path = target;
        while(path)
        {
            // Insertamos al inicio de la lista para tener orden Origen -> Destino
            insertList(&pathList, path, 0);
            
            if(path == start) 
                break;

            // Buscar al padre de 'path'
            // en nuestro mapa hash, decimos "Quien fue el que tuvo la menor distancia a este nodo?"
            // importante aclarar aqui que el nodo inicial no genero un padre para su llave, por lo que la respuesta sera NULL
            EHASH *pElem = hashing(parentMap, path->nodeKey);
            if(pElem)
                path = (NODE*)pElem->pair; // Cuando lo encontramos actualizamos path y volvemos a intentar
            else
                path = NULL; // Camino roto (no deberia pasar si found=1)
        }
    }

    free(priorityQueue->elements); 
    free(priorityQueue);
    
   // Liberamos el mapa de distancias (Borramos la tabla Y los floats)
    freeHash(&distMap, destroyFloat); 
    
    // Liberamos el mapa de padres (Borramos la tabla pero NO los nodos, porque son del grafo)
    freeHash(&parentMap, NULL);

    return pathList;
}

void destroyFloat(void *data)
{
    if(data) 
        free(data); // ¡Totalmente válido y seguro!
}


//                //
//                //
//   QUARANTINE   // 
//                //
//                //


int applySmartQuarantine(CITY *c, int budget)
{
    if(!c || budget <= 0) 
        return -1;

    MD *meta = (MD*)c->people->metadata;
    int maxCapacity = (int)(meta->nPopulation * 0.3); // Capacidad total del array (30%)

    // Si ya esta lleno el bunker, no cabe nadie mas
    if(meta->quarantined >= maxCapacity)
        return 0;

    // Inicializamos el heap temporal
    HEAP *riskHeap = initHeap(meta->nPopulation, compareRisk);
    if(!riskHeap) 
        return -1;

    // Llenado del Heap
    // Recorremos el array de toda la poblacion linealmente
    for(int i = 0; i < meta->nPopulation; i++)
    {
        PERSON *p = meta->population[i];

        // Solo consideramos candidatos validos:
        // - Vivos
        // - Que no esten en cuarentena
        // - Con riesgo > 0 (si alguien no tiene riesgo no sirve de nada aislarlo)
        if(p->state != FALLECIDO && !p->quarantine && p->globalRisk > 0)
        {
            hPush(riskHeap, p); // Empujamos el nuevo propenso 
        }
    }

    printf("Analisis completado. Candidatos a cuarentena: %d\n", riskHeap->size);

    // Metemos en cuarentena a cuantos hayan sido designados
    int isolatedCount = 0;
    
    // Mientras tengamos presupuesto, candidatos y ESPACIO en el array
    while(isolatedCount < budget && riskHeap->size > 0 && meta->quarantined < maxCapacity)
    {
        PERSON *target = (PERSON*)hPop(riskHeap);
        
        // Aplicar aislamiento
        target->quarantine = true; // Un individuo solo puede salir de cuarentena si...
                                   // se recupero la persona (esto se gestiona en otra función)

        // Lo movemos al array de cuarentena para gestion rapida
        // Usamos el contador actual como indice y LUEGO incrementamos
        meta->quarantineArray[meta->quarantined] = target;
        meta->quarantined++; 

        // Mostramos el dato de registro, esto no se guarda
        printf("\t[!] CUARENTENA: %s (Riesgo: %.2f) -> Aislado. (Total: %d/%d)\n", target->name, target->globalRisk, meta->quarantined, maxCapacity);

        isolatedCount++;
    }

    // Liberamos el heap temporal
    // Las personas siguen en el grafo/array original
    free(riskHeap->elements);
    free(riskHeap);
    
    return isolatedCount; // Retornamos cuantos logramos aislar
}

// Criterio para ordenar la cuarentena:
// Prioridad a los que ya se recuperaron (Estado RECUPERADO va primero)
// O por dias restantes para sanar.

int releaseRecovered(CITY *c)
{
    if(!c) 
        return -1;
    MD *meta = (MD*)c->people->metadata;
    if(meta->quarantined == 0) 
        return -1;

    // Borrado seguro al reves
    for(int i = meta->quarantined - 1; i >= 0; i--)
    {
        PERSON *p = meta->quarantineArray[i];

        // Checamos si ya se recuperó o falleció
        if(p->state == RECUPERADO || p->state == FALLECIDO)
        {
            p->quarantine = false; 

            // Eliminar del Array (Swap con el último, esto deja el espacio libre como si no hubiera pasado nada)
            meta->quarantineArray[i] = meta->quarantineArray[meta->quarantined - 1];
            meta->quarantineArray[meta->quarantined - 1] = NULL;
            meta->quarantined--;
        }
    }
    
    if(meta->quarantined > 1)
    {
        // Configuramos el criterio global antes de ordenar
        eCrit = 4; 
        
        // Llamamos a tu mSort recursivo directamente sobre el array de cuarentena 
        // mergeSort devuelve una copia asi que no lo necesitamos aqui
        mSort(meta->quarantineArray, 0, meta->quarantined - 1);
    }

    return 0;
}


//                //
//                //
//   CLUSTERING   // 
//                //
//                //


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

// Comentarios adicionales:
// El BFS es precioso

int bfsCluster(GRAPH *g, PERSON *start, HASH *visited)
{
    int count = 0;
    QUEUE *toVisit = createWrap();
    
    // Iniciamos BFS
    append(&toVisit, start, 0);
    saveKey(&visited, start->personKey, start); // En lugar de una lista de visitamos utilizamos un hash
                                                // Porque es mas rapido

    // En pocas palabras esto lo que hace es decir
    // Contando desde x Infectado todos estos a su alrededor estan enfermos 
    while(toVisit->first)
    {
        PERSON *current = (PERSON*)dequeueData(toVisit);
        count++;    // Contamos la persona como parte del cluster

        // Sacamos el nodo y vemos sus adyacencias para anexarlas al cluster
        NODE *uNode = hashNode(g, current->personKey);
        if(uNode)
        {
            EDGE *e = uNode->firstEdge;
            while(e)
            {
                PERSON *target = (PERSON*)e->destinationNode->data;
                
                // CONDICIÓN DEL BROTE:
                // Solo nos propagamos si el vecino está INFECTADO
                // y si NO lo hemos contado ya en este barrido.
                if(target->state == INFECTADO)
                {
                    // Ya fue visitado???
                    if(!hashing(visited, target->personKey))
                    {
                        // Si no esta lo marcamos y lo pegamos a la cola
                        saveKey(&visited, target->personKey, target);  
                        append(&toVisit, target, 0);                 
                    }
                }
                e = e->nextNode;
            }
        }
    }
    
    // Liberamos el wrapper de la cola de nodos por visitar
    free(toVisit); 
    return count;
}

// Recibe el grafo de una ciudad y para cada infectado genera un cluster
// Importante aclarar que un brote se puede extender hasta otra ciudad 
// Si A y C son personas de Mexico y NewYork ambos infectados estos dos ya generan un cluster
int detectOutbreak(GRAPH *peopleGraph)
{
    MD *meta = (MD*)peopleGraph->metadata;
    if (!meta || !meta->infectedList) 
        return 0;

    int maxClusterSize = 0;
    int currentClusterSize = 0;

    // Hash en lugar de una lista para no procesar al mismo infectado dos veces
    // (Si A contagió a B, el cluster de A es el mismo que el de B)
    // Tarde o temprano se unen los clusters y se forma una masa
    HASH *visited = initHash(0); 

    // Iteramos sobre los infectados
    LIST *current = meta->infectedList;
    while(current)
    {
        PERSON *p = (PERSON*)current->data;

        // Si no hemos procesado a este infectado en un cluster anterior:
        if (hashing(visited, p->personKey) == NULL)
        {
            // Contamos su cluster
            currentClusterSize = bfsCluster(peopleGraph, p, visited);
            
            if (currentClusterSize > maxClusterSize)
            {
                maxClusterSize = currentClusterSize;
            }
        }
        current = current->next;
    }

    // Limpieza del hash temporal 
    // al pasar NULL como segundo parametro le indicamos al callback
    // que no queremos deshacernos de los datos solo de la estructura
    // que los mostraba
    freeHash(&visited, NULL); 

    return maxClusterSize; // Retorna el tamaño del brote más grande encontrado
}


// Estructura auxiliar para pasar multiples parametros al callback
typedef struct SimulationContext
{
    int currentDay;
    LIST *newInfected;   // Lista temporal para guardar a los que se infectan hoy
    LIST *newRecovered;  // Lista temporal para los que se recuperan hoy
} SC;


//               //
//               //
//   CONSULTAS   // 
//               //
//               //


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

int reportPerson(char *cityName, char *personName)
{
    // Hashing de la ciudad en el mapa
    NODE *cityNode = hashNode(map, cityName);
    if(!cityNode)
    {
        printf("\n [ERROR] La ciudad '%s' no existe en el mapa.\n", cityName);
        return -1;
    }

    CITY *c = (CITY*)cityNode->data;
    
    // Hashing a la persona en la ciudad especificada
    // c->people es un grafo
    NODE *personNode = hashNode(c->people, personName);
    if(!personNode)
    {
        printf("\n [ERROR] '%s' no es residente de %s.\n", personName, c->name);
        return -1;
    }

    PERSON *p = (PERSON*)personNode->data;

    // Impresion Bonita
    printf("\n========================================\n");
    printf(" FICHA MEDICA INDIVIDUAL\n");
    printf("========================================\n");
    printf(" ID: \t\t%d\n", p->id);
    printf(" Nombre: \t%s\n", p->name);
    printf(" Residencia: \t%s\n", c->name);
    
    char stateStr[25];
    
    healthStateToString(p->state,stateStr);

    printf(" Estado: \t%s\n", stateStr);
    
    if(p->v)
        printf(" Portador de: \t%s (Letalidad: %.2f)\n", p->v->name, p->v->lethality);
    
    printf(" Riesgo Global: %.4f (Potencial de contagio)\n", p->globalRisk);
    printf(" Estatus: \t%s\n", p->quarantine ? "[ EN CUARENTENA ]" : "LIBRE");
    printf("========================================\n");
}

int reportCity(char *cityName)
{
    NODE *cityNode = hashNode(map, cityName);
    if(!cityNode)
    {
        printf("\n [ERROR] La ciudad '%s' no existe.\n", cityName);
        return -1;
    }

    CITY *c = (CITY*)cityNode->data;
    MD *meta = (MD*)c->people->metadata;

    printf("\n========================================\n");
    printf(" REPORTE DE ZONA: %s\n", c->name);
    printf("========================================\n");
    printf(" Poblacion Total: \t%d\n", meta->nPopulation);
    
    printf("Personas en Cuarentena: %d\n", meta->quarantined);
    
    // REPORTE O(1) DEL HISTORIAL (Queue)
    QUEUE *q = meta->contagionHistory;
    
    printf("\n --- HISTORIAL DE CONTAGIOS (O(1)) ---\n");
    if(q && q->first)
    {
        // Caso mas antiguo
        CONTAGION *firstCase = (CONTAGION*)q->first->data;
        printf(" [PRIMER REGISTRO]\n");
        printf("   Dia: %d | %s contagio a %s\n", firstCase->day, firstCase->sourceName, firstCase->targetName);
        printf("   Cepa: %s\n", firstCase->virusName);

        // Caso mas reciente
        if(q->last && q->last != q->first)
        {
            CONTAGION *lastCase = (CONTAGION*)q->last->data;
            printf("\n [ULTIMO REGISTRO]\n");
            printf("   Dia: %d | %s contagio a %s\n", 
                   lastCase->day, lastCase->sourceName, lastCase->targetName);
            printf("   Cepa: %s\n", lastCase->virusName);
        }
    }
    else
    {
        printf(" [!] No se han registrado contagios en esta zona.\n");
    }
    printf("========================================\n");
}


//                //
//                //
//   SIMULACIÓN   // 
//                //
//                //


// Callback utilizado en traverseGraph
// para hacer todos los calculos de la población 
void simulationCallback(void *data, void *param)
{
    NODE *currentNode = (NODE*)data;
    if(!currentNode) 
        return;

    PERSON *p = (PERSON*)currentNode->data;
    SC *ctx = (SC*)param;

    float infWeight = 0; // Peso combinado de adyacencias contagiadas
    float myConnectivity = 0; // Riesgo global calculado puramente de cantidad de adyacencias

    EDGE *edge = currentNode->firstEdge;
    
    NODE *virusSources[100]; // Array simple para candidatos a contagiarme
    float sourceProbabilities[100];
    int candidatesCount = 0;

    while(edge)
    {
        NODE *neighborNode = edge->destinationNode;
        PERSON *neighbor = (PERSON*)neighborNode->data;

        // Sumamos el peso de cada adyacencia para determinar la conectividad de una persona
        // Que tan propenso es a contagiar
        // Aqui se balancean un poco las cosas con las conexiones
        // Aqui contagia mas quien tiene ConexionFuerte + MuchasConexiones
        myConnectivity += edge->weight;

        // Revisamos si el vecino esta infectado para proceder
        if(neighbor->state == INFECTADO && neighbor->v != NULL)
        {
            // Probabilidad de que ESTE vecino me contagie
            float weight = neighbor->v->contagiousness * edge->weight;
            
            // Factor de reducción la persona ya se habia contagiado
            // genero anticuerpos
            if(p->state == RECUPERADO) 
                weight *= 0.1f; // (Corregido a 0.1f para dar resistencia, no inmunidad total)
            else if(p->state == VACUNADO) 
                weight = 0.0f;

            if(weight > 0 && candidatesCount < 100)
            {
                virusSources[candidatesCount] = neighborNode;
                sourceProbabilities[candidatesCount] = weight;
                candidatesCount++;
                infWeight += weight;
            }
        }
        edge = edge->nextNode;
    }

    // Actualizamos metricas
    p->infRisk = infWeight; 
    if(p->state == INFECTADO && p->v)
        p->globalRisk = myConnectivity * p->v->contagiousness;
    else
        p->globalRisk = myConnectivity * 0.5f; // Potencial (con virus promedio)


    if(p->state == INFECTADO)
    {
        p->daysInfected++; // Aumentamos los dias infectado de esta persona

        if(p->v)
        {
            // La letalidad es la probabilidad de morir CADA DÍA que estás enfermo
            // distribuida linealmente en el periodo de enfermedad

            // Para suavizar, usamos letalidad / recoveryTime que distribuye la probabilidad de morir en los dias de enfermedad
            float dailyLethality = p->v->lethality / p->v->recovery; 

            float deathRoll = (float)(rand() % 1000) / 1000.0f;
            if(deathRoll < dailyLethality)
            {
                p->state = FALLECIDO;
                p->v = NULL; // El virus muere con el huesped
                return; // Terminamos este nodo porque ya no puede hacer nada
            }

            float mutationRoll = (float)(rand() % 1000) / 1000.0f;
            if(mutationRoll < p->v->mProbablity)
            {
                // ¡MUTA!
                // Elegimos un nuevo virus aleatorio del catálogo global
                int newVirusID = rand() % 50; // 50 cepas
                p->v = &viruses[newVirusID];
            }
        }

        if(p->v && p->daysInfected >= p->v->recovery) // Revisamos si de acuerdo a su virus ya se mejoro
        {
            // Lo agendamos para recuperacion al final del dia para que sus adyacencias no lo consideren como recuperado en el estado de hoy
            insertList(&ctx->newRecovered, p, 0);
        }
        return; // Un infectado no se vuelve a infectar (ese dia)
    }

    if((p->state == SUSCEPTIBLE || p->state == RECUPERADO) && candidatesCount > 0)
    {
        // ¿Me contagio?
        // Usamos una probabilidad compuesta simple basada en el peso total de infección
        float chance = infWeight; 
        if(chance > 1.0f) chance = 1.0f;

        // Tirada
        float roll = (float)(rand() % 1000) / 1000.0f;

        if(roll < chance)
        {
            // ¡CONTAGIADO!
            
            // ¿Quién fue? 
            // Elegimos cual de los vecinos fue el culpable basado en su aporte al peso de infección
            float roulette = (float)(rand() % 1000) / 1000.0f * infWeight;
            float cumsum = 0.0f;
            PERSON *sourcePatient = NULL;

            for(int i=0; i < candidatesCount; i++)
            {
                cumsum += sourceProbabilities[i];
                if(roulette <= cumsum)
                {
                    sourcePatient = (PERSON*)virusSources[i]->data;
                    break;
                }
            }
            
            if(!sourcePatient) 
                sourcePatient = (PERSON*)virusSources[0]->data;

            // Preparamos el Log de Contagio
            CONTAGION *log = createContagion(ctx->currentDay, sourcePatient->id, sourcePatient->name, p->id, p->name, sourcePatient->v->name);
            
            p->v = sourcePatient->v;    // Hereda cepa no hay problema con los calculos porque
                                        // aun no esta marcado como infectado
            
            // Todos los nodos apuntan a su grafo que guarda su metaData
            if(currentNode->graph && currentNode->graph->metadata)
            {
                MD *cityMeta = (MD*)currentNode->graph->metadata;
                append(&cityMeta->contagionHistory, log, 0); // Encolamos el log del contagio a los contagios por ciudad
            }

            // Agendamos el cambio de estado al finalizar
            insertList(&ctx->newInfected, p, 0);
        }
    }
}

int stepSimulation(int day)
{
    printf("--- Simulando Dia %d ---\n", day);

    SC ctx;
    ctx.currentDay = day;
    ctx.newInfected = NULL;
    ctx.newRecovered = NULL;

    // Solo necesitamos un punto de entrada válido al Mapa de personas.
    // Usamos el primer nodo de la primera ciudad disponible.
    // Gracias a las conexiones inter-ciudades, traverseGraph saltará de país en país.
    // Recordemos que garantizamos la primera conexion asi que el grafo es
    // valgame la redundancia, conexo
    
    NODE *entryPoint = NULL;
    
    // Buscamos una entrada válida (por seguridad, si la ciudad 0 estuviera vacía)
    for(int i=0; i < nCities; i++)
    {
        NODE *cityNode = hashNode(map, cityNames[i]);
        if(cityNode)
        {
            CITY *c = (CITY*)cityNode->data;
            if(c->people && c->people->currentNode)
            {
                entryPoint = c->people->currentNode;
                break; // ¡Encontramos la entrada a la Red!
            }
        }
    }

    if(entryPoint)
    {
        LIST *visited = NULL; // Control de ciclos global
        traverseGraphWParameter(&visited, entryPoint, &ctx, simulationCallback);
        freeList(&visited, NULL); 
    }
    else
    {
        printf("El mundo esta vacio.\n"); // Debugging por si los archivos no cargaran
                                          // esta funcion seria lo primero en saltar a la vista 
    }
    
    // Despues de los calculos finaliza el dia y cambiamos el estado de infección
    while(ctx.newInfected)
    {
        PERSON *p = (PERSON*)popData(&ctx.newInfected);
        p->state = INFECTADO;
        p->daysInfected = 0;
        
        // Agregar a la lista de infectados de su ciudad
        NODE *cNode = hashNode(map, p->cityKey);
        if(cNode) {
            CITY *c = (CITY*)cNode->data;
            MD *m = (MD*)c->people->metadata;
            insertList(&m->infectedList, p, 0);
        }
    }

    // Luego anexamos a los recien recuperados
    while(ctx.newRecovered)
    {
        PERSON *p = (PERSON*)popData(&ctx.newRecovered);
        p->state = RECUPERADO;
        p->v = NULL; 
        p->daysInfected = 0;
        
        // Para removerlo de lista de infectados
        NODE *cNode = hashNode(map, p->cityKey);
        CITY *c = (CITY*)cNode->data;
        MD *m = (MD*)c->people->metadata;
        extract(&m->infectedList,p,comparePerson);
    }

    // Gestión de Cuarentena (Liberar y Aislar)
    for(int i=0; i < nCities; i++)
    {
        NODE *node = hashNode(map, cityNames[i]);
        if(node)
        {
            CITY *c = (CITY*)node->data;
            releaseRecovered(c);          // Liberamos a los que ya sanaron
            applySmartQuarantine(c, 5);   // Aislamos a los más peligrosos (Presupuesto diario fijo: 5)
        }
    }

    return 0;
}

int comparePerson(void *person1, void *person2)
{
    PERSON *p1 = (PERSON*)person1;
    PERSON *p2 = (PERSON*)person2;

    return p1->id == p2->id ? 1 : 0;
}


//                //
//                //
//   AUXILIARES   // 
//                //
//                //


int compareDijkstra(void *a, void *b) // AUXILIA -> DIJKSTRA
{
    D_STATE *s1 = (D_STATE*)a;
    D_STATE *s2 = (D_STATE*)b;

    return (s1->cost < s2->cost);
}

HealthState stringToState(char *str) // AUXILIA -> ENUM HEALTHSTATE
{
    if(strcmp(str, "SUSCEPTIBLE") == 0) 
        return SUSCEPTIBLE;
    if(strcmp(str, "INFECTADO") == 0) 
        return INFECTADO;
    if(strcmp(str, "VACUNADO") == 0) 
        return VACUNADO;
    if(strcmp(str, "RECUPERADO") == 0) 
        return RECUPERADO;
    
    return FALLECIDO;
}

int compareRecovery(void *a, void *b) // AUXILIA -> HEAP_CUARENTENA
{
    PERSON *p1 = (PERSON*)a;
    PERSON *p2 = (PERSON*)b;

    // Si p1 es RECUPERADO y p2 no, p1 va primero
    if (p1->state == RECUPERADO && p2->state != RECUPERADO) 
        return 1;
    if (p1->state != RECUPERADO && p2->state == RECUPERADO) 
        return 0;

    // Si ambos estan igual, desempatamos por dias infectado (quien lleva mas tiempo)
    return (p1->daysInfected > p2->daysInfected);
}

int compareRisk(void *a, void *b) // AUXILIA -> HEAP_RIESGO
{              
    PERSON *p1 = (PERSON*)a;
    PERSON *p2 = (PERSON*)b;
    
    // Queremos que flote el de MAYOR riesgo
    return (p1->globalRisk > p2->globalRisk);
}


int swap(PERSON **a, PERSON **b) // AUXILIA -> QUICK SORT
{
    PERSON *temp = *a;

    if(!*a && !*b)
        return -1;

    *a = *b;
    *b = temp;

    return 0;
}

int criteria(PERSON *p1, PERSON *p2, int eval) // AUXILIA -> SORTING EN GENERAL
{
    switch(eval)
    {
        case 1:
            return (p1->infRisk > p2->infRisk);

        case 2:
            return (p1->daysInfected > p2->daysInfected);

        case 3: 
            return (strcmp(p1->name, p2->name) < 0);
            
        case 4:
            return (p1->id < p2->id);
            
        default: 
            return 0;
    }
}

int copy(PERSON **src, PERSON **dest, int size) // AUXILIA MERGE SORT Y QUICK SORT 
{
    if(!src || !dest)
        return NULL;

    for(int i=0; i<size; i++)
        dest[i] = src[i]; // Apuntamos al mismo struct persona pero el arreglo es distinto

    return 0;
}

// AUXILIA -> HEAP.H
int compare(void *person1, void *person2) // Callback necesario en el heap, al ser generico las comparaciones deben ser definidas por el usuario
{
    return criteria((PERSON*)person1, (PERSON*)person2, eCrit);
}

/*
    La funcion criteria realiza las evaluaciones de persona 1 y persona 2 respetando
    la jerarquia de la libreria: Izquierda Comparacion Derecha

    Y gracias a esto permite en un solo heapSort generar 4 acomodos distintos
    Lo que se traduce a 4 heaps con criterios de ordenamiento distinto

    Max/Min
*/

// Copia la representación en texto del estado al buffer proporcionado
int healthStateToString(HealthState state, char *buffer) // AUXILIA -> HEALTHSTATE EN GENERAL
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