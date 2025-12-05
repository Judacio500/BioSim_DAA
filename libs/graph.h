#ifndef GRAPH_H
#define GRAPH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"
#include "list.h"

// STRUCT DEFINITION

typedef void(*Action)(void *data);
typedef void(*ActionP)(void *data, void *param);

typedef struct node 
{
    char *nodeKey;                    
                                    // key variable this one can be substituted for anything, name it a float, char, string, even a struct
                                    // if used as a string a hash table can be implemented     
    void *data;               
                                    // We are using a void pointer to save our data so that our graphs can hold any data type
                                    // integers, lists, characters, or even other complex structures such as trees or even more graphs       
    struct edge *firstEdge;     
                                    // pointer to the beginning of the list that contains the edges to this node
                                    // works as a wrapper for the related nodes so that a weight can be assigned to each edge
    struct node *parent;
                                    // A very specific pointer to implement BFS, for Automaths
                                    // since a status can be reached through different paths
                                    // the instance of a node needs awareness of the path the Automath
                                    // followed in order to reach the desired status
                                    // it's important to point out that even if a certain status is found
                                    // twice it's technically not the same since the path followed
                                    // to achive that status is different
    float cost;                
                                    // Saves the global cost of the path taken towards that node after expansion; 
    struct graph *graph;
                                    // Pointer to the graph that contains this node
} NODE;

typedef struct edge
{
    float weight;                   // weight/cost/time whatever, this variable tells the program if this path is expensive or 
                                    // set to NULL if not needed, for all i care you could even plug a list in here and make your own format for weighting

    struct node *destinationNode;   // pointer to the destination node, if a graph is bidirected two relationships should be created for each node involved in the graph
                                    // this creates a lot of redundancy, but that's alright graphs do need some of ts
                                    
    struct edge *nextNode;
                                    // pointer to the next edge of the same node
                                    // NULL when no relationships, though a node in the graph should always be related to at least 1 node if
                                    // used as a bidirected graph
} EDGE;

typedef struct graph
{
    char *name;                     
                                    // Label to refer the graph
    struct node *currentNode;       
                                    // When the graph is created this node will be assigned to the first node inserted in the graph 
                                    // This node will hold it's last value at the end of any operations with the graph a new origin position may
                                    // be obtained through the hash table
    struct hash *hashTable;
                                    // The hash table
    struct list *grapHeads;
                                    // List that includes the keys to each subgraph inside the graph
                                    // We can define a subgraph as a number of nodes that share a connection but are not connected with
                                    // a different cluster of nodesw
    void *metadata;
} GRAPH;

// Function Prototypes
GRAPH *createGraph(char *name, void *metadata);
NODE *createNode(GRAPH *graph, char *key, void *data);
EDGE *createEdge(float weight, NODE *destinationNode);
int insertEdge(EDGE **nodeList, float weight, NODE *destinationNode);
NODE *addNode(GRAPH *graph, char *key, void *data);
int addEdge(GRAPH *graph, char *key_1, char *key_2, float weight, int op);
int addEdgeThrough(NODE *node_1, NODE *node_2, float weight, int op);
int isInList(LIST *rList, char *key);
int traverseGraph(LIST **recurrenceList, NODE *current, Action act);
int traverseGraphWParameter(LIST **recurrenceList, NODE *current, void *param, ActionP act);
NODE *hashNode(GRAPH *graph,char *key);

#endif
