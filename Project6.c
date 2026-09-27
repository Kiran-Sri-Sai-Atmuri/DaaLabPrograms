#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
 
#define MAX_TOKEN 256
 
typedef struct {
    int x;
    int y;
} Pixel;
 
typedef struct {
    int area;
    int minX, minY, maxX, maxY;
} Component;
 
/* Read next meaningful token from a PGM file, skipping comments. */
int readToken(FILE *fp, char *token, int maxLen) {
    int c;
    int i = 0;
 
    while ((c = fgetc(fp)) != EOF) {
        if (isspace(c)) {
            continue;
        }
        if (c == '#') {
            while ((c = fgetc(fp)) != EOF && c != '\n') {
                /* skip comment */
            }
            continue;
        }
        break;
    }
 
    if (c == EOF) {
        return 0;
    }
 
    do {
        if (i < maxLen - 1) {
            token[i++] = (char)c;
        }
        c = fgetc(fp);
    } while (c != EOF && !isspace(c) && c != '#');
 
    if (c == '#') {
        while ((c = fgetc(fp)) != EOF && c != '\n') {
            /* skip rest of comment */
        }
    }
 
    token[i] = '\0';
    return 1;
}
 
int readPGM(const char *filename, int **image, int *width, int *height, int *maxValue) {
    FILE *fp = fopen(filename, "r");
    char token[MAX_TOKEN];
    int total, i;
 
    if (fp == NULL) {
        printf("Error: Could not open %s\n", filename);
        return 0;
    }
 
    if (!readToken(fp, token, MAX_TOKEN) || strcmp(token, "P2") != 0) {
        printf("Error: Only ASCII PGM (P2) images are supported.\n");
        fclose(fp);
        return 0;
    }
    if (!readToken(fp, token, MAX_TOKEN)) { fclose(fp); return 0; }
    *width = atoi(token);
    if (!readToken(fp, token, MAX_TOKEN)) { fclose(fp); return 0; }
    *height = atoi(token);
    if (!readToken(fp, token, MAX_TOKEN)) { fclose(fp); return 0; }
    *maxValue = atoi(token);
 
    if (*width <= 0 || *height <= 0 || *maxValue <= 0) {
        printf("Error: Invalid PGM header.\n");
        fclose(fp);
        return 0;
    }
 
    total = (*width) * (*height);
    *image = (int *)malloc(total * sizeof(int));
    if (*image == NULL) {
        printf("Error: Not enough memory.\n");
        fclose(fp);
        return 0;
    }
 
    for (i = 0; i < total; i++) {
        if (!readToken(fp, token, MAX_TOKEN)) {
            printf("Error: Image ended before all pixels were read.\n");
            free(*image);
            *image = NULL;
            fclose(fp);
            return 0;
        }
        (*image)[i] = atoi(token);
    }
 
    fclose(fp);
    return 1;
}
 
int writePGM(const char *filename, const int *image, int width, int height, int maxValue) {
    FILE *fp = fopen(filename, "w");
    int x, y;
 
    if (fp == NULL) {
        printf("Error: Could not create %s\n", filename);
        return 0;
    }
 
    fprintf(fp, "P2\n%d %d\n%d\n", width, height, maxValue);
 
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            fprintf(fp, "%d ", image[y * width + x]);
        }
        fprintf(fp, "\n");
    }
 
    fclose(fp);
    return 1;
}
 
void thresholdImage(const int *image, unsigned char *binary, int total,
                    int threshold) {
    int i;
    for (i = 0; i < total; i++) {
        /* Dark pixel = possible person = 1 */
        binary[i] = (image[i] < threshold) ? 1 : 0;
    }
}
 
Component floodFill(const unsigned char *binary, unsigned char *visited,
                    int width, int height, int startX, int startY,
                    Pixel *queue) {
    int front = 0;
    int rear = 0;
Component c;
    int dx, dy;
 
    c.area = 0;
    c.minX = c.maxX = startX;
    c.minY = c.maxY = startY;
 
    queue[rear].x = startX;
    queue[rear].y = startY;
    rear++;
    visited[startY * width + startX] = 1;
 
    while (front < rear) {
        Pixel p = queue[front++];
        c.area++;
 
        if (p.x < c.minX) c.minX = p.x;
        if (p.x > c.maxX) c.maxX = p.x;
        if (p.y < c.minY) c.minY = p.y;
        if (p.y > c.maxY) c.maxY = p.y;
 
        /* 8-neighbour connectivity */
        for (dy = -1; dy <= 1; dy++) {
            for (dx = -1; dx <= 1; dx++) {
                int nx, ny, index;
                if (dx == 0 && dy == 0) {
                    continue;
                }
 
                nx = p.x + dx;
                ny = p.y + dy;
 
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    index = ny * width + nx;
                    if (binary[index] == 1 && visited[index] == 0) {
                        visited[index] = 1;
                        queue[rear].x = nx;
                        queue[rear].y = ny;
                        rear++;
                    }
                }
            }
        }
    }
 
    return c;
}
 
