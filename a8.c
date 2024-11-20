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

// Restore heap by shifting down (downward heapify)
void downwardHeapify(struct heapNode* arr, int n, int i, int* heapIndex){
    int smallest = i; // Assume current node is smallest
    int left = 2 * i + 1; // Left child index
    int right = 2 * i + 2; // Right child index

    // Check if left child exists and is smaller than current node
    if(left < n && (arr[left].distance < arr[smallest].distance)){
        smallest = left;
    }

    // Check if right child exists and is smaller than current node
    if(right < n && (arr[right].distance < arr[smallest].distance)){
        smallest = right;
    }

    // If the smallest node is not current node, swap and continue
    if(smallest != i){
        swap(arr, i, smallest, heapIndex);
        downwardHeapify(arr, n, smallest, heapIndex);
    }
}

// Restore heap by shifting up (upward heapify)
void upwardHeapify(struct heapNode* arr, int i, int* heapIndex){
    // Continue shifting up until property restored
    while(i > 0 && (arr[i].distance < arr[(i - 1) / 2].distance)){
        swap(arr, i, ((i - 1) / 2), heapIndex); // Swap with parent
        i = (i - 1) / 2; // Move up to parent's index
    }
}

// Dequeue minimum element from the heap
void dequeue(struct heapNode* arr, int n, int* heapIndex){
    if(n == 0) return;

    // Swap root (minimum) with last element in the heap
    swap(arr, 0, n, heapIndex);

    // Restore heap property by shifting down
    downwardHeapify(arr, n, 0, heapIndex);
}

// Relax an edge during Dijkstra's algorithm
void relaxEdge(struct Graph* graph, struct heapNode* arr, int u, struct graphNode* v, int step, int n){
    // Compute effective weight for edge at current step
    int effectiveWeight = v->weights[step % graph->periodLength];

    // Check if the distance through edge improves the shortest path to v->label
    if((graph->heapIndex[v->label] < n) && (arr[graph->heapIndex[v->label]].distance > (arr[graph->heapIndex[u]].distance + effectiveWeight))){
        
        // Update the distance and predecessor for the target vertex
        arr[graph->heapIndex[v->label]].distance = arr[graph->heapIndex[u]].distance + effectiveWeight;
        arr[graph->heapIndex[v->label]].predecessor = u;
        arr[graph->heapIndex[v->label]].step = step + 1;

        // Restore the heap property for the updated vertex
        upwardHeapify(arr, graph->heapIndex[v->label], graph->heapIndex);
    }
}

// Dijkstra's algorithm implementation (from lecture slides), modified for periodic weights
void dijkstra(struct Graph* graph, int source, int traget){
    // Initialize the heap (priority queue) with graph vertices
    struct heapNode* arr = (struct heapNode*)malloc(graph->vertexCount * sizeof(struct heapNode));
    int n = graph->vertexCount;

    // Initialize all heap nodes with infinite distance and no predecessors
    for(int i = 0; i < graph->vertexCount; i++){
        arr[i].label = i;
        arr[i].distance = INT_MAX;
        arr[i].predecessor = -1;
        arr[i].step = 0;
        graph->heapIndex[i] = i;
    }

    // Set source vertex's distance to 0
    arr[0].distance = 0;
    arr[0].label = source;
    graph->heapIndex[source] = 0;

    // Main Dijkstra's loop
    while(n != 0){
        dequeue(arr, n - 1, graph->heapIndex); // Extract minimum distance node
        n--;
        int u = arr[n].label; // Current node
        int step = arr[n].step; // Current step for calculating periodic weights
        struct graphNode* v = graph->adj[u]; // Traverse adjacency list of u

        // Relax all edges from u to neighbors
        while(v){
            relaxEdge(graph, arr, u, v, step, n);
            v = v->next;
        }
    }

    // Print shortest path and the total weight
}

int main(int argc, char *argv[]){

}