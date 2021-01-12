# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
# include <semaphore.h>

// struct for publishers
typedef struct {
	int total_published_book;
	int publisher_type; // publisher type number
	int publisher_id;
	pthread_t *thread_t; // thread struct
} publisher_t;

typedef struct {
	int total_packaged_book;
	int packager_id;
	pthread_t *thread_t;
} packager_t;

publisher_t *publisher_thread;
packager_t *packager_thread;
sem_t *empty_semaphores; // counting sem should be initialized with buffer_size for each publisher
sem_t *full_semaphores; // counting sem should be initialized with 0 for each publisher
sem_t *packager_sem; // binary semaphore
int *publisher_buffers[];
int *packager_buffers[];

int n_publisher_type, n_publisher_thread, n_packager_thread,
		n_books, packaging_size, buffer_size;

void redInfo() { printf("\033[0;31m [INFO] \033[0;37m"); }
void greenInfo() { printf("\033[0;32m [INFO] \033[0;37m"); }
void yellowInfo() { printf("\033[0;33m [INFO] \033[0;37m"); }

void *publisherThreadFun(publisher_t *thread) {
	greenInfo();
	printf("Thread %d have been created for publisher type %d\n", thread->publisher_id, thread->publisher_type);
}

void *packagerThreadFun(packager_t *thread) {
	greenInfo();
	printf("Packager %d have been created\n", thread->packager_id);
}

void *publishBook(publisher_t *thread) {
	int sem_idx = (thread->publisher_type - 1);
	sem_t *current_emtpy_sem = (empty_semaphores + sem_idx);
	sem_t *current_full_sem = (full_semaphores + sem_idx);
	do{

	//publish a book
	sem_wait(current_emtpy_sem);
	sem_wait(mutex);

	//place in buffer

	sem_post(mutex);
	sem_post(current_full_sem);
	}while(1)
}

void *packageBook(packager_t *thread) {
	do{
	sem_wait(full);
	sem_wait(mutex);

	// remove book from buffer

	sem_post(mutex);
	sem_post(empty);
	// package a book

	}while(1)
}

void *doubleTheSize(packager_t *thread) {

}

int main(int argc, char *argv[]) {
	int i, c, j, k, l;

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
	packager_thread = malloc(n_packager_thread * sizeof(packager_t));

	// memory allocation for semaphores of each publication type
	empty_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	full_semaphores = malloc(n_publisher_type * sizeof(sem_t));

	// initializing semaphores
	for (l=0; l < n_publisher_type; l++){
		sem_init((empty_semaphores+l), 0, buffer_size);
		sem_init((full_semaphores+l), 0, 0);
	}

	// creating publisher threads
	for (i=0; i < total_publisher_thread; i++) {
		(publisher_thread+i)->publisher_type = (i / n_publisher_thread) + 1;
		(publisher_thread+i)->publisher_id = (i % n_publisher_thread) + 1;
		(publisher_thread+i)->total_published_book = 0;
		(publisher_thread+i)->thread_t = malloc(sizeof(pthread_t));
		pthread_create((publisher_thread+i)->thread_t, NULL, (void *) publisherThreadFun, (void *)(publisher_thread+i));
	}
	// creating packager threads
	for (j=0; j < n_packager_thread; j++) {
		(packager_thread+j)->packager_id = j+1;
		(packager_thread+j)->total_packaged_book = 0;
		(packager_thread+j)->thread_t = malloc(sizeof(pthread_t));
		pthread_create((packager_thread+j)->thread_t, NULL, (void *) packagerThreadFun, (void *)(packager_thread+j));
	}
	
	// joining them
	for (c=0; c < total_publisher_thread; c++) {
		pthread_join(*((publisher_thread+c)->thread_t),NULL);
	}
	for (k=0; k < n_packager_thread; k++) {
		pthread_join(*((packager_thread+k)->thread_t), NULL);
	}

	free(publisher_thread);
	free(packager_thread);
	return 0;
}
