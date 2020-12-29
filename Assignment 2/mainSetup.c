#include <sys/wait.h>   // include necessary wait functions
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>   // include necessary directory handling functions
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>
 
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */

//flags:
#define CREATE_FLAGS1 (O_WRONLY|O_CREAT|O_TRUNC)
#define CREATE_FLAGS2 (O_WRONLY|O_CREAT|O_APPEND)
#define CREATE_MODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)
#define readOnly_Flag O_RDONLY

/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

// global variables
bool isFg = true;
int fg;

void setup(char inputBuffer[], char *args[], int *background) {
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO, inputBuffer, MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
       However, if this occurs, errno is set to EINTR. We can check this  value
       and disregard the -1 value */

    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	    exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
    for (i = 0; i < length; i++) { /* examine every character in the inputBuffer */

        switch (inputBuffer[i]) {
    	    case ' ':
	        case '\t' :               /* argument separators */
		        if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
		            ct++;
		        }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
        		if (start != -1){
                    args[ct] = &inputBuffer[start];     
        		    ct++;
		        }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
        		break;

	        default :             /* some other character */
        		if (start == -1)
		            start = i;
                if (inputBuffer[i] == '&'){
		            *background  = 1;
                    inputBuffer[i-1] = '\0';
		        }
	    } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

struct backgroundProcess {   // this struct represents a node in linkedlist when creating background processes
	char **commandLineArgs;   // this field stores commands and arguments entered by user
	struct backgroundProcess *nextBackgroundProcess;   // this field points to the next node in linkedlist
	pid_t backgroundProcessId;   // this field stores the background process id
	int processJobId;   // this field stores the job id of the background process
};

struct bookmark {   // this struct represents a node in linkedlist when creating bookmarks
	char **commandLineArgs;   // this field stores commands and arguments entered by user
	struct bookmark *nextBookmark;   // this field points to the next node in linkedlist
	int index;   // this field stores the index of the this node in linkedlist
};

int isFileExists(char *path) {   // this function is used to check if a file exists in the given path
    if (access(path, F_OK | X_OK) == -1)
        return 0;

    return 1;
}

void copyArgs(char **dest, char **src) {   // this function is used to copy the command line args to another array
	int i;
	for (i = 0; (src[i] != NULL) && (*src[i] != '&'); i++) {   // stop at either null or & (because processes can run in background)
		dest[i] = strdup(src[i]);   // dubplicate each element
	}
	dest[i] = NULL;
}

void copyBookmarkArgs(char **dest, char **src) {   // this function is used to copy the command line args of bookmark to another array.
	int i;										   // the difference with copyArgs() is that it takes care of "" in the source array
	for (i = 1; src[i] != NULL; i++) {
		dest[i-1] = strdup(src[i]);   // duplicate each element
	}
	strcpy(dest[0], dest[0] + 1);   // get rid of " in the first token
	strncpy(dest[i-2], dest[i-2], strlen(dest[i-2])-1);  // getting rid of " from the final token
	dest[i-2][strlen(dest[i-2])-1] = '\0';
	dest[i-1] = NULL;
}

void removeLLNode(struct backgroundProcess *currentPointer) {   // this function is used to delete a given node from the linkedlist
	struct backgroundProcess *tempPointer = currentPointer;   // create a temproray pointer
	tempPointer = currentPointer->nextBackgroundProcess;   // get the next node
	currentPointer = tempPointer;   // assign this node's pointer to next node's one
}

void shiftLLNodes(struct bookmark *next, struct bookmark *prev, struct bookmark **head) {   // this function is used to shift the contents of linkedlist
	struct bookmark *tempPointer = NULL;													// when a node is being deleted
	if (next == *head) {   // check if the head of linkedlist is being deleted
		tempPointer = *head;
		*head = next->nextBookmark;
	} else {   // check if other nodes are being deleted
		tempPointer = next;
		prev->nextBookmark = next->nextBookmark;
	}

	int index = tempPointer->index;   // loop over the remaining nodes and update their index field
	while (tempPointer->nextBookmark != NULL) {
		tempPointer = tempPointer->nextBookmark;
		tempPointer->index = index++;
	}
}

void catchCTRLZ(int sigNo) {   // this function is the handler when ctrl-Z is being presses
	int exit_stat;
	if (isFg) {   //checks if there are any foreground processes
		kill(fg, 0);   //checks if foreground process is still running, if it is not then it sets errno to ESRCH
		if (errno != ESRCH) {
			kill(fg, SIGKILL);   //because there is a foreground process still running it sends a kill signal
			waitpid(-fg, &exit_stat, WNOHANG);   //checks if any zombie children exits 
			isFg = false;
		} else {
			fprintf(stderr, "There is no foreground process which is still running");
			isFg = false;
			printf("\nmyshell:");
			fflush(stdout);
		}
	} else {
		printf("\nmyshell:");
		fflush(stdout);
	}
}

int search(char pwd[], int isRecursive, char keyword[]) {   // this function searches for the given keyword in files of pwd
	DIR *dir;
	struct dirent *dirEnt;
	dir = opendir(pwd);
	if (dir == NULL) {
		fprintf(stderr, "directory does not exist\n");
		return 1;
	}

	while ((dirEnt = readdir(dir)) != NULL) {   // loop over all files and directories in present working directory
		char *absPath = (char *) malloc(1 + strlen(pwd) + strlen(dirEnt->d_name));   // allocate memory for absPath
		strcpy(absPath, pwd);   // copy string
		strcat(absPath, "/");   // concatenate forward slash
		strcat(absPath, dirEnt->d_name);   // concatenate program name with absPath

		// this condition is used to check if the search is running in recursive mode
		if (isRecursive && dirEnt->d_type == 4 && strcmp(dirEnt->d_name, "..") != 0 && strcmp(dirEnt->d_name, ".") != 0) {
			search(absPath, isRecursive, keyword);   // recursive call to search in the new directory
		}

		char *filename = dirEnt->d_name;   // a pointer to filename
		int mode = 0;   // this mode is used to check if we have a .c, .C, .h, or .H file

		// the following conditions check for eligible file extensions
		for (int i = 0; filename[i] != '\0'; i++) {
			char ext = *(filename + i + 1);
			if (*(filename + i) == '.' && (ext == 'c' || ext == 'C' || ext == 'h' || ext == 'H') && *(filename + i + 2) == '\0') {
				mode = 1;
			}
		}

		// if no eligible file exists, then continue with the next file
		if (!mode) {
			continue;
		}

		char lineBuffer[1000];   // this array is used to store each line of the eligible file
		FILE *file;

		if ((file = fopen(absPath, "r")) == NULL) {   // this condition opens the file
			fprintf(stderr, "file does not exist!");
			exit(1);
		}

		char *status;   // this status is true until we reach EOF
		int line = 0;
		do {   // this loop gets a line of file in its iteration, and checks if the substring exists
			status = fgets(lineBuffer, sizeof(lineBuffer), file);
			if (strstr(lineBuffer, keyword) != NULL) {
				printf("%d: %s/%s -> %s", line, pwd, dirEnt->d_name, lineBuffer);   // print matched line
			}
			line++;
		} while (status);
		fclose(file);
	}
	closedir(dir);
	return 0;
}

int main(void) {
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    int programExecution;   // this variable is used to check if we are in execution mode or in built-in functions mode
	int backgroundProcessStatus;   // this variable is used to check if there are any background processes before exit function
    struct backgroundProcess *bgLLHead = NULL;   // this pointer is used to point to the head of linkedlist which keeps background processes
	struct bookmark *bmLLHead = NULL;   // this pointer is used to point to the head of linkedlist which keeps bookmarked items

	struct sigaction act;
	act.sa_handler = catchCTRLZ;   // determine the handler for the signal
	act.sa_flags = SA_RESTART;   // determine the flags for the signal
	int stat = sigemptyset(&act.sa_mask);
	if (stat == -1) {
		perror("Error to initialize signal set");
		exit(1);
	}
	stat=sigaction(SIGTSTP,&act,NULL);
	if (stat==-1) {
		perror("Error to set signal handler for SIGTSTP");
		exit(1);
	}

    while (1) {
        background = 0;
        programExecution = 1;   // this variable will be used to differentiate between different requirements

        printf("myshell: ");
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        
        /** the steps are:
        (1) fork a child process using fork()
        (2) the child process will invoke execv()
        (3) if background == 0, the parent will wait,
        otherwise it will invoke the setup() function again. */

		// this condition is to check whether the user entered any commands
		if (args[0] == NULL) {
			fprintf(stderr, "no command entered, try again!\n");
			continue;
		}

		// <ps_all>
        // this if condition will turn off execution mode and perform its task (ps_all)
        if (strcmp(args[0], "ps_all") == 0) {
            programExecution = 0;
	    	pid_t childProcessId;

			struct backgroundProcess *bgLLNode = bgLLHead;
			printf("Finished:\n");
			while (bgLLNode != NULL) {
				childProcessId = bgLLNode->backgroundProcessId;
				if (waitpid(childProcessId, NULL, WNOHANG) == childProcessId) {
					char **arguments = bgLLNode->commandLineArgs;
					printf("   [%d]  ", bgLLNode->processJobId);
					for (int i = 0; arguments[i] != NULL; i++) {
						printf("%s ", arguments[i]);
					}
					printf("(Pid=%d) \n", bgLLNode->backgroundProcessId);
					removeLLNode(bgLLNode);
				}
				bgLLNode = bgLLNode->nextBackgroundProcess;
			}

			printf("\nRunning:\n");
			bgLLNode = bgLLHead;
			while (bgLLNode != NULL) {
				pid_t childProcessId = bgLLNode->backgroundProcessId;
				if (waitpid(childProcessId, NULL, WNOHANG) == 0) {
					char **arguments = bgLLNode->commandLineArgs;
					printf("   [%d]  ", bgLLNode->processJobId);
					for (int i = 0; arguments[i] != NULL; i++) {
						printf("%s ", arguments[i]);
					}
					printf("(Pid=%d) \n", bgLLNode->backgroundProcessId);
				}
				bgLLNode = bgLLNode->nextBackgroundProcess;
			}
        }
		// </ps_all>

		// <search>
		if (strcmp(args[0], "search") == 0) {
			char lineBuffer[1000];
			char keyword[10];   // we assume the search word can be at most 10 chars, although its easily changeable
			int isRecursive = 0;
			programExecution = 0;

			if (args[1] == NULL) {   // this condition is to check if a keyword is provided
				fprintf(stderr, "a keyword should have been provided\n");
				continue;
			} else if (strcmp(args[1], "-r") == 0) {   // this condition is to check if we do a recursive search
				isRecursive = 1;
				strncpy(keyword, args[2] + 1, strlen(args[2]) - 2);
			} else {   // this condition is to check if we do a non recursive search
				strncpy(keyword, args[1] + 1, strlen(args[1]) - 2);
			}
			search(".", isRecursive, keyword);
		}
		// </search>

		// <bookmark>
		if (strcmp(args[0], "bookmark") == 0) {
			programExecution = 0;

			// this condition is used to list all of the bookmarked commands
			if (strcmp(args[1], "-l") == 0) {
				struct bookmark *bmLLNode = bmLLHead;
				while (bmLLNode != NULL) {
					char **bmArguments = bmLLNode->commandLineArgs;
					fprintf(stdout, "   %d  \" ", bmLLNode->index);
					for (int i = 0; bmArguments[i] != NULL; i++) {
						fprintf(stdout, "%s ", bmArguments[i]);
					}
					fprintf(stdout, "\"\n");
					bmLLNode = bmLLNode->nextBookmark;
				}
			}

			// this condition is used to execute a bookmarked command given its index
			else if (strcmp(args[1], "-i") == 0) {
				struct bookmark *bmLLNode = bmLLHead;
				while (bmLLNode != NULL) {
					int index = bmLLNode->index;
					if ((*args[2] - '0') == index) {
						copyArgs(args, bmLLNode->commandLineArgs);
						programExecution = 1;
						break;
					}
					bmLLNode = bmLLNode->nextBookmark;
				}
			}

			// this condition is used to delete a command from bookmark given its index
			else if (strcmp(args[1], "-d") == 0) {
				struct bookmark *nextNode = bmLLHead;
				struct bookmark *prevNode = NULL;
				while (nextNode != NULL) {
					int index = nextNode->index;
					if ((*args[2] - '0') == index) {
						shiftLLNodes(nextNode, prevNode, &bmLLHead);
						break;
					}
					prevNode = nextNode;
					nextNode = nextNode->nextBookmark;
				}
			}

			// this condition is used to add a new bookmarked command
			else {
				struct bookmark *bmLLNode = NULL;
				if (bmLLHead == NULL) {
					bmLLNode = (struct bookmark *) malloc(sizeof(struct bookmark));
					bmLLNode->index = 0;
					char **bookmarkArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
					copyBookmarkArgs(bookmarkArgs, args);
					bmLLNode->commandLineArgs = bookmarkArgs;
					bmLLNode->nextBookmark = NULL;
					bmLLHead = bmLLNode;
				} else {
					bmLLNode = bmLLHead;
					do {
						if (bmLLNode->nextBookmark == NULL) {
							break;
						}
						bmLLNode = bmLLNode->nextBookmark;
					} while (bmLLNode != NULL);

					struct bookmark *currentBMNode = (struct bookmark *) malloc(sizeof(struct bookmark));
					char **bookmarkArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
					copyBookmarkArgs(bookmarkArgs, args);
					currentBMNode->commandLineArgs = bookmarkArgs;
					currentBMNode->nextBookmark = NULL;
					int currentIndex = bmLLNode->index;
					currentBMNode->index = currentIndex + 1;
					bmLLNode->nextBookmark = currentBMNode;
                }

			}
		}
		// </bookmark>

		// <exit>
		// the following block contains the functionality of exit
		if (strcmp(args[0], "exit") == 0) {
			programExecution = 0;
			backgroundProcessStatus = 1;

			struct backgroundProcess *bgLLNode = bgLLHead;

			while (bgLLNode != NULL) {
				if (waitpid(bgLLNode->backgroundProcessId, NULL, WNOHANG) >= 1) {
					;
				}

				int status = kill(bgLLNode->backgroundProcessId, 0);
				if (status == 0) {
					fprintf(stderr, "dear user, you have background processes still running. close them first then exit.\n");
					backgroundProcessStatus = 0;
					break;
				}

				bgLLNode = bgLLNode->nextBackgroundProcess;
			}

			if (backgroundProcessStatus) {
				exit(0);
			}
		}
		// </exit>

		// <execution>
		// the following block contains the program execution
        if (programExecution) {
            pid_t childpid;
			int redirectionMode = 0;
			char inputFile[MAX_LINE/2+1], outputFile[MAX_LINE/2+1];
			int fd;
			int fd2;

            if ((childpid = fork()) == -1 ) {   // this condition is to check if fork was successfull
                fprintf(stderr, "failed to fork!");
                continue;

            } else if (childpid == 0) {   // this condition is true when the processor schedules the child process

				for (int i = 0; args[i] != NULL; i++) {

					if (strcmp(args[i], ">") == 0) {   // checks if redirection is in create/truncate mode
						if (*(args + i + 1) == NULL) {   // if there is no output file gives necessary warnings
							fprintf(stderr, "A file should have been provided\n");
							exit(1);
						}
						args[i] = NULL;
						redirectionMode = 1;   // sets 1 if its in create/truncate mode for later checks
						strcpy(outputFile, *(args + i + 1));   // copy output file name
						break;

					} else if (strcmp(args[i], ">>") == 0) {   //checks if redirection is in create/append mode
						if (*(args + i + 1) == NULL) {
							fprintf(stderr, "A file should have been provided\n");
							exit(1);
						}
						args[i]=NULL;
						redirectionMode = 2;   // sets 2 if its in create/append mode for later checks
						strcpy(outputFile, *(args + i + 1));   // copy output file name
						break;

					} else if(strcmp(args[i], "<") == 0) {   // checks if redirection is in input mode
						if (*(args + i + 1) == NULL) {
							fprintf(stderr, "A file should have been provided\n");
							exit(1);
						}
						args[i] = NULL;
						redirectionMode = 3;   //sets 3 if its in input mode
						strcpy(inputFile, *(args + i + 1));
						if (*(args + i + 2) != NULL && strcmp(args[i+2], ">") == 0) {   //there is an output redirection so it is been handling here
							if(*(args + i + 3) == NULL) {
								fprintf(stderr, "A file should have been provided\n");
								exit(1);
							}
							redirectionMode += 1;   // create/truncate + input mode=4
							strcpy(outputFile, *(args + i + 3));
							break;				
						} else if (*(args + i + 2) == NULL) {
							break; //because there is no output redirection					
						}

					} else if(strcmp(args[i], "2>") == 0) {   // checks if redirection is in error mode
						if (*(args + i + 1) == NULL) {
							fprintf(stderr,"A file should have been provided\n");
							exit(1);
						}
						args[i]=NULL;
						redirectionMode = 5; // sets 5 if redirection is in output error
						strcpy(outputFile, *(args + i + 1));   // copy output file name
						break;
					}
				}

				switch (redirectionMode) {

					// <case 1 : writing/truncation (>)>
					case 1:
						fd = open(outputFile, CREATE_FLAGS1, CREATE_MODE);   // open the output file
						if (fd == -1) {   // error handling if file could not open
							fprintf(stderr, "Failed to open the file");
							exit(1);
						}

						if (dup2(fd, STDOUT_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard output");
							exit(1);
						}

						if (close(fd) == -1) {   // error handling when closing the file
							fprintf(stderr, "Failed to close the file");
							exit(1);
						}
						break;
					// </case 1>

					// <case 2 : appending (>>)>
					case 2:
						fd = open(outputFile, CREATE_FLAGS2, CREATE_MODE);   // open the output file
						if (fd == -1) {   // error handling if file could not open
							fprintf(stderr, "Failed to open the file");
							exit(1);	
						}
						
						if (dup2(fd, STDOUT_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard output");
							exit(1);
						}

						if (close(fd) == -1) {   // error handling when closing the file
							fprintf(stderr, "Failed to close the file");
							exit(1);
						}
						break;
					// </case 2>

					// <case 3 : reading (<)>
					case 3:
						fd = open(inputFile, readOnly_Flag);   // open the input file
						if (fd == -1) {   // error handling if file could not open 
							fprintf(stderr, "Failed to open the file");
							exit(1);
						}
						
						if (dup2(fd, STDIN_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard input");
							exit(1);
						}

						if (close(fd) == -1) {   // error handling when closing the file
							fprintf(stderr, "Failed to close the file");
							exit(1);
						}
						break;
					// </case 3>

					// <case 4 : reading and writing at the same time>
					case 4:
						fd = open(inputFile, readOnly_Flag);   // open the input file
						fd2 = open(outputFile, CREATE_FLAGS1, CREATE_MODE);

						if ((fd == -1) || (fd2 == -1)) {   // error handling if file could not open 
							fprintf(stderr, "Failed to open the file");
							exit(1);
						}

						if (dup2(fd, STDIN_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard input");
							exit(1);
						}
						
						if (dup2(fd2, STDOUT_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard input");
							exit(1);
						}

						if ((close(fd) == -1) || (close(fd2) == -1)) {   // error handling when closing the file
							fprintf(stderr, "Failed to close the file");
							exit(1);
						}

						break;
					// </case 4>

					// <case 5 : writing stderr (2>)>
					case 5:
						fd = open(outputFile, CREATE_FLAGS1, CREATE_MODE);   // open the input file
						if (fd == -1) {   // error handling if file could not open 
							fprintf(stderr, "Failed to open the file");
							exit(1);
						}
						
						if (dup2(fd, STDERR_FILENO) == -1) {   // error handling if dup2 is unsuccessful
							fprintf(stderr, "Failed to redirect standard output");
							exit(1);
						}

						if (close(fd) == -1) {   // error handling when closing the file
							fprintf(stderr, "Failed to close the file");
							exit(1);
						}
						break;
					// </case 5>
				}

				char *path = getenv("PATH");   // this returns all of the dirs in PATH env variable
				if (path == NULL) {   // this condition is to check if PATH is empty
					fprintf(stderr, "getenv returned NULL\n");
					exit(1);
				}

				char *dir = strtok(path, ":");   // tokenize with a delimitir
				while (dir != NULL) {   // as long as there is a token (directory), run this loop
					char *absPath = (char *) malloc(1 + strlen(dir) + strlen(args[0]) );   // allocate memory for absPath
					strcpy(absPath, dir);   // copy string
					strcat(absPath, "/");   // concatenate forward slash
					strcat(absPath, args[0]);   // concatenate program name with absPath

					if (isFileExists(absPath)) {   // this condition checks if the given program exists in directory
						char **arguments = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
						copyArgs(arguments, args);
						execv(absPath, arguments);   // if so, execute execv() from child process
						fprintf(stderr, "An error must have happened when running execv()!");
						exit(1);   // terminate the child process if execv() did not work properly
					}

					dir = strtok(NULL, ":");
				}

            } else {   // this condition is true when the processor schedules the parent process
                if (background == 0) {   // this condition is true if the process is foreground
					fg = childpid;
					isFg = true;
                	waitpid(childpid, NULL, 0);   // wait for the child until its terminated
					isFg = false;

                } else {   // this condition is true if the process is background
                    struct backgroundProcess *bgLLNode = NULL;

                    if (bgLLHead == NULL) {   // this condition is true if there are no nodes in the linkedlist yet
                        bgLLNode = (struct backgroundProcess *) malloc(sizeof(struct backgroundProcess));   // allocate memory for the node
                        bgLLNode->backgroundProcessId = childpid;   // set the node's background process id
						char **bgArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);   // create an auxiliary args array
						copyArgs(bgArgs, args);   // copy arguments to the auxiliary array
                        bgLLNode->commandLineArgs = bgArgs;   // set the command line arguments of the node
                        bgLLNode->nextBackgroundProcess = NULL;   // set the next node as NULL
						bgLLNode->processJobId = 1;   // set the job id
                        bgLLHead = bgLLNode;   // assign the created node to linked list head

                    } else {   // this condition is true, if there are at least 1 node in the linked list
                        bgLLNode = bgLLHead;

                        do {   // this loop will traverse over the linked list to get its final node
                            if (bgLLNode->nextBackgroundProcess == NULL) {
                                break;
                            }
                            bgLLNode = bgLLNode->nextBackgroundProcess;
                        } while (bgLLNode != NULL);

                        struct backgroundProcess *currentBGNode = NULL;   // create and allocate memory to a node
                        currentBGNode = (struct backgroundProcess *) malloc(sizeof(struct backgroundProcess));
                        currentBGNode->backgroundProcessId = childpid;   // set the background process id
						char **bgArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);   // create an auxiliary args array
						copyArgs(bgArgs, args);   // copy args
                        currentBGNode->commandLineArgs = bgArgs;   // set command line args
                        currentBGNode->nextBackgroundProcess = NULL;   // set the next as NULL
						int currentJobId = bgLLNode->processJobId;   // get the previous node's job id, increment and assign it to new node
						currentBGNode->processJobId = currentJobId + 1;
                        bgLLNode->nextBackgroundProcess=currentBGNode;   // add newly created node to the end of linked list
                    }
                }
            }
        }
		// </execution>
    }
}
