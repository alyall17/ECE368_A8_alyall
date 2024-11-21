#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Priority Queue Node
struct PQNode {
    int node;
    int step;
    int distance;
};

// Comparator for Priority Queue
int compare(const void* a, const void* b) {
    struct PQNode* n1 = (struct PQNode*)a;
    struct PQNode* n2 = (struct PQNode*)b;
    return n1->distance - n2->distance;
}

// Graph Representation
struct Edge {
    int to;
    int* weights; // Periodic weights
    struct Edge* next;
};

struct Graph {
    int vertices;
    int period;
    struct Edge** adjList; // Original adjacency list
};

// Create a new graph
struct Graph* createGraph(int vertices, int period) {
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    graph->vertices = vertices;
    graph->period = period;
    graph->adjList = (struct Edge**)malloc(vertices * sizeof(struct Edge*));
    for (int i = 0; i < vertices; i++) {
        graph->adjList[i] = NULL;
    }
    return graph;
}

// Add edge to the graph
void addEdge(struct Graph* graph, int from, int to, int* weights) {
    struct Edge* edge = (struct Edge*)malloc(sizeof(struct Edge));
    edge->to = to;
    edge->weights = weights;
    edge->next = graph->adjList[from];
    graph->adjList[from] = edge;
}

// Dijkstra's Algorithm
int dijkstra(struct Graph* graph, int source, int target, int* path, int* pathLength) {
    int V = graph->vertices;
    int P = graph->period;

    // Priority queue
    struct PQNode* pq = (struct PQNode*)malloc(V * P * sizeof(struct PQNode));
    int pqSize = 0;

    // Distance table
    int** distances = (int**)malloc(V * sizeof(int*));
    for (int i = 0; i < V; i++) {
        distances[i] = (int*)malloc(P * sizeof(int));
        for (int j = 0; j < P; j++) {
            distances[i][j] = INT_MAX;
        }
    }

    // Predecessor table to track the path
    int** predecessors = (int**)malloc(V * sizeof(int*));
    for (int i = 0; i < V; i++) {
        predecessors[i] = (int*)malloc(P * sizeof(int));
        for (int j = 0; j < P; j++) {
            predecessors[i][j] = -1;
        }
    }

    // Enqueue source
    distances[source][0] = 0;
    pq[pqSize++] = (struct PQNode){source, 0, 0};

    while (pqSize > 0) {
        struct PQNode current = pq[0];
        pq[0] = pq[--pqSize];
        qsort(pq, pqSize, sizeof(struct PQNode), compare);

        int u = current.node;
        int step = current.step;

        struct Edge* edge = graph->adjList[u];
        while (edge) {
            int v = edge->to;
            int nextStep = (step + 1) % P;
            int weight = edge->weights[step];
            int newDistance = distances[u][step] + weight;

            if (newDistance < distances[v][nextStep]) {
                distances[v][nextStep] = newDistance;
                predecessors[v][nextStep] = u;
                pq[pqSize++] = (struct PQNode){v, nextStep, newDistance};
                qsort(pq, pqSize, sizeof(struct PQNode), compare);
            }

            edge = edge->next;
        }
    }

    // Find the shortest distance to the target across all steps
    int minDistance = INT_MAX;
    int finalStep = -1;
    for (int i = 0; i < P; i++) {
        if (distances[target][i] < minDistance) {
            minDistance = distances[target][i];
            finalStep = i;
        }
    }

    // Reconstruct the path
    if (minDistance != INT_MAX) {
        int currentNode = target;
        int currentStep = finalStep;
        *pathLength = 0;

        while (currentNode != source || currentStep != 0) {
            path[(*pathLength)++] = currentNode;
            int prevNode = predecessors[currentNode][currentStep];
            currentStep = (currentStep - 1 + P) % P;
            currentNode = prevNode;
        }
        path[(*pathLength)++] = source;
    }

    // Free memory
    for (int i = 0; i < V; i++) {
        free(distances[i]);
        free(predecessors[i]);
    }
    free(distances);
    free(predecessors);
    free(pq);

    return minDistance;
}

// Free the graph
void freeGraph(struct Graph* graph) {
    for (int i = 0; i < graph->vertices; i++) {
        struct Edge* edge = graph->adjList[i];
        while (edge) {
            struct Edge* temp = edge;
            edge = edge->next;
            free(temp->weights);
            free(temp);
        }
    }
    free(graph->adjList);
    free(graph);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <graph_file>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    int V, P;
    fscanf(file, "%d %d", &V, &P);

    struct Graph* graph = createGraph(V, P);

    int from, to;
    while (fscanf(file, "%d %d", &from, &to) == 2) {
        int* weights = (int*)malloc(P * sizeof(int));
        for (int i = 0; i < P; i++) {
            fscanf(file, "%d", &weights[i]);
        }
        addEdge(graph, from, to, weights);
    }
    fclose(file);

    int* path = (int*)malloc(V * P * sizeof(int));
    int pathLength = 0;
    int lastSource = -1;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        int source, target;
        if (sscanf(buffer, "%d %d", &source, &target) != 2) break;

        if (source != lastSource) {
            pathLength = 0;
            //printf("Recomputing paths from source %d\n", source);
            lastSource = source;
        }

        int distance = dijkstra(graph, source, target, path, &pathLength);
        if (distance == INT_MAX) {
            printf("No path found\n");
        } else {
            //printf("Shortest distance: %d\n", distance);
            //printf("Shortest path: ");
            for (int i = pathLength - 1; i >= 0; i--) {
                printf("%d ", path[i]);
            }
            printf("\n");
        }
    }

    free(path);
    freeGraph(graph);
    return 0;
}
