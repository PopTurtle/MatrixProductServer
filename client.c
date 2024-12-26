#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "communication.h"
#include "matrix.h"

#define EPERROR(funstr)                                                        \
    perror(funstr);                                                            \
    exit(EXIT_FAILURE);


// Lis une valeur entiere depuis une chaine de caractere avec atoi.
// Renvoie -1 en cas d'erreur et associe la valeur de atoi a res
int get_arg(char *text, int *res) {
    *res = atoi(text);
    return *res - 1;
}


// Les parametres d'appels, formulés [nom = default_value],  sont les suivants:
// ./client [m = 2] [n = 2] [p = 2] [sup = 9] [client_name]
// Si client_name est passé (char *), le client affiche le resultat de sa
// requete sur la sortie standard avec le préfixe client_name
int main(int argc, char *argv[]) {
    // Gestion des parametres
    int m, n, p, sup;
    bool print_result;
    char *name;

    if (argc >= 2 && get_arg(argv[1], &m) == -1) { m = 2; }
    if (argc >= 3 && get_arg(argv[2], &n) == -1) { n = 2; }
    if (argc >= 4 && get_arg(argv[3], &p) == -1) { p = 2; }
    if (argc >= 5 && get_arg(argv[4], &sup) == -1) { sup = 9; }

    if (argc >= 6) {
        name = argv[5];
        print_result = true;
    } else {
        name = NULL;
        print_result = false;
    }

    printf("%d %d %d %d %s\n", m, n, p, sup, name);

    // Recupere le pid du processus courant
    pid_t pid = getpid();

    // Creer le tube de reponse
    char response_pipe_n[RESPONSE_PIPE_NAME_BUFF_SIZE];
    response_pipe_name(response_pipe_n, pid);
    
    if (mkfifo(response_pipe_n, S_IRUSR | S_IWUSR) == -1) {
        EPERROR("mkfifo");
    }

    // Ouvre le tube de reponse (puis le ferme en écriture)
    int fd;
    pid_t pid_writer = fork();
    switch (pid_writer) {
        case -1:
            EPERROR("fork");
            unlink(response_pipe_n);
            return EXIT_FAILURE;
        case 0:
            if (open(response_pipe_n, O_WRONLY) == -1) {
                EPERROR("open");
            }
            exit(EXIT_SUCCESS);
        default:
            fd = open(response_pipe_n, O_RDONLY);
            if (fd == -1) {
                EPERROR("open");
            }
    }

    if (waitpid(pid_writer, NULL, 0) == -1) {
        EPERROR("waitpid");
    }

    // Ouvre le tube des requetes
    int request_fd = open(SERVER_PIPE_NAME, O_WRONLY);
    if (request_fd == -1) {
        EPERROR("open");
    }

    // Prepare la requete
    request *r = request_from(pid, m, n, p , sup);

    // Envoie la requete
    send_request(request_fd, r);

    // Attends une reponse sur le tube
    size_t buf_size = MAT_PROD_SIZE(m, n, p);
    char *rbuff = malloc(buf_size);
    if (rbuff == NULL) {
        EPERROR("malloc");
    }

    int *mat_a = (int *) (rbuff);
    int *mat_b = (int *) (rbuff + m * n);
    int *mat_c = (int *) (rbuff + m * n + n * p);

    if (receive_response(fd, rbuff, buf_size) == -1) {
        fprintf(stderr, "Error: Could not receive properly the server response");
    }

    // Affiche les différentes matrices (si demande)
    if (print_result) {
        printf("Client: %s\n", name);
        print_matrix("Matrice A:", mat_a, m, n);
        print_matrix("Matrice B:", mat_b, n, p);
        print_matrix("Matrice C:", mat_c, m, p);
    }


    // TEMPORAIRE -----------------------------------------------
    if (unlink(response_pipe_n) == -1) {
        EPERROR("unlink");
    }

    return EXIT_SUCCESS;
}
