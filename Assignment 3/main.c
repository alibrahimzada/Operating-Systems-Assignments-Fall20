# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
# include <semaphore.h>

// struct for publishers
typedef struct {
	int published_book;
	int publisher_type; // publisher type number
	int publisher_id;
	pthread_t *thread_t; // thread struct
} publisher_t;

typedef struct {
	int packaged_book;
	int packager_id;
	pthread_t *thread_t;
} packager_t;

publisher_t *publisher_thread;
packager_t *packager_thread;
sem_t *empty_semaphores; // counting sem should be initialized with buffer_size for each publisher
sem_t *full_semaphores; // counting sem should be initialized with 0 for each publisher
pthread_mutex_t *publisher_mutex; 
pthread_mutex_t packager_mutex;
int *total_published_book, total_packaged_book = 0;
// sem_t *packager_sem; // binary semaphore
char ***publisher_buffers; // triple pointer keeps that buffer_of_publisher_type->list_of_book_names->book_name (char *)
char **packager_buffers;

int n_publisher_type, n_publisher_thread, n_packager_thread,
		n_books, packaging_size, buffer_size;

void redInfo() { printf("\033[0;31m [INFO] \033[0;37m"); }
void greenInfo() { printf("\033[0;32m [INFO] \033[0;37m"); }
void yellowInfo() { printf("\033[0;33m [INFO] \033[0;37m"); }


void *publishBook(publisher_t *thread) {
	greenInfo();
	printf("Thread %d have been created for publisher type %d\n", thread->publisher_id, thread->publisher_type);

	int sem_idx = (thread->publisher_type - 1);

	sem_t *current_emtpy_sem = (empty_semaphores + sem_idx);
	sem_t *current_full_sem = (full_semaphores + sem_idx);
	sem_t *current_publisher_mutex = (publisher_mutex + sem_idx);
	char **current_publisher_buffer = *(publisher_buffers + sem_idx);
	char *string;
	int sem_value, *n_book_published = (total_published_book+sem_idx);

	do {
	if (current_full_sem == sizeOfBuffer(current_publisher_buffer)) {
		doubleTheSize(current_publisher_buffer); // doubling the size for current buffer
	}
	//publish a book
	sem_wait(current_emtpy_sem);
	pthread_mutex_lock(current_publisher_mutex);

	string = malloc(8); // book_name as a string
	sem_getvalue(current_full_sem, &sem_value); // getting the int value of semaphore

	thread->published_book += 1; // incrementing total published book
	sprintf(string, "Book%d_%d", thread->publisher_id, ++(*n_book_published)); // book name
	*(current_publisher_buffer + sem_value) = string; // adding book to buffer

	pthread_mutex_unlock(current_publisher_mutex);
	sem_post(current_full_sem);

	printf("%s is published and put into the buffer %d.", string, thread->publisher_type);

	} while(thread->published_book != n_books);
}

void *packBook(packager_t *thread) {
	greenInfo();
	printf("Packager %d have been created\n", thread->packager_id);

	int random_publisher_idx;
	sem_t *current_emtpy_sem;
	sem_t *current_full_sem;
	sem_t *current_publisher_mutex;
	char **current_publisher_buffer;
	
	do {
	random_publisher_idx = rand() % n_publisher_type;
	current_emtpy_sem = (empty_semaphores + random_publisher_idx);
	current_full_sem = (full_semaphores + random_publisher_idx);
	current_publisher_mutex = (publisher_mutex + random_publisher_idx);
	current_publisher_buffer = *(publisher_buffers + random_publisher_idx);

	sem_wait(current_full_sem);
	pthread_mutex_lock(current_publisher_mutex); 

	char *string = malloc(8); // book_name as a string
	int sem_value;
	sem_getvalue(current_full_sem, &sem_value); // getting the int value of semaphore

	thread->packaged_book += 1; // incrementing the number of packaged book

	pthread_mutex_unlock(current_publisher_mutex);
	sem_post(current_emtpy_sem);
	// package a book

	} while(thread->packaged_book != packaging_size);
}

void *doubleTheSize(char **current_publisher_buffer) {
	current_publisher_buffer = (char **) realloc(current_publisher_buffer, sizeof(current_publisher_buffer)*2);
}

int sizeOfBuffer(char **buffer){
	int i;
	for (i=0; (buffer+i) != NULL; i++);
	return i;
}

int main(int argc, char *argv[]) {
	int i, c, j, k, l;
	srand(time(NULL));   // Initialization, should only be called once.

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
	total_published_book = malloc(n_publisher_type * sizeof(int));

	// memory allocation for semaphores of each publication type
	empty_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	full_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	publisher_mutex = malloc(n_publisher_type * sizeof(pthread_mutex_t));

	// initializing semaphores
	for (l=0; l < n_publisher_type; l++){
		sem_init((empty_semaphores+l), 0, buffer_size);
		sem_init((full_semaphores+l), 0, 0);
		pthread_mutex_init((publisher_mutex+l), NULL);
	}
	pthread_mutex_init(&packager_mutex, NULL);

	// creating publisher threads
	for (i=0; i < total_publisher_thread; i++) {
		(publisher_thread+i)->publisher_type = (i / n_publisher_thread) + 1;
		(publisher_thread+i)->publisher_id = (i % n_publisher_thread) + 1;
		(publisher_thread+i)->published_book = 0;
		(publisher_thread+i)->thread_t = malloc(sizeof(pthread_t));
		pthread_create((publisher_thread+i)->thread_t, NULL, (void *) publishBook, (void *)(publisher_thread+i));
	}
	// creating packager threads
	for (j=0; j < n_packager_thread; j++) {
		(packager_thread+j)->packager_id = j+1;
		(packager_thread+j)->packaged_book = 0;
		(packager_thread+j)->thread_t = malloc(sizeof(pthread_t));
		pthread_create((packager_thread+j)->thread_t, NULL, (void *) packBook, (void *)(packager_thread+j));
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
