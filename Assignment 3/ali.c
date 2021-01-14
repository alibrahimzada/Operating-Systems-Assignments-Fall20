# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
# include <semaphore.h>

typedef struct {   // this structure is used to pass multiple data to a publisher thread
	int publisher_type;
	int publisher_id;
} publisher_args;

// threads
pthread_t *publisher_thread;
pthread_t *packager_thread;

sem_t *empty_semaphores; // counting sem should be initialized with buffer_size for each publisher
sem_t *full_semaphores; // counting sem should be initialized with 0 for each publisher
pthread_mutex_t *publisher_mutex;
pthread_mutex_t packager_mutex;
int *total_published_book, total_packaged_book = 0;

char ***publisher_buffers; // triple pointer keeps that buffer_of_publisher_type->list_of_book_names->book_name (char *)
char **packager_buffers;

// global variables
int n_publisher_type, n_publisher_thread, n_packager_thread,
	n_req_books, package_size, buffer_size;

void red_info() { printf("\033[0;31m [INFO] \033[0;37m"); }
void green_info() { printf("\033[0;32m [INFO] \033[0;37m"); }
void yellow_info() { printf("\033[0;33m [INFO] \033[0;37m"); }

void *publish(void *thread_args) {
	publisher_args *args = (publisher_args *) thread_args;   // retrieving the arguments
	int books_published_by_thread = 0;

	// TODO: the first thread scheduled of this type will allocate memory to buffer
	// TODO: we should handle synchronization

	while (books_published_by_thread < n_req_books) {   // as long as a publisher thread did not publish required number of books, do:
		;

		books_published_by_thread++;   // incrementing the counter upon successful publishing
	}

	pthread_exit(NULL);   // terminating the publisher thread because it has finished publishing the required number of books
}

void *pack(void *thread_args) {
	int *packager_id = (int *) thread_args;

	// TODO: the first thread scheduled of this type will allocate memory to buffer
	// TODO: we should handle synchronization

	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
	srand(time(NULL));   // initialization, should only be called once.

	if (argc != 10) {   // checking if there are insufficient number of arguments
		// if so, print an error to stderr, and exit the program
		fprintf(stderr, "error :insufficient number of command line arguments\n");
		return 1;
	}

	for (int i = 0; i < argc; i++) {   // this loop parses the given arguments
		if (strcmp(argv[i], "-n") == 0) {
			n_publisher_type = atoi(argv[++i]);   // storing total number of publisher types
			n_publisher_thread = atoi(argv[++i]);   // storing total number of publisher thread per type
			n_packager_thread = atoi(argv[++i]);   // storing total number of packager thread
		} else if (strcmp(argv[i], "-b") == 0) {
			n_req_books = atoi(argv[++i]);   // storing the required number of books to be published per publisher thread
		} else if (strcmp(argv[i], "-s") == 0) {
			package_size = atoi(argv[++i]);   // storing package size
			buffer_size = atoi(argv[++i]);   // storing buffer size
		}
	}

	int total_publishers = n_publisher_type * n_publisher_thread;   // calculating total number of publisher threads
	publisher_args publisher_args_array[total_publishers];
	publisher_thread = malloc(total_publishers * sizeof(pthread_t));   // dynamic memory allocation for threads
	packager_thread = malloc(n_packager_thread * sizeof(pthread_t));   // dynamic memory allocation for threads
	total_published_book = calloc(n_publisher_type, sizeof(int));   // dynamic memory allocation for total published book for each thread type, and setting them to 0

	empty_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	full_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	publisher_mutex = malloc(n_publisher_type * sizeof(pthread_mutex_t));

	for (int i = 0; i < n_publisher_type; i++) {
		sem_init((empty_semaphores + i), 0, buffer_size);
		sem_init((full_semaphores + i), 0, 0);
		pthread_mutex_init((publisher_mutex + i), NULL);
	}
	pthread_mutex_init(&packager_mutex, NULL);

	for (int i = 0; i < total_publishers; i++) {   // this loop creates each publisher thread and ask them to publish
		publisher_args_array[i].publisher_type = i / n_publisher_thread + 1;   // setting publisher type
		publisher_args_array[i].publisher_id = (i % n_publisher_thread) + 1;   // setting publisher id of a specific type
		pthread_create(&publisher_thread[i], NULL, publish, (void *) &publisher_args_array[i]);
	}

	for (int i = 0; i < n_packager_thread; i++) {   // this loop creates each packager thread and ask them to pack
		int packager_id = i + 1;
		pthread_create(&packager_thread[i], NULL, pack, (void *) &packager_id);
	}

	for (int i = 0; i < total_publishers; i++) {   // this loop forces the main thread to wait for each publisher thread to finish
		pthread_join(publisher_thread[i], NULL);
	}

	for (int i = 0; i < n_packager_thread; i++) {   // this loop forces the main thread to wait for each packager thread to finish
		pthread_join(packager_thread[i], NULL);
	}

	free(publisher_thread);   // deallocating the allocated memory for publisher threads from heap
	free(packager_thread);   // deallocating the allocated memory for packager threads from heap
	free(total_published_book);   // deallocating the allocated memory for total published book from heap

	for (int i = 0; i < n_publisher_type; i++) {   // this loop destroys the created semaphores and mutexes because the program will end
		sem_destroy(empty_semaphores + i);
		sem_destroy(empty_semaphores + i);
		pthread_mutex_destroy(publisher_mutex + i);
	}
	pthread_mutex_destroy(&packager_mutex);
	
	free(empty_semaphores);   // deallocating the allocated memory for empty semaphores from heap
	free(full_semaphores);   // deallocating the allocated memory for full semaphores from heap
	free(publisher_mutex);   // deallocating the allocated memory for publisher mutexes from heap

	puts("main waited successfully");
	return 0;
}
