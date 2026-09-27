#include <stdio.h>

void bubbleSort(int a[], int n) {
    int i, j, temp;
    for(i = 0; i < n - 1; i++) {
        for(j = 0; j < n - 1 - i; j++) {
            if(a[j] > a[j + 1]) {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

void selectionSort(int a[], int n) {
    int i, j, minIndex, temp;
    for(i = 0; i < n - 1; i++) {
        minIndex = i;
        for(j = i + 1; j < n; j++) {
            if(a[j] < a[minIndex]) {
                minIndex = j;
            }
        }
        temp = a[i];
        a[i] = a[minIndex];
        a[minIndex] = temp;
    }
}

void insertionSort(int a[], int n) {
    int i, key, j;
    for(i = 1; i < n; i++) {
        key = a[i];
        j = i - 1;
        while(j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

void copyArray(int src[], int dest[], int n) {
    int i;
    for(i = 0; i < n; i++) {
        dest[i] = src[i];
    }
}

void printArray(int a[], int n) {
    int i;
    for(i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
}

int main() {
    int n, i;
    int original[50], b[50], s[50], in[50];
    printf("Enter number of elements: ");
    scanf("%d", &n);
    printf("Enter %d elements: ", n);
    for(i = 0; i < n; i++) {
        scanf("%d", &original[i]);
    }

    copyArray(original, b, n);
    copyArray(original, s, n);
    copyArray(original, in, n);

    bubbleSort(b, n);
    selectionSort(s, n);
    insertionSort(in, n);

    printf("Bubble Sort: ");
    printArray(b, n);
    printf("Selection Sort: ");
    printArray(s, n);
    printf("Insertion Sort: ");
    printArray(in, n);

    return 0;
}
