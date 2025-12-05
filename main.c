// Librerias Independientes
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h> 

// Librerias Personales
#include"graph.h"
#include"list.h"
#include"hash.h"
#include"heap.h"

VIRUS viruses[50];

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
    float accumulatedCost; // Costo acumulado o probabilidad logarítmica
    struct node *previousNode;
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
    struct list *infectedList;      // Lista rápida de infectados activos
    struct list *quarantineList;    // Lista rápida de gente aislada
    struct queue *contagionHistory; // Cola que guarda el historial de contagios, ordenado por defecto segun el dia en el que sucedio
}MD;

typedef struct city
{
    int id;
    char name[50]; // Tambien cityKey, en el grafo es una cadena dinamica asi que tecnicamente no estan en el mismo lugar (dirección de memoria)
    int population;
    
    GRAPH *people; // El grafo interno de personas
}CITY;

MD *createMetadata();

// Grafo inicial
// Sus nodos son ciudades
// La clave hash de cada ciudad es su nombre
// Global porque no usamos multi hilo :D
GRAPH *map = NULL;

int main(int argc, char const *argv[])
{

}

