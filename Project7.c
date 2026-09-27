#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FACE_SIZE 32
#define MAX_PIXELS 262144   /* enough for images up to about 512 x 512 */
#define DATABASE_FILE "faces.db"
#define UNKNOWN_THRESHOLD 38.0

typedef struct {
    int width;
    int height;
    int maxValue;
    int *pixels;
} Image;

void freeImage(Image *img) {
    if (img->pixels != NULL) {
        free(img->pixels);
        img->pixels = NULL;
    }
}

/* Read the next non-comment token from a P2 PGM file. */
int readToken(FILE *fp, char *token, int size) {
    int c;
    int i = 0;

    while ((c = fgetc(fp)) != EOF) {
        if (c == '#') {
            while ((c = fgetc(fp)) != '\n' && c != EOF) {
                ;
            }
        } else if (c != ' ' && c != '\n' && c != '\t' && c != '\r') {
            break;
        }
    }

    if (c == EOF) {
        return 0;
    }

    do {
        if (i < size - 1) {
            token[i++] = (char)c;
        }
        c = fgetc(fp);
    } while (c != EOF && c != ' ' && c != '\n' && c != '\t' && c != '\r' && c != '#');

    if (c == '#') {
        while ((c = fgetc(fp)) != '\n' && c != EOF) {
            ;
        }
    }

    token[i] = '\0';
    return 1;
}

int loadPGM(const char *filename, Image *img) {
    FILE *fp = fopen(filename, "r");
    char token[100];
    int total;
    int i;

    img->pixels = NULL;

    if (fp == NULL) {
        printf("Could not open image file: %s\n", filename);
        return 0;
    }

    if (!readToken(fp, token, sizeof(token)) || strcmp(token, "P2") != 0) {
        printf("This program needs an ASCII PGM (P2) image.\n");
        fclose(fp);
        return 0;
    }

    if (!readToken(fp, token, sizeof(token))) {
        fclose(fp);
        return 0;
    }
    img->width = atoi(token);

    if (!readToken(fp, token, sizeof(token))) {
        fclose(fp);
        return 0;
    }
    img->height = atoi(token);

    if (!readToken(fp, token, sizeof(token))) {
        fclose(fp);
        return 0;
    }
    img->maxValue = atoi(token);

    total = img->width * img->height;

    if (img->width <= 0 || img->height <= 0 || total > MAX_PIXELS || img->maxValue <= 0) {
        printf("Invalid or too-large image.\n");
        fclose(fp);
        return 0;
    }

    img->pixels = (int *)malloc(total * sizeof(int));
    if (img->pixels == NULL) {
        printf("Not enough memory.\n");
        fclose(fp);
        return 0;
    }

    for (i = 0; i < total; i++) {
        if (!readToken(fp, token, sizeof(token))) {
            printf("Image ended before all pixels were read.\n");
            freeImage(img);
            fclose(fp);
            return 0;
        }
        img->pixels[i] = atoi(token);
    }

    fclose(fp);
    return 1;
}

/* Convert any input size to 32 x 32 using nearest-neighbour resizing. */
void resizeToFace(const Image *img, int face[FACE_SIZE][FACE_SIZE]) {
    int r, c;

    for (r = 0; r < FACE_SIZE; r++) {
        for (c = 0; c < FACE_SIZE; c++) {
            int srcRow = r * img->height / FACE_SIZE;
            int srcCol = c * img->width / FACE_SIZE;
            int value = img->pixels[srcRow * img->width + srcCol];

            /* Convert the image's original maximum value to the 0..255 range. */
            value = value * 255 / img->maxValue;
            face[r][c] = value;
        }
    }
}

/* Stretch brightness so the darkest pixel becomes 0 and brightest becomes 255. */
void normalizeFace(int face[FACE_SIZE][FACE_SIZE]) {
    int r, c;
    int minValue = 255;
    int maxValue = 0;

    for (r = 0; r < FACE_SIZE; r++) {
        for (c = 0; c < FACE_SIZE; c++) {
            if (face[r][c] < minValue) minValue = face[r][c];
            if (face[r][c] > maxValue) maxValue = face[r][c];
        }
    }

    if (maxValue == minValue) {
        return;
    }

    for (r = 0; r < FACE_SIZE; r++) {
        for (c = 0; c < FACE_SIZE; c++) {
            face[r][c] = (face[r][c] - minValue) * 255 / (maxValue - minValue);
        }
    }
}

int prepareFace(const char *filename, int face[FACE_SIZE][FACE_SIZE]) {
    Image img;

    if (!loadPGM(filename, &img)) {
        return 0;
    }

    resizeToFace(&img, face);
    normalizeFace(face);
    freeImage(&img);
    return 1;
}

