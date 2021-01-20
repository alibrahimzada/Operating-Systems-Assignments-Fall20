# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>
# include <semaphore.h>

// threads
pthread_t *publisher_thread;   // we dynamically allocate memory at runtime and create publisher threads
pthread_t *packager_thread;   // we dynamically allocate memory at runtime and create packager threads

sem_t *empty_semaphores; // counting sem should be initialized with buffer_size for each publisher
sem_t *full_semaphores; // counting sem should be initialized with 0 for each publisher
pthread_mutex_t *publisher_mutex;   // publisher mutex for publishing-packing concurrency
pthread_mutex_t packager_mutex;   // packager mutex for packing consisentcy
int *total_published_book, *total_thread_left;   // global variables

char ***publisher_buffers; // triple pointer keeps that buffer_of_publisher_type->list_of_book_names->book_name (char *)
char **packager_buffers;   // double pointers for keeping a buffer of packaged books
int only_one_thread_allowed = 1;   // global variable

// the following variables will be updated once from command line arguments upon execution
int n_publisher_type, n_publisher_thread, n_packager_thread,
	n_req_books, package_size, buffer_size;

// a function to return the buffer size
int get_buffer_size(char **buffer) {
	int i;
	for (i = 0; *(buffer + i) != 0; i++);
	return i;
}

// a function to check if there is at least one thread remaining from a specific publisher type
int thread_checking(int publisher_type_idx) {
	int *publisher_type_threads_left = total_thread_left + publisher_type_idx;
	if (*publisher_type_threads_left == 0) {
		return 0;   // no threads left
	}
	return 1;   // at least one thread left of the given type
}

// a function to print the content of packager buffer once its full
void print_packager_buffer(int current_buffer_size){
	for (int i = 0; i < current_buffer_size; i++) {
		printf("%s ", *(packager_buffers + i));
		*(packager_buffers + i) = 0;
	}
}

// a custom publish function which will be run by each publisher thread
void *publish(void *thread_args) {
	int temp = (int) thread_args;   // retrieving the argument

	int publisher_type = (temp / n_publisher_thread) + 1;   // setting publisher type
	int publisher_id = (temp % n_publisher_thread) + 1;   // setting publisher id of a specific type
	int books_published_by_thread = 0;   // variable to keep track of total published books by a specific thread

	int sem_idx = (publisher_type - 1);   // getting the semaphore index of a publisher type
	int full_sem_value;
	int empty_sem_value;
	sem_t *current_empty_sem = empty_semaphores + sem_idx;   // retrieving the empty semaphore of a specific publisher type
	sem_t *current_full_sem = full_semaphores + sem_idx;   // retrieving the full semaphore of a specific publisher type
	pthread_mutex_t *current_publisher_mutex = publisher_mutex + sem_idx;   // retrieving the publisher mutex of a specific publisher type

	int *n_book_published = total_published_book + sem_idx;   // keeping track of total number of published books by a specific publisher type
	char *book_detail = malloc(16);   // we assume that published book details can have at most 15 characters (i.e. Book1_150000000)

	printf("\033[0;32m [INFO] \033[0;37m Thread %d have been created for publisher type %d\n", publisher_id, publisher_type);

	if (*(publisher_buffers + sem_idx) == NULL) {   // the first thread of a specific type scheduled will initialize the buffer
		*(publisher_buffers + sem_idx) = calloc(buffer_size, 16);
	}

	while (books_published_by_thread < n_req_books) {   // as long as a publisher thread did not publish required number of books, do:

		pthread_mutex_lock(current_publisher_mutex);   // lock the publisher mutex
		sem_getvalue(current_full_sem, &full_sem_value);   // getting the int value of full semaphore
		sem_getvalue(current_empty_sem, &empty_sem_value);   // getting the int value of empty semaphore
		int publisher_buffer_size = get_buffer_size(*(publisher_buffers + sem_idx));   // getting the buffer size of the corresponding publisher type
		if (publisher_buffer_size - empty_sem_value == publisher_buffer_size) {   // checking if buffer is full, if so, the publisher thread will first double its size
			printf("\033[0;32m [INFO] \033[0;37m Publisher %d of type %d : Buffer is full. Resizing the buffer.\n", publisher_id, publisher_type);
			for (int i = 0; *(*(publisher_buffers + sem_idx) + i) != 0; i++) {
				sem_post(current_empty_sem);
			}
			unsigned long buffer_size = sizeof(publisher_buffers + sem_idx);
			*(publisher_buffers + sem_idx) = realloc(*(publisher_buffers + sem_idx), buffer_size * buffer_size * 2);
		}
		sem_wait(current_empty_sem);   // publishing a book and decrementing the semaphore
		strcpy(book_detail, "");
		sprintf(book_detail, "Book%d_%d", publisher_type, ++(*n_book_published)); // book name
		*(*(publisher_buffers + sem_idx) + full_sem_value) = malloc(16);
		strcpy(*(*(publisher_buffers + sem_idx) + full_sem_value), book_detail); // adding book to buffer
		printf("\033[0;33m [INFO] \033[0;37m Publisher %d of type %d : %s is published and put into the buffer %d.\n", publisher_id, publisher_type, book_detail, publisher_type);

		books_published_by_thread++;   // incrementing the counter upon successful publishing
		sem_post(current_full_sem);   // incrementing after a book is successfully published
		pthread_mutex_unlock(current_publisher_mutex);   // unlocking the mutex
	}

	printf("\033[0;31m [INFO] \033[0;37m Publisher %d of type %d Finished publishing %d books. Exiting the system\n", publisher_id, publisher_type, n_req_books);
	free(book_detail);
	*(total_thread_left + sem_idx) = *(total_thread_left + sem_idx) - 1;   // updating the total thread left of a specific publisher type
	pthread_exit(NULL);   // terminating the publisher thread because it has finished publishing the required number of books
}

