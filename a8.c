#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Priority queue node
struct PQNode {
    int node;     // Vertex ID
    int step;     // Time step (mod P)
    int distance; // Current distance
};

// Edge structure
struct Edge {
    int to;               // Destination node
    int* weights;         // Array of weights (size P)
    struct Edge* next;    // Pointer to the next edge
};

// Graph structure
struct Graph {
    int vertices;         // Number of vertices
    int period;           // Period P
    struct Edge** adjList; // Adjacency list
};

// Comparator for priority queue
int compare(const void* a, const void* b) {
    struct PQNode* n1 = (struct PQNode*)a;
    struct PQNode* n2 = (struct PQNode*)b;
    return n1->distance - n2->distance;
}

// Create a graph
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

// Add an edge to the graph
void addEdge(struct Graph* graph, int from, int to, int* weights) {
    struct Edge* edge = (struct Edge*)malloc(sizeof(struct Edge));
    edge->to = to;
    edge->weights = weights;
    edge->next = graph->adjList[from];
    graph->adjList[from] = edge;
}

// Free graph memory
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

// Dijkstra's algorithm
void dijkstra(struct Graph* graph, int source, int** dist, int** pred) {
    int V = graph->vertices;
    int P = graph->period;

    // Initialize distances and predecessors
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < P; j++) {
            dist[i][j] = INT_MAX;
            pred[i][j] = -1;
        }
    }

    // Priority queue
    struct PQNode* pq = (struct PQNode*)malloc(V * P * sizeof(struct PQNode));
    int pqSize = 0;

    // Set source distance to 0 at step 0
    dist[source][0] = 0;
    pq[pqSize++] = (struct PQNode){source, 0, 0};

    while (pqSize > 0) {
        // Dequeue minimum element
        struct PQNode current = pq[0];
        pq[0] = pq[--pqSize];
        qsort(pq, pqSize, sizeof(struct PQNode), compare);

        int u = current.node;
        int step = current.step;

        // Process neighbors
        struct Edge* edge = graph->adjList[u];
        while (edge) {
            int v = edge->to;
            int nextStep = (step + 1) % P;
            int weight = edge->weights[step];
            int newDistance = dist[u][step] + weight;

            if (newDistance < dist[v][nextStep]) {
                dist[v][nextStep] = newDistance;
                pred[v][nextStep] = u;
                pq[pqSize++] = (struct PQNode){v, nextStep, newDistance};
                qsort(pq, pqSize, sizeof(struct PQNode), compare);
            }

            edge = edge->next;
        }
    }

    free(pq);
}

// Reconstruct the path from source to target
int findPath(int source, int target, int** pred, int** dist, int P, int* path, int* pathLength) {
    int minDistance = INT_MAX;
    int finalStep = -1;

    // Find the step with the shortest distance to the target
    for (int i = 0; i < P; i++) {
        if (dist[target][i] < minDistance) {
            minDistance = dist[target][i];
            finalStep = i;
        }
    }

    if (minDistance == INT_MAX) {
        // No path found
        *pathLength = 0;
        return INT_MAX;
    }

    // Reconstruct the path
    int currentNode = target;
    int currentStep = finalStep;
    *pathLength = 0;

    while (currentNode != source || currentStep != 0) {
        path[(*pathLength)++] = currentNode;
        int prevNode = pred[currentNode][currentStep];
        currentStep = (currentStep - 1 + P) % P; // Move to the previous step
        currentNode = prevNode;
    }

    path[(*pathLength)++] = source; // Add the source to the path
    return minDistance;
}

// Main function
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

    // Read edges
    int from, to;
    while (fscanf(file, "%d %d", &from, &to) == 2) {
        int* weights = (int*)malloc(P * sizeof(int));
        for (int i = 0; i < P; i++) {
            fscanf(file, "%d", &weights[i]);
        }
        addEdge(graph, from, to, weights);
    }
    fclose(file);

    // Distance and predecessor tables
    int** dist = (int**)malloc(V * sizeof(int*));
    int** pred = (int**)malloc(V * sizeof(int*));
    for (int i = 0; i < V; i++) {
        dist[i] = (int*)malloc(P * sizeof(int));
        pred[i] = (int*)malloc(P * sizeof(int));
    }

    int* path = (int*)malloc(V * P * sizeof(int));
    int pathLength = 0;
    int lastSource = -1;

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        int source, target;
        if (sscanf(buffer, "%d %d", &source, &target) != 2) break;

        if (source != lastSource) {
            lastSource = source;
            dijkstra(graph, source, dist, pred); // Recompute paths from new source
        }

        int distance = findPath(source, target, pred, dist, P, path, &pathLength);
        if (distance == INT_MAX) {
            printf("No path found\n");
        } else {
            for (int i = pathLength - 1; i >= 0; i--) {
                printf("%d ", path[i]);
            }
            printf("\n");
        }
    }

    // Free memory
    for (int i = 0; i < V; i++) {
        free(dist[i]);
        free(pred[i]);
    }
    free(dist);
    free(pred);
    free(path);
    freeGraph(graph);
    return 0;
}
