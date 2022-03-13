/*****************************************************
A header library to facilitate basic interactions with
UNIX directories.
******************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

int dirop_count(char* dir_path) {
    printf("%s\n", dir_path);
    DIR *dp;
    int i = 0;
    struct dirent *ep;
    dp = opendir (dir_path);

    if (dp != NULL) {
        while (ep = readdir (dp)) i++;
        (void) closedir (dp);
    }
    else perror ("Couldn't open the directory");

    return i - 2; // i-2, as the . and .. dirs also get counted.
}
