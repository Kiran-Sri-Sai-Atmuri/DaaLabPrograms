#include <stdio.h> 
#include <stdlib.h> 
  
#define MAXV 100 
#define MAXE 200 
  
typedef struct { 
    int u, v, weight; 
} Edge; 
  
int parent[MAXV + 1], rank_value[MAXV + 1]; 
  
int find_set(int x) { 
    if (parent[x] != x) 
        parent[x] = find_set(parent[x]); /* Path compression */ 
    return parent[x]; 
} 
  
void unite(int a, int b) { 
    a = find_set(a); 
    b = find_set(b); 
    if (a == b) return; 
  
    if (rank_value[a] < rank_value[b]) { 
        parent[a] = b; 
    } else if (rank_value[a] > rank_value[b]) { 
        parent[b] = a; 
    } else { 
        parent[b] = a; 
        rank_value[a]++; 
    } 
} 
  
int compare_edges(const void *left, const void *right) { 
    const Edge *a = left, *b = right; 
    if (a->weight != b->weight) 
        return (a->weight > b->weight) - (a->weight < b->weight); 
    if (a->u != b->u) return (a->u > b->u) - (a->u < b->u); 
    return (a->v > b->v) - (a->v < b->v); 
} 
  
int main(void) { 
    Edge edges[MAXE], chosen[MAXV - 1]; 
    int vertices, edge_count, i, selected = 0; 
    long long total = 0; 
  
    printf("Enter number of vertices (2 to 100): "); 
    if (scanf("%d", &vertices) != 1 || 
        vertices < 2 || vertices > MAXV) { 
        printf("Invalid number of vertices.\n"); 
        return 1; 
    } 
    printf("Enter number of edges (0 to 200): "); 
    if (scanf("%d", &edge_count) != 1 || 
        edge_count < 0 || edge_count > MAXE) { 
        printf("Invalid number of edges.\n"); 
        return 1; 
    } 
    printf("Enter edges as u v weight:\n"); 
    for (i = 0; i < edge_count; i++) { 
        if (scanf("%d %d %d", &edges[i].u, 
                  &edges[i].v, &edges[i].weight) != 3 || 
            edges[i].u < 1 || edges[i].u > vertices || 
            edges[i].v < 1 || edges[i].v > vertices) { 
            printf("Invalid edge.\n"); 
            return 1; 
        } 
    } 
  
    for (i = 1; i <= vertices; i++) { 
        parent[i] = i; 
        rank_value[i] = 0; 
    } 
    qsort(edges, (size_t)edge_count, 
          sizeof(edges[0]), compare_edges); 
  
    for (i = 0; i < edge_count && selected < vertices - 1; i++) { 
        int u = edges[i].u, v = edges[i].v; 
        if (find_set(u) == find_set(v)) continue; 
        chosen[selected++] = edges[i]; 
        total += edges[i].weight; 
        unite(u, v); 
    } 
  
    if (selected != vertices - 1) { 
        printf("MST does not exist: graph is disconnected.\n"); 
        return 0; 
    } 
  
    printf("Edges in the MST:\n"); 
    for (i = 0; i < selected; i++) { 
        printf("%d - %d : %d\n", chosen[i].u, 
               chosen[i].v, chosen[i].weight); 
    } 
    printf("Total weight: %lld\n", total); 
    return 0; 
}