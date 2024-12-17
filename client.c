#include <stdlib.h>
#include <stdio.h>
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

int read_full_buff(char *buff, size_t size, int fd) {
    while (size > 0) {
        int count = read(fd, buff, size);
        if (count == -1) {
            return -1;
        }
        buff += count;
        size -= count;
    }
    return 0;
}

int main(void) {

    // Gestion des parametres
    int m = 2;
    int n = 3;
    int p = 2;
    int sup = 7;

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

    if (read_full_buff(rbuff, buf_size, fd) == -1) {
        EPERROR("read");
    }



    // 
    print_matrix("MAT A:", (const int *) rbuff, m, n);
    print_matrix("MAT B:", (const int *) (rbuff + MAT_SIZE(m, n)), n, p);
    print_matrix("MAT C:", (const int *) (rbuff + MAT_SIZE(m, n) + MAT_SIZE(n, p)), m, p);    


    // TEMPORAIRE -----------------------------------------------
    if (unlink(response_pipe_n) == -1) {
        EPERROR("unlink");
    }

    return EXIT_SUCCESS;
}
