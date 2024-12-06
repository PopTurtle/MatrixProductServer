#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stats.h>

#define SERVER_PIPE_NAME "fifo_server_mp"

#define RESPONSE_PIPE_NAME "response_mp"
#define RESPONSE_PIPE_NAME_BUFF_SIZE 32

void eperror(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}

int main(void) {
    // Recupere le pid du processus courant
    pid_t pid = getpid();

    // Creer le tube de reponse
    const char response_pipe_name[RESPONSE_PIPE_NAME_BUFF_SIZE];
    sprintf(response_pipe_name, RESPONSE_PIPE_NAME "_%ld", (long) pid);

    if (mkfifo(response_pipe_name, S_IRUSR | S_IWUSR) == -1) {
        eperror(mkfifo);
    }

    int response_fd = open(response_pipe_name, O_RDONLY);
    if (response_fd == -1) {
        perror("open");
        unlink(mkfifo);
        exit(EXIT_FAILURE);
    }

    if (unlink(response_pipe_name) == -1) {
        eperror("unlink");
    }

    // Ouvre le tube des requetes
    int request_fd = open(PIPE_NAME, O_WRONLY);
    if (request_fd == -1) {
        eperror("open");
    }

    // Prepare et envoie la requete
    // Taille des matrice (m * n) et (n * p)
    int m = 4;
    int n = 3;
    int p = 3;

    // Borne superieure des elements à generer
    int sup = 10;

    // ENVOIE LA REQUETE ICI
    
    return EXIT_SUCCESS;
}