// a custom pack function which will be run by each packager thread
void *pack(void *thread_args) {
	int temp = (int) thread_args;   // retrieving the arguments

	int packager_id = temp + 1;   // setting the packager id

	printf("\033[0;32m [INFO] \033[0;37m Packager %d have been created\n", packager_id);

	// the following variables will be storing necessary data to process packing
	int random_publisher_idx;
	sem_t *current_emtpy_sem;
	sem_t *current_full_sem;
	pthread_mutex_t *current_publisher_mutex;
	char *book_detail = malloc(16); // book_name as a string

	if (packager_buffers == NULL) {   // the first packager thread will allocate the package buffer
		packager_buffers = calloc(package_size, 16);
	}

	int full_sem_value, current_package_size, i;
	while (only_one_thread_allowed) {   // as long as there is published books left in the buffers and publisher threads are running, do:

		pthread_mutex_lock(&packager_mutex);   // lock the packager mutex

		strcpy(book_detail, ""); // making string empty
		random_publisher_idx = rand() % n_publisher_type;   // randomly select one of the publisher buffers

		current_full_sem = (full_semaphores + random_publisher_idx);   // get the current full semaphore for the randomly selected publisher
		sem_getvalue(current_full_sem, &full_sem_value);
		for (i = 0; full_sem_value == 0 && thread_checking(random_publisher_idx) == 0 && i < n_publisher_type; i++) {
			// if the buffer of the randomly selected publisher is empty, and there are no threads left of that type, we select a new publisher type randomly
			random_publisher_idx = (random_publisher_idx + 1) % n_publisher_type;
			current_full_sem = (full_semaphores + random_publisher_idx);
			sem_getvalue(current_full_sem, &full_sem_value);
		}

		if (i == n_publisher_type || only_one_thread_allowed == 0) {   // if we have checked all of the available publisher types, and there exist no
																	   // book in buffers, and all publisher threads have finished execution, then packager will exit
			if (only_one_thread_allowed) {
				current_package_size++;
				printf("There are no publishers left in the system.\nOnly %d of %d number of books could be packaged.\n", current_package_size, package_size);
				printf("The package contains:\n");
				print_packager_buffer(current_package_size);
				printf("Exiting the system.\n");
				only_one_thread_allowed = 0;
			}
			pthread_mutex_unlock(&packager_mutex);
			break;
		}

		current_emtpy_sem = (empty_semaphores + random_publisher_idx);   // get the empty semaphore of the randomly selected publisher
		current_publisher_mutex = (publisher_mutex + random_publisher_idx);   // get the mutex of the randomly selected publisher

		sem_wait(current_full_sem);   // wait until there is a book in the buffer

		pthread_mutex_lock(current_publisher_mutex);   // lock the publisher mutex so no other publisher thread of the same type could publish a new book

		// popping from the beginning of data structure, and shifting elements 1 to the left (FIFO)
		strcpy(book_detail, *(*(publisher_buffers + random_publisher_idx))); // getting book from the beginning of buffer
		for (i = 0; i < get_buffer_size(*(publisher_buffers + random_publisher_idx)) - 1; i++) {   // shifting the books after popping the first element
			strcpy(*(*(publisher_buffers + random_publisher_idx) + i), *(*(publisher_buffers + random_publisher_idx) + i + 1));
		}
		*(*(publisher_buffers + random_publisher_idx) + i + 1) = 0;

		current_package_size = get_buffer_size(packager_buffers);   // get the current package size
		if (current_package_size == package_size) {   // if the current package is full, we print its content and then clear it
			printf("\033[0;33m [INFO] \033[0;37m Packager %d : Package is full: ", packager_id);
			print_packager_buffer(current_package_size);
			current_package_size = 0;
			printf("\n");
		}
		*(packager_buffers + current_package_size) = malloc(16);   // allocate space to the new element of package
		strcpy(*(packager_buffers + current_package_size), book_detail);

		sem_post(current_emtpy_sem);
		printf("\033[0;33m [INFO] \033[0;37m Packager %d : Put %s into the package.\n", packager_id, book_detail);

		pthread_mutex_unlock(current_publisher_mutex);   // unlock the publisher mutex after successfull packing

		pthread_mutex_unlock(&packager_mutex);   // unlock the packager mutex
	}

	free(book_detail);
	pthread_exit(NULL);   // packager thread will exit the system after its job is done
}

