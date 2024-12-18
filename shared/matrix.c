#include "matrix.h"

// tmp_data : shortcut de thread_mat_product_data
typedef struct {
    int i;
    int j;

    int m;
    int n;
    int p;

    int *res;
    int *mat_a;
    int *mat_b;
} tmp_data;

static int rand_inclusive(int min, int max) {
    double r = (((double) rand()) - 1) / RAND_MAX;
    r = (r * (max + 1 - min));
    return min + (int) r;
}

typedef void * (*runnable)(void *);

static void *thread_mat_product(tmp_data *d) {
    int sum = 0;
    int index_a = d->i * d->n;
    int index_b = d->j;

    for (int c = 0; c < d->n; ++c) {
        int va = *(d->mat_a + index_a);
        int vb = *(d->mat_b + index_b);
        sum += va * vb;

        index_a += 1;
        index_b += d->p;
    }

    *(d->res + d->i * d->p + d->j) = sum;
    return d;
}

void random_matrix(int *mat, int m, int n, int sup, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < m * n; ++i) {
        int value = rand_inclusive(0, sup);
        *(mat + i) = value;
    }
}

int multithreaded_matrix_product(int *res, int *mat_a, int *mat_b, int m, int n, int p) {
    int err = 0;

    // Créer m * p thread qui calcul chacun une case de la matrice res
    size_t thread_count = (size_t) (m * p);
    size_t thread_made = 0;
    pthread_t *threads = malloc(thread_count * sizeof *threads);
    if (threads == NULL) {
        perror("malloc");
        return -1;
    }

    int errnum;

    tmp_data pre_arg = {
        .i = 0,
        .j = 0,
        .m = m,
        .n = n,
        .p = p,
        .res = res,
        .mat_a = mat_a,
        .mat_b = mat_b,
    };
    
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            int index = i * n + j;
            tmp_data *arg = malloc(sizeof *arg);
            if (arg == NULL) {
                perror("malloc");
                return -1;
            }

            pre_arg.i = i;
            pre_arg.j = j;
            *arg = pre_arg;

            if ((errnum = pthread_create(&threads[index], NULL, (runnable) thread_mat_product, arg)) != 0) {
                fprintf(stderr, "pthread_create: %s\n", strerror(errnum));
                err = -1;
                goto join_threads;
            }

            thread_made += 1;
        }
    }

join_threads:
    // Rejoins tous les threads
    for (size_t n = 0; n < thread_made; ++n) {
        if ((errnum = pthread_join(threads[n], NULL)) != 0) {
            fprintf(stderr, "pthread_join: %s\n", strerror(errnum));
            err = -1;
        }
    }
    return err;
}

void print_matrix(const char* name, const int *mat, int m, int n) {
    printf("%s\n", name);
    for (int l = 0; l < m; ++l) {
        for (int r = 0; r < n; ++r) {
            printf("%d ", *(mat + n * l + r));
        }
        printf("\n");
    }
}