void drawRectangle(int *image, int width, int height,
                   int minX, int minY, int maxX, int maxY, int value) {
    int x, y;
 
    if (minX < 0) minX = 0;
    if (minY < 0) minY = 0;
    if (maxX >= width) maxX = width - 1;
    if (maxY >= height) maxY = height - 1;
 
    for (x = minX; x <= maxX; x++) {
        image[minY * width + x] = value;
        image[maxY * width + x] = value;
    }
    for (y = minY; y <= maxY; y++) {
        image[y * width + minX] = value;
        image[y * width + maxX] = value;
    }
}
 
int countPeople(const int *image, int width, int height,
                int threshold, int minArea,
                int *markedImage, Component *accepted, int maxAccepted) {
    int total = width * height;
    unsigned char *binary = (unsigned char *)calloc(total, sizeof(unsigned char));
    unsigned char *visited = (unsigned char *)calloc(total, sizeof(unsigned char));
    Pixel *queue = (Pixel *)malloc(total * sizeof(Pixel));
    int x, y;
    int count = 0;
 
    if (binary == NULL || visited == NULL || queue == NULL) {
        printf("Error: Not enough memory for processing.\n");
        free(binary);
        free(visited);
        free(queue);
        return -1;
    }
 
    thresholdImage(image, binary, total, threshold);
 
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            int index = y * width + x;
            if (binary[index] == 1 && visited[index] == 0) {
                Component c = floodFill(binary, visited, width, height, x, y, queue);
 
                if (c.area >= minArea) {
                    if (count < maxAccepted) {
                        accepted[count] = c;
                    }
                    count++;
                }
            }
        }
    }
 
    /* Copy original image and draw boxes around accepted components. */
    memcpy(markedImage, image, total * sizeof(int));
    for (x = 0; x < count && x < maxAccepted; x++) {
        drawRectangle(markedImage, width, height,
                      accepted[x].minX, accepted[x].minY,
                      accepted[x].maxX, accepted[x].maxY, 0);
    }
 
    free(binary);
    free(visited);
    free(queue);
    return count;
}
 
int main(void) {
    char filename[256];
    int *image = NULL;
    int *markedImage = NULL;
    Component *people = NULL;
    int width, height, maxValue;
    int threshold = 150;
    int minArea = 120;
    int count, i;
 
    printf("============================================\n");
    printf("     SIMPLE PEOPLE COUNTER USING C\n");
    printf("============================================\n");
    printf("This program works best with a light background\n");
    printf("and dark, separated human-like silhouettes.\n\n");
 
    printf("Enter PGM image filename: ");
    if (scanf("%255s", filename) != 1) {
        return 1;
    }
 
    if (!readPGM(filename, &image, &width, &height, &maxValue)) {
        return 1;
    }
 
    printf("\nImage loaded successfully.\n");
    printf("Width      : %d pixels\n", width);
    printf("Height     : %d pixels\n", height);
    printf("Max value  : %d\n", maxValue);
 
    printf("\nEnter threshold (recommended 150): ");
    scanf("%d", &threshold);
    printf("Enter minimum object area (recommended 120): ");
    scanf("%d", &minArea);
 
    markedImage = (int *)malloc(width * height * sizeof(int));
    people = (Component *)malloc(width * height * sizeof(Component));
 
    if (markedImage == NULL || people == NULL) {
        printf("Error: Not enough memory.\n");
        free(image);
        free(markedImage);
        free(people);
        return 1;
    }
 
    count = countPeople(image, width, height, threshold, minArea,
                        markedImage, people, width * height);
 
    if (count < 0) {
        free(image);
        free(markedImage);
        free(people);
        return 1;
    }
 
    printf("\n============================================\n");
    printf("RESULT: Estimated number of people = %d\n", count);
    printf("============================================\n");
 
    for (i = 0; i < count; i++) {
        printf("Person %d: area=%d pixels, box=(%d,%d) to (%d,%d)\n",
               i + 1,
               people[i].area,
               people[i].minX, people[i].minY,
               people[i].maxX, people[i].maxY);
    }
 
    if (writePGM("counted_output.pgm", markedImage, width, height, maxValue)) {
        printf("\nMarked image saved as counted_output.pgm\n");
    }
 
    printf("\nImportant: This is a beginner image-processing project,\n");
    printf("not a production AI people detector.\n");
 
    free(image);
    free(markedImage);
    free(people);
    return 0;
}
