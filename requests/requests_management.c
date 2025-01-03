#include "requests_management.h"

#define SHM_SIZE(m, n, p) \
    (sizeof(int) + MAT_PROD_SIZE(m, n, p))

#define SHM_MEETING \
    ((volatile int *) shm)

#define SHM_MAT_A \
    ((volatile int *) (shm + sizeof(int)));

#define SHM_MAT_B \
    ((volatile int *) (shm + sizeof(int) + MAT_SIZE(dim_a[0], dim_a[1])))

#define SHM_MAT_C \
    ((volatile int *) (shm + sizeof(int) + MAT_SIZE(dim_a[0], dim_a[1]) + MAT_SIZE(dim_b[0], dim_b[1])))

#define STEP_START 0
#define STEP_MAT_GENERATED 1
#define STEP_PRODUCT_ENDED 2


static void worker_a(const request *r);
static void worker_b(const request *r, char *shm);

void manage_request(const request *r) {
    worker_a(r);
}

void worker_a(const request *r) {
    int dim_a[2];
    int dim_b[2];
    request_mat_size_a(r, dim_a);
    request_mat_size_b(r, dim_b);

    // Creer le segment de memoire partagee
    char *shm = mmap(NULL, SHM_SIZE(dim_a[0], dim_a[1], dim_b[1]), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shm == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    volatile int *meeting = SHM_MEETING;
    volatile int *mat_a = SHM_MAT_A;

    *meeting = STEP_START;

    // On se fork avec le worker b
    switch(fork()) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            worker_b(r, shm);
            exit(EXIT_SUCCESS);
        default:
            random_matrix((int *) mat_a, dim_a[0], dim_a[1], request_sup(r), getpid());
            *meeting = STEP_MAT_GENERATED;
    }

    // Attend que le calcul matriciel soit terminé
    while (*meeting != STEP_PRODUCT_ENDED) { ; }

    // Transmet le resultat
    if (send_response(r, (const int *) mat_a, (const int *) SHM_MAT_B, (const int *) SHM_MAT_C) == -1) {
        fprintf(stderr, "Impossible d'envoyer la reponse\n");
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}

void worker_b(const request *r, char *shm) {
    int dim_a[2];
    int dim_b[2];
    request_mat_size_a(r, dim_a);
    request_mat_size_b(r, dim_b);
    
    volatile int *mat_b = SHM_MAT_B;
    
    random_matrix((int *) mat_b, dim_b[0], dim_b[1], request_sup(r), getpid());
    
    // Attend que la matrice A soit generee avant de continuer
    volatile int *meeting = SHM_MEETING;
    while (*meeting != STEP_MAT_GENERATED) { ; }

    // Realise le produit des matrices
    volatile int *mat_a = SHM_MAT_A;
    volatile int *mat_c = SHM_MAT_C;
    multithreaded_matrix_product((int *) mat_c, (int *) mat_a, (int *) mat_b, dim_a[0], dim_a[1], dim_b[1]);
    *meeting = STEP_PRODUCT_ENDED;

    exit(EXIT_SUCCESS);
}
