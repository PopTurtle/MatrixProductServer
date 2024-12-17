#include "matrix.h"

static int rand_inclusive(int min, int max) {
    double r = (((double) rand()) - 1) / RAND_MAX;
    r = (r * (max + 1 - min));
    return min + (int) r;
}

void random_matrix(int *mat, int m, int n, int sup, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < m * n; ++i) {
        int value = rand_inclusive(0, sup);
        *(mat + i) = value;
    }
}

void multithreaded_matrix_product(int *res, int *mat_a, int *mat_b) {
    
    
    
     /* Incorrecte : pas de tailles de matrices */ }

void print_matrix(const char* name, const int *mat, int m, int n) {
    printf("%s\n", name);
    for (int l = 0; l < m; ++l) {
        for (int r = 0; r < n; ++r) {
            printf("%d ", *(mat + n * l + r));
        }
        printf("\n");
    }
}