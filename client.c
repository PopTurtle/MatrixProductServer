#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#include "communication.h"
#include "matrix.h"

#ifndef CLIENT_TIMEOUT
#define CLIENT_TIMEOUT 5
#endif

#define EPERROR(funstr)                                                        \
    perror(funstr);                                                            \
    exit(EXIT_FAILURE);


#define UPERROR(funstr, pipe_name)                                             \
    perror(funstr);                                                            \
    unlink(pipe_name);                                                         \
    exit(EXIT_FAILURE);


// Action a executer lorsque le client se termine. C'est à dire
// à la fin de l'execution ou quand SIGALRM est recu (timeout).
// Le parametre n'a aucun effet sur l'execution.
void on_client_close(int);


// Lis une valeur entiere depuis une chaine de caractere avec atoi.
// Renvoie -1 en cas d'erreur et associe la valeur de atoi a res
int get_arg(char *text, int *res);


// Les parametres d'appels, formulés [nom = default_value],  sont les suivants:
// ./client [m = 2] [n = 2] [p = 2] [sup = 9] [client_name]
// Si client_name est passé (char *), le client affiche le resultat de sa
// requete sur la sortie standard avec le préfixe client_name
int main(int argc, char *argv[]) {
    // Gestion des parametres
    int m, n, p, sup;
    bool print_result;
    char *name;

    if (argc < 2 || get_arg(argv[1], &m) == -1) { m = 2; }
    if (argc < 3 || get_arg(argv[2], &n) == -1) { n = 2; }
    if (argc < 4 || get_arg(argv[3], &p) == -1) { p = 2; }
    if (argc < 5 || get_arg(argv[4], &sup) == -1) { sup = 9; }

    if (argc >= 6) {
        name = argv[5];
        print_result = true;
    } else {
        name = NULL;
        print_result = false;
    }

    // Recupere le pid du processus courant
    pid_t pid = getpid();

    // Creer le tube de reponse
    char response_pipe_n[RESPONSE_PIPE_NAME_BUFF_SIZE];
    response_pipe_name(response_pipe_n, pid);
    
    if (mkfifo(response_pipe_n, S_IRUSR | S_IWUSR) == -1) {
        EPERROR("mkfifo");
    }

    // Ajoute une action de terminaison du client
    struct sigaction t_action;
    t_action.sa_handler = on_client_close;
    t_action.sa_flags = 0;
    if (sigfillset(&t_action.sa_mask) == -1) {
        EPERROR("sigemptyset");
    }

    if (sigaction(SIGTERM, &t_action, NULL) == -1) {
        EPERROR("sigaction");
    }

    if (sigaction(SIGINT, &t_action, NULL) == -1) {
        EPERROR("sigaction");
    }

    if (sigaction(SIGALRM, &t_action, NULL) == -1) {
        EPERROR("sigaction");
    }

    // Ouvre le tube de reponse (puis le ferme en écriture)
    int fd;
    pid_t pid_writer = fork();
    switch (pid_writer) {
        case -1:
            UPERROR("fork", response_pipe_n);
        case 0:
            if (open(response_pipe_n, O_WRONLY) == -1) {
                EPERROR("open");
            }
            exit(EXIT_SUCCESS);
        default:
            fd = open(response_pipe_n, O_RDONLY);
            if (fd == -1) {
                UPERROR("open", response_pipe_n);
            }
    }

    if (waitpid(pid_writer, NULL, 0) == -1) {
        UPERROR("waitpid", response_pipe_n);
    }

    // Ouvre le tube des requetes
    int request_fd = open(SERVER_PIPE_NAME, O_WRONLY);
    if (request_fd == -1) {
        UPERROR("open", response_pipe_n);
    }

    // Prepare la requete
    request *r = request_from(pid, m, n, p , sup);

    // Envoie la requete
    send_request(request_fd, r);

    // Attends une reponse sur le tube
    size_t buf_size = MAT_PROD_SIZE(m, n, p);
    char *rbuff = malloc(buf_size);
    if (rbuff == NULL) {
        UPERROR("malloc", response_pipe_n);
    }

    int *mat_a = ((int *) rbuff);
    int *mat_b = ((int *) rbuff + m * n);
    int *mat_c = ((int *) rbuff + m * n + n * p);

    // Initialise le timeout
    alarm(CLIENT_TIMEOUT);
    if (receive_response(fd, rbuff, buf_size) == -1) {
        fprintf(stderr, "Error: Could not receive properly the server response");
    }

    // Affiche les différentes matrices (si demande)
    if (print_result) {
        printf("Resultat client \"%s\"\n", name);
        print_matrix("Matrice A:", mat_a, m, n);
        print_matrix("Matrice B:", mat_b, n, p);
        print_matrix("Matrice C:", mat_c, m, p);
    }

    // Unlink le tube de reponse afin de le supprimer lors de la terminaison
    on_client_close(0);

    return EXIT_SUCCESS;
}

int get_arg(char *text, int *res) {
    *res = atoi(text);
    return *res - 1;
}

void on_client_close([[maybe_unused]] int) {
    // Recupere le nom du tube de reponse
    char response_pipe_n[RESPONSE_PIPE_NAME_BUFF_SIZE];
    response_pipe_name(response_pipe_n, getpid());

    // Unlink le tube
    if (unlink(response_pipe_n) == -1) {
        EPERROR("unlink");
    }

    exit(EXIT_FAILURE);
}