void registerFace(void) {
    char name[50];
    char filename[200];
    int face[FACE_SIZE][FACE_SIZE];
    FILE *fp;
    int r, c;

    printf("\nEnter a name (one word): ");
    scanf("%49s", name);

    printf("Enter the PGM face-image filename: ");
    scanf("%199s", filename);

    if (!prepareFace(filename, face)) {
        printf("Registration failed.\n");
        return;
    }

    fp = fopen(DATABASE_FILE, "a");
    if (fp == NULL) {
        printf("Could not open the face database.\n");
        return;
    }

    fprintf(fp, "%s\n", name);
    for (r = 0; r < FACE_SIZE; r++) {
        for (c = 0; c < FACE_SIZE; c++) {
            fprintf(fp, "%d ", face[r][c]);
        }
    }
    fprintf(fp, "\n");

    fclose(fp);
    printf("Face registered successfully for %s.\n", name);
}

/* Mean Absolute Difference: smaller score means the images are more similar. */
double compareFaces(int a[FACE_SIZE][FACE_SIZE], int b[FACE_SIZE][FACE_SIZE]) {
    long totalDifference = 0;
    int r, c;

    for (r = 0; r < FACE_SIZE; r++) {
        for (c = 0; c < FACE_SIZE; c++) {
            totalDifference += abs(a[r][c] - b[r][c]);
        }
    }

    return (double)totalDifference / (FACE_SIZE * FACE_SIZE);
}

void recognizeFace(void) {
    char filename[200];
    int testFace[FACE_SIZE][FACE_SIZE];
    int savedFace[FACE_SIZE][FACE_SIZE];
    char savedName[50];
    char bestName[50] = "Unknown";
    double bestDifference = 1000000.0;
    double difference;
    double similarity;
    FILE *fp;
    int r, c;
    int count = 0;

    printf("\nEnter the PGM image to recognize: ");
    scanf("%199s", filename);

    if (!prepareFace(filename, testFace)) {
        printf("Could not prepare the test face.\n");
        return;
    }
    fp = fopen(DATABASE_FILE, "r");
    if (fp == NULL) {
        printf("No face database found. Register at least one face first.\n");
        return;
    }

    while (fscanf(fp, "%49s", savedName) == 1) {
        int ok = 1;

        for (r = 0; r < FACE_SIZE && ok; r++) {
            for (c = 0; c < FACE_SIZE; c++) {
                if (fscanf(fp, "%d", &savedFace[r][c]) != 1) {
                    ok = 0;
                    break;
                }
            }
        }

        if (!ok) {
            break;
        }

        difference = compareFaces(testFace, savedFace);
        printf("Compared with %-15s : difference = %.2f\n", savedName, difference);

        if (difference < bestDifference) {
            bestDifference = difference;
            strcpy(bestName, savedName);
        }
        count++;
    }

    fclose(fp);

    if (count == 0) {
        printf("The database is empty.\n");
        return;
    }

    similarity = 100.0 * (1.0 - bestDifference / 255.0);
    if (similarity < 0.0) similarity = 0.0;

    printf("\nBest comparison: %s\n", bestName);
    printf("Average pixel difference: %.2f\n", bestDifference);
    printf("Simple similarity score: %.1f%%\n", similarity);

    if (bestDifference <= UNKNOWN_THRESHOLD) {
        printf("RESULT: Face recognized as %s\n", bestName);
    } else {
        printf("RESULT: Unknown face\n");
    }
}

void listFaces(void) {
    FILE *fp = fopen(DATABASE_FILE, "r");
    char name[50];
    int value;
    int i;
    int count = 0;

    if (fp == NULL) {
        printf("No face database found.\n");
        return;
    }

    printf("\nRegistered people:\n");
    while (fscanf(fp, "%49s", name) == 1) {
        printf("  %d. %s\n", ++count, name);

        for (i = 0; i < FACE_SIZE * FACE_SIZE; i++) {
            if (fscanf(fp, "%d", &value) != 1) {
                fclose(fp);
                return;
            }
        }
    }

    if (count == 0) {
        printf("  No faces registered yet.\n");
    }

    fclose(fp);
}

void clearDatabase(void) {
    char answer;

    printf("Are you sure you want to delete all registered faces? (y/n): ");
    scanf(" %c", &answer);

    if (answer == 'y' || answer == 'Y') {
        if (remove(DATABASE_FILE) == 0) {
            printf("Face database deleted.\n");
        } else {
            printf("Database was already empty or could not be deleted.\n");
        }
    } else {
        printf("Nothing was deleted.\n");
    }
}

int main(void) {
    int choice;

    do {
        printf("\n========================================\n");
        printf("   BEGINNER FACE RECOGNITION IN C\n");
        printf("========================================\n");
        printf("1. Register a face\n");
        printf("2. Recognize a face\n");
        printf("3. List registered people\n");
        printf("4. Clear face database\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input.\n");
            return 0;
        }

        switch (choice) {
            case 1:
                registerFace();
                break;
            case 2:
                recognizeFace();
                break;
            case 3:
                listFaces();
                break;
            case 4:
                clearDatabase();
                break;
            case 5:
                printf("Program ended.\n");
                break;
            default:
                printf("Please choose a number from 1 to 5.\n");
        }
    } while (choice != 5);

    return 0;
}
