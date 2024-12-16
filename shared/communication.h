#ifndef COMMUNICATION__H
#define COMMUNICATION__H

#define SERVER_PIPE_NAME "fifo_server_mp_89716355745231"
#define RESPONSE_PIPE_PREFIX "response_mp_376517187_"

#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

// Structure utilisée pour les requetes
typedef struct request request;



struct request { // ???????????????????????????????????????????????????????
	// PID du client
	pid_t pid;

	// Taille des matrices (m * n et n * p)
	int m;
	int n;
	int p;

	// Borne superieure des elements
	int sup;
};

// Envoie une requete sur le tube de descripteur r_fd,
// renvoie -1 en cas d'erreur
extern int send_request(int r_fd, const request *r);

// Bloque le processus jusqu'à ce qu'une requete soit correctement lue.
// Renvoie -1 en cas d'erreur
extern int listen_request(int r_fd, request *r_out);

#endif