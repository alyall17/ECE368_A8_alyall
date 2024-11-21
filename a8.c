#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Graph Node in Adjacency List
struct graphNode{
    int label; // Destination node
    int* weight; // Dynamically allocated weights
    struct graphNode* next; // Pointer to the next node
};

// Node in Dijkstra heap
struct treeNode{
    int label; // Node label
    int distance; // Distance from the source
    int predecessor; // Predecessor node in shortest path
    int step; // Number of steps taken to reach this node
};

struct Graph{
    struct graphNode ** graph; // Adjacency List
    int* heapIndex;
    int vertices; // Number of vertices
    int period; // Period of weights
};

// Create a graph from the data file
struct Graph* createGraph(int vertices, int period){
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    graph->vertices = vertices;
    graph->period = period;
    graph->graph = (struct graphNode**)malloc(vertices * sizeof(struct graphNode*));
    graph->heapIndex = (int*)malloc(vertices * sizeof(int));
    for(int i = 0; i < vertices; i++){
        graph->graph[i] = NULL;
    }
    return graph;
}

// Free memory from the graph
void freeGraph(struct Graph* graph){
    for(int i = 0; i < graph->vertices; i++){
        struct graphNode* current = graph->graph[i];
        while(current){
            struct graphNode* temp = current;
            current = current->next;
            free(temp->weight);
            free(temp);
        }
    }
    free(graph->graph);
    free(graph->heapIndex);
    free(graph);
}

// Add edge to the graph (modified function from lecture notes)
void addEdge(struct Graph* graph, int from, int to, int* weights){
    struct graphNode* new = (struct graphNode*)malloc(sizeof(struct graphNode));
    new->label = to;
    new->weight = (int*)malloc(graph->period * sizeof(int));
    for(int i = 0; i < graph->period; i++){
        new->weight[i] = weights[i];
    }
    new->next = graph->graph[from];
    graph->graph[from] = new;
}

// Dequeue minimum element from the heap (modified from lecture slides)
void dequeue(struct treeNode* arr, int n, int* heapIndex){
    struct treeNode temp = arr[n];
    arr[n] = arr[0];
    arr[0] = temp;

    heapIndex[arr[0].label] = 0;
    heapIndex[arr[n].label] = n;

    n--;
    int i = 0;
    int j;
    while((j = 2 * i + 1) <= n){
        if(j < n && arr[j].distance > arr[j + 1].distance) j++;
        if(arr[j].distance >= temp.distance) break;

        arr[i] = arr[j];
        heapIndex[arr[i].label] = i;
        i = j;
    }

    arr[i] = temp;
    heapIndex[arr[i].label] = i;
}

// Update location in the heap
void update(struct treeNode* arr, int i, int* heapIndex){
    struct treeNode temp = arr[i];
    int j = i;
    while(j > 0 && arr[(j - 1) / 2].distance > temp.distance){
        arr[j]= arr[(j - 1) / 2];
        heapIndex[arr[j].label] = j;
        j = (j - 1) / 2;
    }
    arr[j] = temp;
    heapIndex[arr[j].label] = j;
}

// Update the shortest path and propagate steps backward
/*void updateSteps(struct treeNode* arr, int* heapIndex, int current, int newStep) {
    while (current != -1) {
        arr[heapIndex[current]].step = newStep;
        newStep--;  // Step decrements as we move backward in the path
        current = arr[heapIndex[current]].predecessor;
    }
}*/

