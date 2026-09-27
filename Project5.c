#include <stdio.h>

#define MAX 20
#define INF 1000000000

int findMinDistance(int dist[], int visited[], int n)
{
    int min = INF;
    int minIndex = -1;

    for (int i = 0; i < n; i++)
    {
        if (visited[i] == 0 && dist[i] < min)
        {
            min = dist[i];
            minIndex = i;
        }
    }

    return minIndex;
}

void printPath(int parent[], int destination)
{
    int path[MAX];
    int length = 0;
    int current = destination;

    while (current != -1)
    {
        path[length] = current;
        length++;
        current = parent[current];
    }

    for (int i = length - 1; i >= 0; i--)
    {
        printf("%c", 'A' + path[i]);
        if (i != 0)
            printf(" -> ");
    }
}

void dijkstra(int graph[MAX][MAX], int n, int source)
{
    int dist[MAX];
    int visited[MAX];
    int parent[MAX];

    for (int i = 0; i < n; i++)
    {
        dist[i] = INF;
        visited[i] = 0;
        parent[i] = -1;
    }

    dist[source] = 0;

    for (int count = 0; count < n - 1; count++)
    {
        int u = findMinDistance(dist, visited, n);

        if (u == -1)
            break;

        visited[u] = 1;

        for (int v = 0; v < n; v++)
        {
            if (visited[v] == 0 &&
                graph[u][v] > 0 &&
                dist[u] != INF &&
                dist[u] + graph[u][v] < dist[v])
            {
                dist[v] = dist[u] + graph[u][v];
                parent[v] = u;
            }
        }
    }

    printf("\nShortest routes from vertex %c:\n", 'A' + source);
    printf("--------------------------------------------------\n");
    printf("Destination\tDistance\tRoute\n");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < n; i++)
    {
        printf("%c\t\t", 'A' + i);

        if (dist[i] == INF)
        {
            printf("INF\t\tNo route\n");
        }
        else
        {
            printf("%d\t\t", dist[i]);
            printPath(parent, i);
            printf("\n");
        }
    }
}

int main(void)
{
    int graph[MAX][MAX];
    int n;
    int source;

    printf("SHORTEST ROUTE USING DIJKSTRA'S ALGORITHM\n");
    printf("=========================================\n");

    printf("Enter number of vertices (maximum %d): ", MAX);
    scanf("%d", &n);

    if (n <= 0 || n > MAX)
    {
        printf("Invalid number of vertices.\n");
        return 1;
    }

    printf("\nVertices are named A, B, C, ...\n");
    printf("Enter the adjacency matrix.\n");
    printf("Use 0 when there is no direct edge.\n\n");

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            scanf("%d", &graph[i][j]);

            if (graph[i][j] < 0)
            {
                printf("Negative edge weights are not allowed in Dijkstra's algorithm.\n");
                return 1;
            }
        }
    }

    printf("\nEnter source vertex number (A=0, B=1, C=2, ...): ");
    scanf("%d", &source);

    if (source < 0 || source >= n)
    {
        printf("Invalid source vertex.\n");
        return 1;
    }

    dijkstra(graph, n, source);

    return 0;
}