int main(int argc, char *argv[]) {
	int i;
	srand(time(NULL));   // initialization, should only be called once.

	if (argc != 10) {   // checking if there are insufficient number of arguments
		// if so, print an error to stderr, and exit the program
		fprintf(stderr, "error :insufficient number of command line arguments\n");
		return 1;
	}

	for (i = 0; i < argc; i++) {   // this loop parses the given arguments
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
	publisher_thread = malloc(total_publishers * sizeof(pthread_t));   // dynamic memory allocation for threads
	packager_thread = malloc(n_packager_thread * sizeof(pthread_t));   // dynamic memory allocation for threads
	total_published_book = calloc(n_publisher_type, sizeof(int));   // dynamic memory allocation for total published book for each thread type, and setting them to 0
	total_thread_left = calloc(n_publisher_type, sizeof(int));   // dynamic memory allocation for total thread left in the program

	for (i = 0; i < n_publisher_type; i++) {   // assigning initial values to total thread left of each type
		total_thread_left[i] = n_publisher_thread;
	}

	publisher_buffers = malloc(n_publisher_type * sizeof(**publisher_buffers));   // allocating memory to publisher buffers

	// allocating memory to semaphores and mutexes
	empty_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	full_semaphores = malloc(n_publisher_type * sizeof(sem_t));
	publisher_mutex = malloc(n_publisher_type * sizeof(pthread_mutex_t));

	for (i = 0; i < n_publisher_type; i++) {   // this loop initializes the semaphores and mutexes
		sem_init((empty_semaphores + i), 0, buffer_size);
		sem_init((full_semaphores + i), 0, 0);
		pthread_mutex_init((publisher_mutex + i), NULL);
	}
	pthread_mutex_init(&packager_mutex, NULL);

	for (i = 0; i < total_publishers; i++) {   // this loop creates each publisher thread and ask them to publish
		pthread_create(&publisher_thread[i], NULL, publish, (void *) i);
	}

	for (i = 0; i < n_packager_thread; i++) {   // this loop creates each packager thread and ask them to pack
		pthread_create(&packager_thread[i], NULL, pack, (void *) i);
	}

	for (i = 0; i < total_publishers; i++) {   // this loop forces the main thread to wait for each publisher thread to finish
		pthread_join(publisher_thread[i], NULL);
	}

	for (i = 0; i < n_packager_thread; i++) {   // this loop forces the main thread to wait for each packager thread to finish
		pthread_join(packager_thread[i], NULL);
	}

	free(publisher_thread);   // deallocating the allocated memory for publisher threads from heap
	free(packager_thread);   // deallocating the allocated memory for packager threads from heap
	free(total_published_book);   // deallocating the allocated memory for total published book from heap
	free(total_thread_left);   // deallocating the allocated memory for total thread left

	for (i = 0; i < n_publisher_type; i++) {   // this loop destroys the created semaphores, mutexes, and memory of buffers because the program will end
		sem_destroy(empty_semaphores + i);
		sem_destroy(full_semaphores + i);
		pthread_mutex_destroy(publisher_mutex + i);
		free(*(publisher_buffers + i));
	}
	pthread_mutex_destroy(&packager_mutex);
	
	free(empty_semaphores);   // deallocating the allocated memory for empty semaphores from heap
	free(full_semaphores);   // deallocating the allocated memory for full semaphores from heap
	free(publisher_mutex);   // deallocating the allocated memory for publisher mutexes from heap
	free(publisher_buffers);   // deallocating the allocated memory for publisher buffers from heap

	puts("main waited successfully");   // this print makes sure that main thread waited successfully for all of the created threads
	return 0;
}
