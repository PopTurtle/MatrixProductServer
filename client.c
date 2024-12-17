#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "communication.h"

#define EPERROR(funstr)                                                        \
    perror(funstr);                                                            \
    exit(EXIT_FAILURE);

#include <unistd.h>
int main(void) {
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

    // TEMPORAIRE -----------------------------------------------
    if (unlink(response_pipe_n) == -1) {
        EPERROR("unlink");
    }
    
    // Ouvre le tube des requetes
    int request_fd = open(SERVER_PIPE_NAME, O_WRONLY);
    if (request_fd == -1) {
        EPERROR("open");
    }

    // Prepare la requete
    request *r = request_from(pid, 3, 4, 3, 10);

    // Envoie la requete
    send_request(request_fd, r);

    return EXIT_SUCCESS;
}
