#ifndef COMMUNICATION__H
#define COMMUNICATION__H

#define SERVER_PIPE_NAME "fifo_server_mp_89716355745231"
#define RESPONSE_PIPE_PREFIX "response_mp_376517187_"

#define RESPONSE_PIPE_NAME_BUFF_SIZE 64

#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>


// Structure utilisee pour les requetes
typedef struct request request;

// Renvoie une requete non initialisee, NULL en cas d'erreur d'allocation
extern request *request_empty();

// Renvoie une requete initialisee, NULL en cas d'erreur d'allocation
// Les matrices sont de tailles (m * n) et (n * p)
extern request *request_from(pid_t pid, int m, int n, int p, int sup);

// Desalloue les ressources associees a la requete
extern void request_dispose(request **r);

// Recuperation des donnees des requetes
extern pid_t request_pid(const request *r);
extern void request_mat_size_a(const request *r, int s[2]);
extern void request_mat_size_b(const request *r, int s[2]);
extern int request_sup(const request *r);

// Fabrique le nom d'un tube de reponse a partir d'un pid
// buff doit avoir une longueure minimum de RESPONSE_PIPE_BUFF_SIZE
extern void response_pipe_name(char *buff, pid_t pid);


// Envoie une requete sur le tube de descripteur r_fd,
// renvoie -1 en cas d'erreur
extern int send_request(int r_fd, const request *r);

// Bloque le processus jusqu'à ce qu'une requete soit correctement lue.
// Renvoie -1 en cas d'erreur
extern int listen_request(int r_fd, request *r_out);

#endif