# 🗺️ Hoja de Ruta: BioSim

Plan de implementación para el Simulador Algorítmico de Epidemias.

## 1. Inicialización y Análisis de Datos (Los 4 Sorts)
**Explicación:**
El objetivo es cargar la "BD" desde un archivo de texto, instanciar las estructuras y demostrar 4 algoritmos de ordenamiento de complejidad $O(n \log n)$.
* **Estrategia:** No insertar directo al grafo. Leer archivo -> Llenar array `population` en Metadata -> Ejecutar Sorts sobre el array -> Poblar el grafo `city->people`.

**Checklist:**
- [ ] `MD *createMetadata()`: Malloc de estructura, inicializa listas a NULL y crea queue `contagionHistory`.
- [ ] `void loadData(char *filename, GRAPH *cities)`: Parsea nombres, busca/crea ciudad, crea `PERSON*` y calcula riesgo base.
- [ ] `void mergeSort(PERSON **arr, int low, int high)`: Algoritmo 1 (Estable).
- [ ] `void quickSort(PERSON **arr, int low, int high)`: Algoritmo 2 (Rápido).
- [ ] `void heapSort(PERSON **arr, int n)`: Algoritmo 3 (Usando librería `heap.h`).
- [ ] `void shellSort(PERSON **arr, int n)`: Algoritmo 4 (Requisito académico).
- [ ] `void populateGraphs(GRAPH *cities)`: Inserta nodos y aristas en el grafo desde el array ordenado.

## 2. Detección de Brotes
**Explicación:**
Identificar clusters de infección activa sin recorrer todo el grafo ineficientemente.
* **Estrategia:** Iterar únicamente la `infectedList` de la metadata para iniciar búsquedas locales (BFS) limitadas por profundidad o zona.

**Checklist:**
- [ ] `int detectOutbreak(GRAPH *peopleGraph)`: Itera infectados y retorna el tamaño del cluster conectado.

## 3. Propagación Temporal (Simulación)
**Explicación:**
Avanzar el tiempo $t \to t+1$, gestionando nuevos contagios y recuperaciones.
* **Estrategia:** Iterar solo `infectedList`.
    * **Cura:** Si `daysInfected >= recovery` -> `RECUPERADO`.
    * **Contagio:** Probabilístico sobre vecinos. Si contagia -> Crear `CONTAGION`, encolar en `Queue` y añadir vecino a infectados.

**Checklist:**
- [ ] `void stepSimulation(GRAPH *cities)`: Controla el ciclo de vida, contagios y registro en historial.

## 4. Minimización del Riesgo Total
**Explicación:**
Aislar nodos claves (Cuarentena) usando un enfoque Greedy.
* **Estrategia:** Calcular riesgo individual y usar un **MaxHeap** temporal para seleccionar los nodos más peligrosos según el presupuesto.

**Checklist:**
- [ ] `void calculateGlobalRisk(GRAPH *people)`: Cálculo lineal sobre el array `population`.
- [ ] `void applyQuarantine(GRAPH *people, int budget)`: Uso de Heap para seleccionar y aislar (flag `quarantine=true`).

## 5. Identificación de Rutas Críticas (Dijkstra)
**Explicación:**
Encontrar el camino de mayor probabilidad de infección entre dos personas.
* **Estrategia:** Dijkstra modificado. El peso de la arista se transforma a $Costo = -\log(Probabilidad)$ para maximizar la probabilidad acumulada.

**Checklist:**
- [ ] `LIST *findCriticalPath(NODE *start, NODE *target)`: Implementación de Dijkstra usando `D_STATE` y `heap.h`.

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
- [ ] `void querySystem(char *city, char *personName)`: Muestra datos de la persona y el primer/último contagio registrado en O(1).