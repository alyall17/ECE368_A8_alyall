#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Graph Node in Adjacency List
struct graphNode{
    int label; // Destination node
    int* weights; // Array of periodic weights
    struct graphNode* next; // Pointer to the next node
};

// Overall graph structure
struct Graph{
    int vertexCount; // Number of vertices
    int periodLength; // Length of weight period
    struct graphNode** adj; // Adjacency list
    int* heapIndex; // Heap index array
};

// Node in the heap
struct heapNode{
    int label; // Node label
    int distance; // Distance from the source
    int predecessor; // Predecessor node in shortest path
    int step; // Step at which node is processed
};

// Create a graph from the data file
struct Graph* createGraph(int vertexCount, int periodLength){
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    graph->vertexCount = vertexCount;
    graph->periodLength = periodLength;
    graph->adj = (struct gnode**)calloc(vertexCount, sizeof(struct gnode*));
    graph->heapIndex = (int*)malloc(vertexCount * sizeof(int));
    return graph;
}

// Free memory from the graph
void freeGraph(struct Graph* graph){
    for(int i = 0; i < graph->vertexCount; i++){
        struct graphNode* current = graph->adj[i];
        while(current){
            struct graphNode* temp = current;
            current = current->next;
            free(temp->weights);
            free(temp);
        }
    }
    free(graph->adj);
    free(graph->heapIndex);
    free(graph);
}

// Add edge to the graph (modified function from lecture notes)
void addEdge(struct Graph* graph, int from, int to, int* weights){
    struct graphNode* new = (struct graphNode*)malloc(sizeof(struct graphNode));
    new->label = to;
    new->weights = weights;
    new->next = graph->adj[from];
    graph->adj[from] = new;
}

// Function to swap two heap nodes (for Dijkstra's algorithm)
void swap(struct heapNode* arr, int i, int j, int* heapIndex){
    struct heapNode temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;

    // Update heap index array to reflect new positions
    heapIndex[arr[i].label] = i;
    heapIndex[arr[j].label] = j;
}

//

int main(int argc, char *argv[]){

}