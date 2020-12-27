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

// to determine it is a file or folder
int is_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode); // returns 1 if file
}

// linkedList to keep line, path and lineNumber
typedef struct fileInfo {
    char **line;
    char *path;
    int lineNumber;
    struct fileInfo *nextFileInfo;
} files;

// linkedlist to keep subfolders
struct subFolders {
    char *path;
    struct subFolders *nextFolder;
};

// searching specified word inside file
void searchInsideFile(char *path, const char *word, files *nextFile) {
    FILE * fp; // filePointer of type FILE to open file
    char * line = NULL; // line of file that word appears
    size_t len = 0; // length of file
    int lineCounter = 0; // lineNumber of file that word appears
    ssize_t read; // reading file line by line, in line 47

    char *dot = strrchr(path, '.');  // finding the latest dot in path
    if (dot && (!strcmp(dot, ".c") || !strcmp(dot, ".C") ||
     !strcmp(dot, ".cpp") || !strcmp(dot, ".h") || !strcmp(dot, ".H"))) { // if file name endswith properly
        fp = fopen(path, "r"); // open file
        while ((read = getline(&line, &len, fp)) != -1) { // travers inside with each line
            if (strstr(line, word)!= NULL) { // if line includes word

                // add it to list
                nextFile->nextFileInfo = (files*) malloc(sizeof(files));
                nextFile = nextFile->nextFileInfo;
                nextFile->line = &line;
                nextFile->path = path;
                nextFile->lineNumber = lineCounter;
                nextFile->nextFileInfo = NULL;
            }
            lineCounter++;
        }
        // closing file and make the memory of line free
        fclose(fp);
        if (line)
            free(line);
    }
}

void listingCurrentDir(const char *word, int searchSubFolders, struct subFolders* headOfFolders, files *headOfFiles) {
	DIR *directory; // keeping directory
	struct dirent *filePointer; // filePointer for each file inside current directory
    struct subFolders *folderList; // creating a linkedList for sub-folders
    files *fileList; // creating a linkedlist for founded files with specified word
    char *tempPath; // keeping path temporarly

    folderList = headOfFolders; // keeping head of sub-folders
    fileList = headOfFiles; // keeping head of files
    headOfFolders->path = "./"; // defining initial path as current directory
    folderList->nextFolder = NULL;
    fileList->nextFileInfo = NULL;

    while (headOfFolders != NULL) {    // while there is no folder left
        directory = opendir(headOfFolders->path); // opening directory
        if (directory != NULL)
        {
            while (filePointer = readdir(directory)) { // reading directory and defining each file to pointer
                tempPath = folderList->path; // keeping current directory inside tempPath
                strcat(tempPath, "/"); // concatinating current path with /
                strcat(tempPath, filePointer->d_name); // concatinating path with file name
                if (is_file(tempPath) == 0 && searchSubFolders) { // if temporary path is folder and -r given
                    folderList->nextFolder = (struct subFolders*) malloc(sizeof(struct subFolders));
                    folderList = folderList->nextFolder;
                    folderList->path = tempPath;
                    folderList->nextFolder = NULL;
                } 
                if (is_file(tempPath) == 1) { // if temporary path is a file
                    searchInsideFile(tempPath, word, fileList); // search specified word inside the file
                }
            }
        }
        else
            perror ("Couldn't open the directory");
        (void) closedir(directory);
        headOfFolders = headOfFolders->nextFolder; // go to next folder to search for
    }
}

int main(int argc, char const *argv[])
{
    struct subFolders *headOfFolders; // keeping the head of the subFolders
    files *headOfFiles; // keeping the head of files
    int searchSubFolders = 0; 
    if (strcmp(argv[1], "-r") == 0) {
        searchSubFolders = 1; // if -r given searchSubFolders will be 1
    }
    if (searchSubFolders == 1) { // if -r given;
        if (strcmp(argv[2], "search") == 0) {
            puts("-r given");
            listingCurrentDir(argv[3], searchSubFolders, headOfFolders, headOfFiles);
        }
    } else { // if -r not given
        if (strcmp(argv[1], "search") == 0) {
            listingCurrentDir(argv[2], searchSubFolders, headOfFolders, headOfFiles);
        }
    }
    return 0;
}
