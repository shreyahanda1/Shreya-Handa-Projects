#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> //for PATH_MAX
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER 1001000

int valid(const char *f) {
    if (strlen(f) >= PATH_MAX) {
        fprintf(stderr, "Invalid Command\n");
        return 1;
    }

    for (const char *filePattern = f; *filePattern; filePattern++) {
        if (*filePattern < 32 || *filePattern > 126) {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }

        if (*filePattern == ' ') {
            fprintf(stderr, "Invalid Command\n");
            return 1;
        }
    }
    return 0;
}

int main(void) {
    char *arr = malloc(BUFFER);
    if (arr == NULL) {
        fprintf(stderr, "Operation Failed\n");
        free(arr);
        return 1;
    }

    ssize_t charSeen;
    ssize_t totalChar = 0;

    memset(arr, 0, BUFFER);

    while (totalChar < BUFFER - 1) {
        charSeen = read(STDIN_FILENO, arr + totalChar, BUFFER - 1 - totalChar);
        if (charSeen <= 0) {
            break;
        }

        totalChar += charSeen;
    }
    arr[totalChar] = '\0';

    if (totalChar == 0) {
        fprintf(stderr, "Invalid Command\n");
        free(arr);
        return 1;
    }

    char *gs = strtok(arr, "\n");
    if (gs == NULL) {
        fprintf(stderr, "Invalid Command\n");
        free(arr);
        return 1;
    }

    if (strcmp(gs, "get") == 0) {
        if (totalChar == 0 || arr[totalChar - 1] != '\n') {
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        char *location = strtok(NULL, "\n");
        if (location == NULL || valid(location)) {
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        // gets info about files or directories
        // checks if directory or not, if it is, exits
        struct stat dir;
        stat(location, &dir);

        if (S_ISDIR(dir.st_mode)) {
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        if (access(location, R_OK) == -1) {
            if ((errno == ENOENT) || (errno == EACCES)) {
                fprintf(stderr, "Invalid Command\n");
                free(arr);
                return 1;
            }
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        char *extra = strtok(NULL, "\n");
        if (extra) {
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        int fileOpen = open(location, O_RDONLY);
        if (fileOpen == -1) {
            fprintf(stderr, "Operation Failed\n");
            free(arr);
            close(fileOpen);
            return 1;
        }

        while ((charSeen = read(fileOpen, arr, BUFFER)) > 0) {
            if (write(STDOUT_FILENO, arr, charSeen) != charSeen) {
                fprintf(stderr, "Invalid Command\n");
                close(fileOpen);
                free(arr);
                return 1;
            }
            close(fileOpen);
            return 0;
        }

    } else if (strcmp(gs, "set") == 0) {
        char *location = strtok(NULL, "\n");
        char *content_length = strtok(NULL, "\n");
        char *contents = strtok(NULL, "");

        if (location == NULL || content_length == NULL || valid(location)) {
            fprintf(stderr, "Invalid Command\n");
            free(arr);
            return 1;
        }

        if (contents == NULL) {
            contents = "";
        }

        int length = atoi(content_length);

        int fileOpen2 = open(location, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fileOpen2 == -1) {
            fprintf(stderr, "Operation Failed\n");
            free(arr);
            close(fileOpen2);
            return 1;
        }

        ssize_t total_written = 0;

        // writes the content in chunks until the entire length is written
        while (total_written < length) {
            ssize_t charSeen = write(fileOpen2, contents + total_written, length - total_written);

            if (charSeen == -1) {
                fprintf(stderr, "Operation Failed\n");
                close(fileOpen2);
                free(arr);
                return 1;
            }

            // updates and keep tracks of  total bytes written
            total_written += charSeen;
        }

        close(fileOpen2);
        printf("OK\n");
        free(arr);
        return 0;

    } else {
        fprintf(stderr, "Invalid Command\n");
        free(arr);
        return 1;
    }
    free(arr);
    return 0;
}