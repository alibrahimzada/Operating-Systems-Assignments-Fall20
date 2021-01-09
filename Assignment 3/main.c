# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>

// struct for publishers
typedef struct {
	int publisher_type; // publisher type number
	pthread_t *thread_t; // thread struct
} publisher_t;

publisher_t *publisher_thread;
pthread_t *packager_thread;


void *publisherThreadFun(publisher_t *thread) {
	printf("Thread %lu have been created for publisher type %d\n", pthread_self(), thread->publisher_type);
}

void *packagerThreadFun(pthread_t *thread) {
	printf("Thread %lu have been created as packager\n", pthread_self());
}

int main(int argc, char *argv[]) {
	int n_publisher_type, n_publisher_thread, n_packager_thread,
		n_books, packaging_size, buffer_size, i, c, j, k;

	if (argc != 10) {
		fprintf(stderr, "error :insufficient number of command line arguments\n");
		return 1;
	}

	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-n") == 0) {
			n_publisher_type = atoi(argv[++i]);
			n_publisher_thread = atoi(argv[++i]);
			n_packager_thread = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-b") == 0) {
			n_books = atoi(argv[++i]);
		} else if (strcmp(argv[i], "-s") == 0) {
			packaging_size = atoi(argv[++i]);
			buffer_size = atoi(argv[++i]);
		}
	}
	
	// multiplying publisher types and threads once to find total # of publisher threads
	int total_publisher_thread = n_publisher_type * n_publisher_thread;
	publisher_thread = malloc(total_publisher_thread * sizeof(publisher_t));
	packager_thread = malloc(n_packager_thread * sizeof(pthread_t));

	// creating publisher threads
	for (i=0; i < total_publisher_thread; i++) {
		(publisher_thread+i)->publisher_type = (i / n_publisher_thread) + 1;
		(publisher_thread+i)->thread_t = malloc(sizeof(pthread_t));
		pthread_create((publisher_thread+i)->thread_t, NULL, (void *) publisherThreadFun, (void *)(publisher_thread+i));
	}
	// creating packager threads
	for (j=0; j < n_packager_thread; j++) {
		pthread_create((packager_thread+j), NULL, (void *) packagerThreadFun, (void *)(packager_thread+j));
	}
	
	// joining them
	for (c=0; c < total_publisher_thread; c++) {
		pthread_join(*((publisher_thread+c)->thread_t),NULL);
	}
	for (k=0; k < n_packager_thread; k++) {
		pthread_join(*(packager_thread+k), NULL);
	}

	free(publisher_thread);
	free(packager_thread);
	return 0;
}
