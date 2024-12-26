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


int send_response(const request *r, const int *mat_a, const int *mat_b, const int *mat_c) {
    // Recupere les tailles de matrices
    int dim_a[2];
    int dim_b[2];
    int dim_c[2];
    request_mat_size_a(r, dim_a);
    request_mat_size_b(r, dim_b);
    dim_c[0] = dim_a[0];
    dim_c[1] = dim_b[1];

    // Alloue un buffer de la bonne taille pour les 3 matrices
    char *buff = malloc(MAT_PROD_SIZE(dim_a[0], dim_a[1], dim_b[1]));
    if (buff == NULL) {
        return -1;
    }

    // Ouvre le tube associer a la requete
    char pipe_name[RESPONSE_PIPE_NAME_BUFF_SIZE];
    response_pipe_name(pipe_name, request_pid(r));
    int response_fd = open(pipe_name, O_WRONLY);
    if (response_fd == -1) {
        free(buff);
        return -1;
    }

    // Ecris les matrices dans le buffer
    size_t mat_a_s = MAT_SIZE(dim_a[0], dim_a[1]);
    size_t mat_b_s = MAT_SIZE(dim_b[0], dim_b[1]);
    size_t mat_c_s = MAT_SIZE(dim_c[0], dim_c[1]);
    memcpy(buff, mat_a, mat_a_s);
    memcpy(buff + mat_a_s, mat_b, mat_b_s);
    memcpy(buff + mat_a_s + mat_b_s, mat_c, mat_c_s);

    // Ecris le buffer dans le tube
    if (write(response_fd, buff, mat_a_s + mat_b_s + mat_c_s) == -1) {
        free(buff);
        close(response_fd);
        return -1;
    }

    // Libere les ressources
    free(buff);
    close(response_fd);
    return 0;
}


int __read_full_buff(char *buff, size_t size, int fd) {
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


int receive_response(int fd, char *buff, size_t read_size) {
    while (read_size > 0) {
        int count = read(fd, buff, read_size);
        if (count == -1) {
            return -1;
        }
        buff += count;
        read_size -= count;
    }
    return 0;
}