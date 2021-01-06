# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <pthread.h>

pthread_t *publisher_thread;
pthread_t *packager_thread;

int main(int argc, char *argv[]) {
	int n_publisher_type, n_publisher_thread, n_packager_thread,
		n_books, packaging_size, buffer_size;

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
	
	publisher_thread = malloc(n_publisher_type * n_publisher_thread * sizeof(pthread_t));
	packager_thread = malloc(n_packager_thread * sizeof(pthread_t));

	free(publisher_thread);
	free(packager_thread);
	return 0;
}
