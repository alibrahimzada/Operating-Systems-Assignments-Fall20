#include <sys/wait.h>
#include <sys/types.h> 
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
 
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
 
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

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

struct backgroundProcess {
	char **commandLineArgs;
	struct backgroundProcess *nextBackgroundProcess;
	pid_t backgroundProcessId;
	int processJobId;
};

struct bookmark {
	char **commandLineArgs;
	struct bookmark *nextBookmark;
	int index;
};

int isFileExists(char *path) {
    // Check for file existence
    if (access(path, F_OK | X_OK) == -1)
        return 0;

    return 1;
}

void copyArgs(char **dest, char **src) {
	int i;
	for (i = 0; (src[i] != NULL) && (*src[i] != '&'); i++) {
		dest[i] = strdup(src[i]);
	}
	dest[i] = NULL;
}

void copyBookmarkArgs(char **dest, char **src) {
	int i;
	for (i = 1; src[i] != NULL; i++) {
		dest[i-1] = strdup(src[i]);
	}
	strcpy(dest[0], dest[0] + 1);   // get rid of " in the first token
	strncpy(dest[i-2], dest[i-2], strlen(dest[i-2])-1);  // getting rid of " from the final token
	dest[i-2][strlen(dest[i-2])-1] = '\0';
	dest[i-1] = NULL;
}

void removeLLNode(struct backgroundProcess *currentPointer) {
	struct backgroundProcess *tempPointer = currentPointer;
	tempPointer = currentPointer->nextBackgroundProcess;
	currentPointer = tempPointer;
}

void catchCTRLZ(int sigNo){
	int exit_stat;
	if (isFg) { //checks if there is any fg process
		kill(fg,0); //checks if fg process is still running, if it is not then it sets errno to ESRCH
		if (errno != ESRCH) {
			kill(fg, SIGKILL); //because there is a fg process still running it send a kill signal
			waitpid(-fg, &exit_stat, WNOHANG);//checks if any zombie children exits 
			isFg = false;
		} else {
			fprintf(stderr, "There is no foreground process which is still running");
			isFg=false;
			printf("\nmyshell:");
			fflush(stdout);				
		}
	} else {
		printf("\nmyshell:");
		fflush(stdout);
	}
}

int main(void) {
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    int programExecution;
	int backgroundProcessStatus;
    struct backgroundProcess *bgLLHead = NULL;
	struct bookmark *bmLLHead = NULL;

	struct sigaction act;
	act.sa_handler=catchCTRLZ;
	act.sa_flags=SA_RESTART;
	int stat=sigemptyset(&act.sa_mask);
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
				;
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
            if ( (childpid = fork()) == -1 ) {   // this condition is to check if fork was successfull
                fprintf(stderr, "failed to fork!");
                continue;
            } else if (childpid == 0) {   // this condition is true when the processor schedules the child process

                char *path = getenv("PATH");   // this returns all of the dirs in PATH env variable
                if (path == NULL) {   // this condition is to check if PATH is empty
                    fprintf(stderr, "getenv returned NULL\n");
                    exit(0);
                }

                char *dir = strtok(path, ":");   // tokenize with a delimitir
                while(dir != NULL) {   // as long as there is a token (directory), run this loop
                    char *absPath = (char *) malloc(1 + strlen(dir) + strlen(args[0]) );   // allocate memory for absPath
                    strcpy(absPath, dir);   // copy string
                    strcat(absPath, "/");   // concatenate forward slash
                    strcat(absPath, args[0]);   // concatenate program name with absPath

                    if (isFileExists(absPath)) {   // this condition checks if the given program exists in directory
						char **arguments = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
						copyArgs(arguments, args);
                        execv(absPath, arguments);   // if so, execute execv() from child process
                        fprintf(stderr, "An error must have happened when running execv()!");
                        return(1);   // terminate the child process if execv() did not work properly
                    }

		            dir = strtok(NULL, ":");
	            }

            } else {   // this condition is true when the processor schedules the parent process
                if (background == 0) {
					fg=childpid;
					isFg=true;
                	waitpid(childpid, NULL, 0);
					isFg = false;
                } else {
                    struct backgroundProcess *bgLLNode = NULL;

                    if (bgLLHead == NULL) {
                        bgLLNode = (struct backgroundProcess *) malloc(sizeof(struct backgroundProcess));
                        bgLLNode->backgroundProcessId = childpid;
						char **bgArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
						copyArgs(bgArgs, args);
                        bgLLNode->commandLineArgs = bgArgs;
                        bgLLNode->nextBackgroundProcess = NULL;
						bgLLNode->processJobId = 1;
                        bgLLHead = bgLLNode;
                    } else {
                        bgLLNode = bgLLHead;

                        do {
                            if (bgLLNode->nextBackgroundProcess == NULL) {
                                break;
                            }
                            bgLLNode = bgLLNode->nextBackgroundProcess;
                        } while (bgLLNode != NULL);

                        struct backgroundProcess *currentBGNode = NULL;
                        currentBGNode = (struct backgroundProcess *) malloc(sizeof(struct backgroundProcess));
                        currentBGNode->backgroundProcessId = childpid;
						char **bgArgs = malloc(sizeof(char*) * MAX_LINE / 2 + 1);
						copyArgs(bgArgs, args);
                        currentBGNode->commandLineArgs = bgArgs;
                        currentBGNode->nextBackgroundProcess = NULL;
						int currentJobId = bgLLNode->processJobId;
						currentBGNode->processJobId = currentJobId + 1;
                        bgLLNode->nextBackgroundProcess=currentBGNode;
                    }
                }
            }
        }
		// </execution>
    }
}
