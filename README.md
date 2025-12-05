# 🗺️ Hoja de Ruta: BioSim

Plan de implementación para el Simulador Algorítmico de Epidemias.

## 1. Inicialización y Análisis de Datos (Los 3 Sorts)
**Explicación:**
El objetivo es cargar la "BD" desde un archivo de texto, instanciar las estructuras y demostrar 4 algoritmos de ordenamiento de complejidad $O(n \log n)$.
* **Estrategia:** No insertar directo al grafo. Leer archivo -> Llenar array `population` en Metadata -> Ejecutar Sorts sobre el array -> Poblar el grafo `city->people`.

**Checklist:**
- [x] `Constructores de estructuras`: PERSONA, CIUDAD, LOG DE CONTAGIOS Y METADATA
- [x] `int loadData()`: Carga todos los datos de Ciudades/Personas/Cepas.
- [x] `int loadViruses()`: Carga las 50 cepas del archivo de datos.
- [x] `PERSON **mergeSort(PERSON **arr, int low, int high)`: Algoritmo 1 .
- [x] `PERSON **quickSort(PERSON **arr, int low, int high)`: Algoritmo 2 .
- [x] `PERSON **heapSort(PERSON **arr, int n)`: Algoritmo 3 (Usando librería `heap.h`).
- [x] `int interCityConections()`: Inserta nodos y aristas en el grafo entre ciudades.

## 2. Detección de Brotes
**Explicación:**
Identificar clusters de infección activa sin recorrer todo el grafo ineficientemente.
* **Estrategia:** Iterar únicamente la `infectedList` de la metadata para iniciar búsquedas locales (BFS) limitadas por profundidad o zona.

**Checklist:**
- [x] `int detectOutbreak(GRAPH *peopleGraph)`: Itera infectados y retorna el tamaño del cluster conectado.
- [x] `int bfsCluster(GRAPH *g, PERSON *start, HASH *visited)`: Funcion auxiliar de BFS para escanear las adyacencias.

## 3. Propagación Temporal (Simulación)
**Explicación:**
Avanzar el tiempo $t \to t+1$, gestionando nuevos contagios y recuperaciones.
* **Estrategia:** Iterar solo `infectedList`.
    * **Cura:** Si `daysInfected >= recovery` -> `RECUPERADO`.
    * **Contagio:** Probabilístico sobre vecinos. Si contagia -> Crear `CONTAGION`, encolar en `Queue` y añadir vecino a infectados.

**Checklist:**
- [x] `void simulationCallback(void *data, void *param)`: Callback para el recorrido DFS, permite calcular todo lo relevante en el mapa
- [x] `int stepSimulation(int day)`: Gestor del dia simulado para calculos

## 4. Minimización del Riesgo Total
**Explicación:**
Aislar nodos claves (Cuarentena) usando un enfoque Greedy.
* **Estrategia:** Calcular riesgo individual y usar un **MaxHeap** temporal para seleccionar los nodos más peligrosos según el presupuesto.

**Checklist:**
- [x] `int releaseRecovered(CITY *c)`: Criterio de orden en la cuarentena segun la recuperacion.
- [x] `int applySmartQuarantine(CITY *c, int budget)`: Mantener en cuarentena a las personas de mayor riesgo global.

## 5. Identificación de Rutas Críticas (Dijkstra)
**Explicación:**
Encontrar el camino de mayor probabilidad de infección entre dos personas.
* **Estrategia:** Dijkstra modificado. El peso de la arista se transforma a $Costo = -\log(Probabilidad)$ para maximizar la probabilidad acumulada.

**Checklist:**
- [x] `LIST *findCriticalPath(NODE *start, NODE *target)`: Implementación de Dijkstra usando `D_STATE` y `heap.h`.

## 6. Rutas Óptimas de Contención (Prim)
**Explicación:**
Generar una red mínima para conectar o vacunar focos de infección.
* **Estrategia:** Algoritmo de Prim (MST) donde el costo es la conexión directa o efectividad de la arista.

**Checklist:**
- [ ] `GRAPH *generateVaccinationPlan(GRAPH *people)`: Construcción del árbol de expansión mínima.

## 7. Clustering de Cepas
**Explicación:**
Agrupar variantes del virus por similitud (nombre).
* **Estrategia:** Uso de **Hash Table** con encadenamiento. La clave es `virus->name`.

**Checklist:**
- [ ] `void clusterViruses(LIST *allViruses)`: Agrupamiento y reporte de colisiones en la tabla Hash.

## 8. Almacenamiento y Consulta (La "BD")
**Explicación:**
Demostrar acceso eficiente $O(1)$ a datos e historial.
* **Estrategia:** Uso anidado de Hashes (`Ciudad` -> `Persona`) y acceso directo a la `Queue` de historial.

**Checklist:**
- [x] `int printTable(PERSON **arr, int n, char *title)`: Ordena los datos.
- [x] `int reportPerson(char *cityName, char *personName)`: busca los datos de la personas especificas.
- [x] `int reportCity(char *cityName)`: busca los habitantes de la ciudad.
