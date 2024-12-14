#include <fcntl.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "communication.h"

#define REQUEST_SIZE (4 * sizeof(int) + sizeof(pid_t))

/*
struct request {
	// PID du client
	pid_t pid;

	// Taille des matrices (m * n et n * p)
	int m;
	int n;
	int p;

	// Borne superieure des elements
	int sup;
};*/

static char *__mk_request(const request *r) {
    char *rs = malloc(REQUEST_SIZE);
    if (r == NULL) {
        return NULL;
    }
    *rs = r->m;
    *(rs + sizeof(int)) = r->n;
    *(rs + sizeof(int) * 2) = r->p;
    *(rs + sizeof(int) * 3) = r->sup;
    *(rs + sizeof(int) * 4) = r->pid;
    return rs;
}

static void __rd_request(request *r_out, const char *r_str) {
    r_out->m = *(r_str);
    r_out->n = *(r_str + sizeof(int));
    r_out->p = *(r_str + sizeof(int) * 2);
    r_out->sup = *(r_str + sizeof(int) * 3);
    r_out->pid = *(r_str + sizeof(int) * 4);
}

int send_request(int r_fd, const request *r) {
    // Génère la requete
    char *rs = __mk_request(r);
    if (rs == NULL) {
        return -1;
    }

    // Tente d'envoyer la requete
    if (write(r_fd, rs, REQUEST_SIZE) == -1) {
        free(rs);
        return -1;
    }

    free(rs);
    return 0;
}

int listen_request(int r_fd, request *r_out) {
    char *r_str = malloc(REQUEST_SIZE);
    if (r_str == NULL) {
        return -1;
    }

    // Tente de lire une requete
	ssize_t n;
	while ((n = read(r_fd, r_str, REQUEST_SIZE)) != REQUEST_SIZE) {
		if (n == -1) {
            free(r_str);
			return -1;
		}
	}

    // Extrait les données
    __rd_request(r_out, r_str);
    free(r_str);
    return 0;
}