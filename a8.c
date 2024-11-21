#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Priority queue node
struct PQNode {
    int node; // Vertex ID
    int step; // Time step (mod period)
    int distance; // Current distance
};

// Overall graph structure
struct Graph {
    int vertices; // Number of vertices
    int period; // Period
    struct Edge** adjList; // Adjacency list
};

// Edge structure (graph edges)
struct Edge {
    int to; // Destination node
    int* weights; // Array of weights (size P)
    struct Edge* next; // Pointer to the next edge
};

// Priority queue (min heap) structure
struct MinHeap {
    struct PQNode* nodes; // Array of nodes
    int size; // Current size of the heap
    int capacity; // Maximum capacity of the heap
};

// Comparator for min heap (used for heapify)
int compare(const void* a, const void* b) {
    struct PQNode* n1 = (struct PQNode*)a;
    struct PQNode* n2 = (struct PQNode*)b;
    return n1->distance - n2->distance;
}

// Create periodic graph from vertices and period
struct Graph* createGraph(int vertices, int period) {
    struct Graph* graph = (struct Graph*)malloc(sizeof(struct Graph));
    graph->vertices = vertices;
    graph->period = period;
    graph->adjList = (struct Edge**)malloc(vertices * sizeof(struct Edge*));
    for (int i = 0; i < vertices; i++) {
        graph->adjList[i] = NULL; // Initialize each adjacency list as NULL
    }
    return graph;
}

// Add an edge to the graph with the periodic weights attached (modified from lecture slides)
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

// Create a min-heap with given initial capacity
struct MinHeap* createMinHeap(int capacity) {
    struct MinHeap* heap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
    heap->nodes = (struct PQNode*)malloc(capacity * sizeof(struct PQNode));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Swap two nodes in the heap
void swap(struct PQNode* a, struct PQNode* b) {
    struct PQNode temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify the heap from a given index (restore the heap property)
void heapify(struct MinHeap* heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    // If the left child is smaller than the current node, swap
    if (left < heap->size && heap->nodes[left].distance < heap->nodes[smallest].distance) {
        smallest = left;
    }

    // If right child is smaller than the smallest, swap
    if (right < heap->size && heap->nodes[right].distance < heap->nodes[smallest].distance) {
        smallest = right;
    }

    // If smallest is not the current node, swap and heapify again
    if (smallest != index) {
        swap(&heap->nodes[index], &heap->nodes[smallest]);
        heapify(heap, smallest);
    }
}

// Extract the minimum node from the heap
struct PQNode extractMin(struct MinHeap* heap) {
    // Return invalid node for empty heap
    if (heap->size == 0) {
        struct PQNode empty = {-1, -1, INT_MAX};
        return empty; // Return an invalid node when the heap is empty
    }

    // Get root (minimum), replace with last node and restore heap property
    struct PQNode root = heap->nodes[0];
    heap->nodes[0] = heap->nodes[heap->size - 1];
    heap->size--;
    heapify(heap, 0);

    return root;
}

// Insert a new node into the heap
void insert(struct MinHeap* heap, struct PQNode node) {
    if (heap->size == heap->capacity) {
        // Resize the heap if necessary (this won't happen in a small graph but added for safety)
        heap->capacity *= 2;
        heap->nodes = (struct PQNode*)realloc(heap->nodes, heap->capacity * sizeof(struct PQNode));
    }

    // Insert new node at the end
    int index = heap->size++;
    heap->nodes[index] = node;

    // Bubble up to restore heap property
    while (index > 0 && heap->nodes[index].distance < heap->nodes[(index - 1) / 2].distance) {
        swap(&heap->nodes[index], &heap->nodes[(index - 1) / 2]);
        index = (index - 1) / 2;
    }
}

// Dijkstra's algorithm using heap & priority queue (modified from lecture slides)
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

    // Create a min-heap (priority queue)
    struct MinHeap* heap = createMinHeap(V * P);
    
    // Set source distance to 0 at step 0
    dist[source][0] = 0;
    insert(heap, (struct PQNode){source, 0, 0});

    while (heap->size > 0) {
        // Extract the minimum distance node from the heap
        struct PQNode current = extractMin(heap);
        int u = current.node;
        int step = current.step;

        // Process neighbors of current node
        struct Edge* edge = graph->adjList[u];
        while (edge) {
            int v = edge->to; // Destination
            int nextStep = (step + 1) % P; // Next time step (mod period)
            int weight = edge->weights[step]; // Edge weight at current step
            int newDistance = dist[u][step] + weight; // Calculate new distance

            // Update distance and predecessor if shorter path is found
            if (newDistance < dist[v][nextStep]) {
                dist[v][nextStep] = newDistance;
                pred[v][nextStep] = u;
                insert(heap, (struct PQNode){v, nextStep, newDistance});
            }

            edge = edge->next; // Move to next edge in adjacency list
        }
    }

    // Clean up memory (free mallocs in this block)
    free(heap->nodes);
    free(heap);
}

// Reconstruct the shortest path from source to target
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

    // Reconstruct the path by following predecessors
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

    // Read edges from input file
    int from, to;
    while (fscanf(file, "%d %d", &from, &to) == 2) {
        int* weights = (int*)malloc(P * sizeof(int));
        for (int i = 0; i < P; i++) {
            fscanf(file, "%d", &weights[i]);
        }
        addEdge(graph, from, to, weights);
    }
    fclose(file);

    // Create distance and predecessor tables
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

        // Recompute shortest paths from source if changed
        if (source != lastSource) {
            lastSource = source;
            dijkstra(graph, source, dist, pred); // Recompute paths from new source
        }

        // Find and print shortest path from source to target
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

    // Clean-up/free memory
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
