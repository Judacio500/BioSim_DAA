#include "graph.h"

// Printer Section

/*
    As the name states
    this section is dedicated to functions which's only purpose
    is to reproduce our data structures in mass
*/

GRAPH *createGraph(char *name, void *metadata)
{
    GRAPH *newG = (GRAPH*)malloc(sizeof(GRAPH));     // Basic function to initialize our graph
    if(!newG)
        return NULL;
    newG->name = strdup(name);
    newG->currentNode = NULL;
    newG->hashTable = initHash(0);                   // Function from the hash library
    newG->metadata = metadata;
    return newG;
}

NODE *createNode(GRAPH *graph, char *key, void *data)
{
    NODE *newN = (NODE*)malloc(sizeof(NODE));
    if (!newN) 
        return NULL;
    if(key)
        newN->nodeKey = strdup(key);
    else
        newN->nodeKey = NULL;
    newN->data = data;
    newN->firstEdge = NULL;
    newN->graph = graph;
    newN->parent = NULL;
    newN->cost = 0;
    return newN;
}


EDGE *createEdge(float weight, NODE *destinationNode)
{
    EDGE *newE = (EDGE*)malloc(sizeof(EDGE));
    if(!newE)
        return NULL;
    newE->weight = weight;
    newE->destinationNode = destinationNode;
    newE->nextNode = NULL;
    return newE;
}

// Insertion and Relationship Section

/*
    I guess i don't need to explain this but...

    Section dedicated to functions that work for the insertion of new nodes in the graph
    and it's relationships with other nodes and other auxiliar functions working for the same purpose
*/

int insertEdge(EDGE **nodeList, float weight, NODE *destinationNode)
{
    EDGE *newE = createEdge(weight, destinationNode);
    if(!newE)
        return -1;
    newE->nextNode = *nodeList;
    *nodeList = newE;
    return 0;
}

NODE *addNode(GRAPH *graph, char *key, void *data)
{
    NODE *newNode = createNode(graph, key, data);              // Create the new node with it's key and the wrapped data
    
    if(!newNode)    // Safety check
        return NULL;     

    saveKey(&(graph->hashTable), key, (void*)newNode);     // Pair the key and the node in the hash table

    if(graph->currentNode == NULL)
        graph->currentNode = newNode;                   // If the graph is empty or the last operation left us with a null value 
                                                        // substitue the current node with the one we are adding

    return newNode;
}

int addEdge(GRAPH *graph, char *key_1, char *key_2, float weight, int op)
{
    EHASH *element_1 = hashing(graph->hashTable, key_1);
    EHASH *element_2 = hashing(graph->hashTable, key_2);

    if(!element_1 || !element_2)
        return -1;
    NODE *relate_1 = (NODE*)element_1->pair;
    NODE *relate_2 = (NODE*)element_2->pair;

    switch(op)
    {
        // Directed graph
        case 1:
            insertEdge(&relate_1->firstEdge, weight, relate_2); // Create the relationship just between node_1 -> node_2 with no way back
            break;                                         

        // Undirected graph
        case 2:
            insertEdge(&relate_1->firstEdge, weight, relate_2);
            insertEdge(&relate_2->firstEdge, weight, relate_1);    // Both nodes involved in the relationship get a path towards eachother
            break;
    }

    return 0;
}

int addEdgeThrough(NODE *node_1, NODE *node_2, float weight, int op)
{
    if(!node_1 || !node_2)
        return -1;
    switch(op)
    {
        case 1:
            insertEdge(&node_1->firstEdge, weight, node_2);
            break;
        case 2:
            insertEdge(&node_1->firstEdge, weight, node_2);
            insertEdge(&node_2->firstEdge, weight, node_1);
    }
    return 0;
}

// Search Section

/*
    Graph algorithms which create paths between the nodes through
    the adjacency list such as dijkstra and A* or even bruteforce

    Useful when we got a weighted graph and we want to simulate or 
    solve for real life problems in which we cannot hash
    our way to a different node
*/

/*

A* orders the "to_visit" list in a scale of how competent is each node to find
the solution

This means two thing:

1.-
We Must use BFS since we have to trace the beginning of all paths to value their feasibility
if we use DFS we will first take whichever path we come across next, the necessity to "scout"
all the childs of the interest node in order to decide which path we are taking next is one
of the key strenghts of BFS

2.-
We must use a certain kind of "evaluation function" which will tell us how feasible a node is 
in contrast with other nodes and our GOAL this evaluation function will be taken as a callback
since each 

The evaluation function has to evaluate if the node is feasible globally not locally which means that it
also has to track the overall route cost 

*/

NODE *hashNode(GRAPH *graph,char *key)
{
    EHASH *element = hashing(graph->hashTable, key);
    if(!element)
        return NULL;
    NODE *found = (NODE*)element->pair;
    return found;
}

int traverseGraph(LIST **recurrenceList, NODE *current, Action act)
{
    if(!current)
    {
        return 0;
    }

    if(isInList(*recurrenceList, current->nodeKey))
    {
        return 0;
    }

    insertList(recurrenceList, current,0);
    if(act)
    {
        act(current);
    }

    EDGE *adjL = current->firstEdge;
    while(adjL)
    {
        traverseGraph(recurrenceList, adjL->destinationNode, act);
        adjL = adjL->nextNode;
    }

    return 0;
}

int traverseGraphWParameter(LIST **recurrenceList, NODE *current, void *param, ActionP act)
{
    if(!current)
    {
        return 0;
    }

    if(isInList(*recurrenceList, current->nodeKey))
    {
        return 0;
    }

    insertList(recurrenceList, current,0);
    if(act)
    {
        act(current, param);
    }

    EDGE *adjL = current->firstEdge;
    while(adjL)
    {
        traverseGraphWParameter(recurrenceList, adjL->destinationNode, param, act);
        adjL = adjL->nextNode;
    }

    return 0;
}

int isInList(LIST *rList, char *key)
{
    NODE *element;
    while(rList)
    {
        element = (NODE*)rList->data;
        if(!strcmp(element->nodeKey,key))
            return 1;
        rList = rList->next;
    }
    return 0;
}

// Experimental Section

/*
    This section will be commented most of the time

    i haven't even decided what to put here
    i just don't want to keep coding the insertion function so 
    i'm procrastinating

    
*/
