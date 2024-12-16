#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>

#include "communication.h"
#include "requests_management.h"

#define EPERROR(funstr)                                                        \
    perror(funstr);                                                            \
    exit(EXIT_FAILURE);

void on_sig_term(int);

int main(void) {
	
    // Prépare le recevage du signal de terminaisain
    //struct sigaction action;

	// Créer le tube du serveur
    if (mkfifo(SERVER_PIPE_NAME, S_IRUSR | S_IWUSR) == -1) {
        EPERROR("mkfifo");
    }

    // Ajoute une action tel que le fifo se unlink à la terminaison
    struct sigaction t_action;
    t_action.sa_handler = on_sig_term;
    t_action.sa_flags = 0;
    if (sigemptyset(&t_action.sa_mask) == -1) {
        EPERROR("sigemptyset");
    }

    if (sigaddset(&t_action.sa_mask, SIGTERM)) {
        EPERROR("sigaddset");
    }

    if (sigaction(SIGTERM, &t_action, NULL) == -1) {
        EPERROR("sigaction");
    }

    if (sigaction(SIGINT, &t_action, NULL) == -1) {
        EPERROR("sigaction");
    }

	// Ouvre le tube (puis le ferme en écriture)
    int fd;
    pid_t pid_writer = fork();
    switch (pid_writer) {
        case -1:
            perror("fork");
            unlink(SERVER_PIPE_NAME);
            return EXIT_FAILURE;
        case 0:
            if (open(SERVER_PIPE_NAME, O_WRONLY) == -1) {
                EPERROR("open");
            }
            exit(EXIT_SUCCESS);
        default:
            fd = open(SERVER_PIPE_NAME, O_RDONLY);
            if (fd == -1) {
                EPERROR("open");
            }
    }

    if (waitpid(pid_writer, NULL, 0) == -1) {
        EPERROR("waitpid");
    }
    
    request *current_request = request_empty();
	while (1) {
		listen_request(fd, current_request);
		switch (fork()) {
			case -1:
				EPERROR("fork");
				break;
			case 0:
				if (close(fd) == -1) {
					EPERROR("close");
				}
				manage_request(current_request);
				exit(EXIT_SUCCESS);
		}
	}
	return EXIT_SUCCESS;
}

void on_sig_term([[maybe_unused]] int) {
    if (unlink(SERVER_PIPE_NAME) == -1) {
        EPERROR("unlink");
    }
    exit(EXIT_SUCCESS);
}
