#ifndef MATRIX__H
#define MATRIX__H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

// Taille en mémoire d'une matrice de taille m * n
#define MAT_SIZE(m, n) (sizeof(int) * m * n)

// Taille en mémoire des 3 matrices du produit des matrices
// de tailles (m * n) et (n * p)
#define MAT_PROD_SIZE(m, n, p) \
    (sizeof(int) * (m * n + n * p + m * p))


// Genere une matrice de taille m * n a l'emplacement mat, en utilisant
// des valeurs entieres aleatoires dans l'interval [[0; sup]]. seed permet
// de donner la graine du generateur de valeurs.
extern void random_matrix(int *mat, int m, int n, int sup, unsigned int seed);

// Realise le produit matriciel de mat_a * mat_b et stocke le résultat dans
// res. Avec les matrices de tailles mat_a (m * n) et mat_b (n * p), la fonction
// fait appel a m * p thread pour realiser le calcul.
// Renvoie 0 en cas de succes, sinon peut afficher un message d'erreur
// et renvoie 1
extern int multithreaded_matrix_product(int *res, int *mat_a, int *mat_b, int m, int n, int p);

// Permet d'afficher une matrice de taille m * n dans la sortie
// standard. Affiche name juste au dessus de la matrice
extern void print_matrix(const char *name, const int *mat, int m, int n);

#endif