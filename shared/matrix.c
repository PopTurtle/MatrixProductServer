#include "matrix.h"

void random_matrix(int *mat, int m, int n, int sup) {
    for (int i = 0; i < m * n; ++i) {
        int value = sup; // RANDOM VALUE HERE
        
        *(mat + i) = value;
    }
}

void multithreaded_matrix_product(int *res, int *mat_a, int *mat_b) {
    
    
    
     /* Incorrecte : pas de tailles de matrices */ }

void print_matrix(const char* name, const int *mat, int m, int n) {
    printf("%s\n", name);
    for (int l = 0; l < m; ++l) {
        for (int r = 0; r < n; ++r) {
            printf("%d ", *(mat + m * l + r));
        }
        printf("\n");
    }
}