void recalcSteps(struct treeNode* arr, int* heapIndex, struct Graph* graph, int source, int current) {
    int step = 0; // Start at step 0 for the source
    int node = current;

    // Temporarily store the nodes in the updated path
    int* pathNodes = (int*)malloc(sizeof(int) * heapIndex[source]);
    int pathCount = 0;

    // Trace back to the source to construct the full path
    while (node != -1) {
        pathNodes[pathCount++] = node;
        node = arr[heapIndex[node]].predecessor;
    }

    // Assign steps to each node in the path
    int cumulativeDistance = 0;
    for (int i = pathCount - 1; i >= 0; i--) {
        int currentNode = pathNodes[i];
        arr[heapIndex[pathNodes[i]]].step = step++;

        if (i < pathCount - 1) {
            // Get the predecessor and calculate distance using the periodic weight
            int predecessor = pathNodes[i + 1];
            int edgeWeightIndex = arr[heapIndex[currentNode]].step % graph->period;

            // Find the edge weight from the predecessor to the current node
            struct graphNode* edge = graph->graph[predecessor];
            while (edge != NULL && edge->label != currentNode) {
                edge = edge->next;
            }

            if (edge != NULL) {
                cumulativeDistance += edge->weight[edgeWeightIndex];
            } else {
                fprintf(stderr, "Error: Edge from %d to %d not found\n", predecessor, currentNode);
                free(pathNodes);
                return;
            }
        }

        // Update the node's distance
        arr[heapIndex[currentNode]].distance = cumulativeDistance;
    }

    free(pathNodes);
}

// Dijkstra's algorithm implementation (from lecture slides), modified for periodic weights
void dijkstra(struct Graph* graph, int source, int target){
    int V = graph->vertices;
    int period = graph->period;

    struct treeNode* arr = (struct treeNode*)malloc(V * sizeof(struct treeNode));
    int n = V;

    // Initialize nodes
    for(int i = 0; i < V; i++){
        arr[i].label = i;
        arr[i].distance = INT_MAX;
        arr[i].predecessor = -1;
        arr[i].step = 0; // Start with 0 steps
        graph->heapIndex[i] = i;
    }

    arr[source].distance = 0;
    arr[source].step = 0; // Starting node, no steps taken yet

    while(n > 0){
        dequeue(arr, n - 1, graph->heapIndex);
        n--;
        int u = arr[n].label;

        if(u == target) break; // Early termination if target is reached

        struct graphNode* v = graph->graph[u];
        while(v != NULL){
            int nextStep = arr[n].step + 1;
            int nextWeight = (nextStep % period);
            if(graph->heapIndex[v->label] < n && arr[graph->heapIndex[v->label]].distance > arr[n].distance + v->weight[nextWeight]){
                arr[graph->heapIndex[v->label]].distance = arr[n].distance + v->weight[nextWeight];
                arr[graph->heapIndex[v->label]].predecessor = u;
                //arr[graph->heapIndex[v->label]].step = nextStep; // Update step in graph traversal
                //updateSteps(arr, graph->heapIndex, v->label, arr[n].step + 1);

                // Recalculate steps for all nodes in the updated path
                recalcSteps(arr, graph->heapIndex, graph, source, v->label);
                update(arr, graph->heapIndex[v->label], graph->heapIndex);
            }
            v = v->next;
        }
    }

    // Print the shortest path
    int* path = (int *)malloc(V * sizeof(int));
    int count = 0;
    int current = target;
    while(current != -1){
        path[count++] = current;
        current = arr[graph->heapIndex[current]].predecessor;
    }

    if(arr[graph->heapIndex[target]].distance == INT_MAX){
        printf("No path found\n");
    }
    else{
        for(int i = count - 1; i >= 0; i--){
            printf("%d ", path[i]);
        }
        printf("\n");
    }
    free(path);
    free(arr);
}

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if(!file){
        perror("Error opening file");
        return 1;
    }

    int V;
    int N;
    fscanf(file, "%d %d", &V, &N);

    // Initialize overall graph
    struct Graph* graph = createGraph(V, N);

    // Input edges
    int from;
    int to;
    int* weights = (int*)malloc(N * sizeof(int));
    while(fscanf(file, "%d %d", &from, &to) == 2){
        for(int i = 0; i < N; i++){
            fscanf(file, "%d", &weights[i]);
        }
        addEdge(graph, from, to, weights);
    }
    free(weights);
    fclose(file);

    char input[256];
    int source;
    int target;
    while(fgets(input, sizeof(input), stdin) != NULL){
        if(sscanf(input, "%d %d", &source, &target) == 2){
            dijkstra(graph, source, target);
        }
        else{
            break;
        }
    }

    freeGraph(graph);
    return 0;
}
