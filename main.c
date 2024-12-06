#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <mman.h>

#define PIPE_NAME "fifo_server_mp"

typedef struct {
	// PID du client
	pid_t pid;

	// Taille des matrices (an == bm)
	int am;
	int an;
	int bm;
	int bn;

	// Borne superieure des elements
	int sup;
} request;

void eperror(const char *s) {
	perror(s);
	exit(EXIT_FAILURE);
}

void listen(request *r) {
	r->am = 3;
	r->an = 4;
	r->bm = 4;
	r->bn = 5;
}

void worker_a(request r) {
	// Calcule la taille du segment de memoire partager puis le creer
	// Les matrices sont les pointeurs de tableaux a, b et c
	size_t mmap_size = (r.am * r.an + r.bm * r.bn + r.an * r.bm) * sizeof(int)
	int *a = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	if (a == MAP_FAILED) {
		eperror("mmap");
	}
	int *b = a + (r.am * r.an) * sizeof(int);
	int *c = b + (r.bm * r.bn) * sizeof(int);
}

int main(void) {
	
	// Créer le tube du serveur
	if (mkfifo(PIPE_NAME, S_IRUSR | S_IWUSR) == -1) {
		eperror("mkfifo");
	}

	// Ouvre le tube en lecture (écoute)
	int fd = open(PIPE_NAME, O_RDONLY);
	if (fd == -1) {
		eperror("open");
	}

	// Unlink le tube afin qu'il se supprime quand le serveur se ferme	
	if (unlink(PIPE_NAME) == -1) {
		eperror("unlink");
	}

	request current_request;
	while (1) {
		listen(&current_request);
		switch (fork()) {
			case -1:
				eperror("fork");
			case 0:
				worker_a(current_request);
				break;
		}
	}
	return EXIT_SUCCESS;
}
