#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <dirent.h>


int is_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

typedef struct fileInfo {
    char **line;
    char *path;
    int lineNumber;
    struct fileInfo *nextFileInfo;
} files;

struct subFolders {
    char *path;
    struct subFolders *nextFolder;
};

void searchInsideFile(char *path, const char *word, files *nextFile) {
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    int lineCounter = 1;
    ssize_t read;

    char *dot = strrchr(path, '.');
    if (dot && (!strcmp(dot, ".c") || !strcmp(dot, ".C") ||
     !strcmp(dot, ".cpp") || !strcmp(dot, ".h") || !strcmp(dot, ".H"))) {
        fp = fopen(path, "r");
        while ((read = getline(&line, &len, fp)) != -1) {
            if (strstr(line, word)!= NULL) {
                nextFile = (files*) malloc(sizeof(files*));
                nextFile->line = &line;
                nextFile->path = path;
                nextFile->lineNumber = lineCounter;
                nextFile = nextFile->nextFileInfo;
            }
            lineCounter++;
        }

        fclose(fp);
        if (line)
            free(line);
    }
}

void listingCurrentDir(const char *word, int searchSubFolders, struct subFolders* headOfFolders, files *headOfFiles) {
	DIR *directory;
	struct dirent *filePointer;
    struct subFolders *folderList;
    files *fileList;
    char *tempPath;

    folderList = headOfFolders;
    fileList = headOfFiles;
    folderList->path = "./";

    while (folderList != NULL) {
        directory = opendir(folderList->path);
        folderList->nextFolder = NULL;
        if (directory != NULL)
        {
            while (filePointer = readdir(directory)) {
                tempPath = folderList->path;
                strcat(tempPath, "/");
                strcat(tempPath, filePointer->d_name);
                if (is_file(tempPath) == 0 && searchSubFolders) {
                    folderList->nextFolder = (struct subFolders*) malloc(sizeof(struct subFolders*));
                    folderList = folderList->nextFolder;
                    folderList->path = tempPath;
                } 
                if (is_file(tempPath) == 1) {
                    searchInsideFile(tempPath, word, fileList);
                }
            }

            (void) closedir(directory);
        }
        else
            perror ("Couldn't open the directory");
    }
}

int main(int argc, char const *argv[])
{
    struct subFolders *headOfFolders;
    files *headOfFiles;
    int searchSubFolders = 0;
    if ((argv[0] == "search") == 0) {
        if ((argv[1] == "-r") == 0) {
            searchSubFolders = 1;
            listingCurrentDir(argv[2], searchSubFolders, headOfFolders, headOfFiles);
        }
        listingCurrentDir(argv[1], searchSubFolders, headOfFolders, headOfFiles);

    }
    return 0;
}
