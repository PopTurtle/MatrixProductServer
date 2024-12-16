#include "communication.h"

#define REQUEST_SIZE (4 * sizeof(int) + sizeof(pid_t))

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

request *request_empty() {
    request *r = malloc(sizeof *r);
    if (r == NULL) {
        return NULL;
    }
    return r;
}

request *request_from(pid_t pid, int m, int n, int p, int sup) {
    request *r = malloc(sizeof *r);
    if (r == NULL) {
        return NULL;
    }
    r->pid = pid;
    r->m = m;
    r->n = n;
    r->p = p;
    r->sup = sup;
    return r;
}

void request_dispose(request **r) {
    free(*r);
    *r = NULL;
}

pid_t request_pid(const request *r) {
    return r->pid;
}

void request_mat_size_a(const request *r, int s[2]) {
    s[0] = r->m;
    s[1] = r->n;
}

void request_mat_size_b(const request *r, int s[2]) {
    s[0] = r->n;
    s[1] = r->p;
}

int request_sup(const request *r) {
    return r->sup;
}

void response_pipe_name(char *buff, pid_t pid) {
    sprintf(buff, RESPONSE_PIPE_PREFIX "%ld", (long) pid);
}

static char *__mk_request(const request *r) {
    char *rs = malloc(REQUEST_SIZE);
    if (r == NULL) {
        return NULL;
    }
    * (int *) rs = r->m;
    * (int *) (rs + sizeof(int)) = r->n;
    * (int *) (rs + sizeof(int) * 2) = r->p;
    * (int *) (rs + sizeof(int) * 3) = r->sup;
    * (pid_t *) (rs + sizeof(int) * 4) = r->pid;
    return rs;
}

static void __rd_request(request *r_out, const void *r_str) {
    r_out->m = *((int *) r_str);
    r_out->n = *((int *) r_str + 1);
    r_out->p = *((int *) r_str + 2);
    r_out->sup = *((int *) r_str + 3);
    r_out->pid = *((pid_t *) ((int *) r_str + 4));
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
    __rd_request(r_out, (const void *) r_str);
    free(r_str);
    return 0;
}