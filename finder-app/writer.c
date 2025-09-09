#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "syslog.h"

int main(int argc, char* argv[]) {
    
    /* Initialise syslog */
    openlog("writer", 0, LOG_USER);

    /* Parse Args (write-file and write-text)*/
    if (argc != 3) {
        fprintf(stderr, "Usage: writer <writefile> <writestr>\n");
        exit(EXIT_FAILURE);
    }
    char* writeFile = argv[1];
    char* writeText = argv[2];


    /* Open file - I think I should use O_TRUNC to clear the file, and O_CREAT in case it doesn't exist. */
    int fd;
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH; // rw-rw-rw-
    fd = open (writeFile, O_WRONLY | O_TRUNC | O_CREAT, mode);
    if (fd == -1) {
        syslog (LOG_ERR, "%s cannot be opened", writeFile);
        exit(EXIT_FAILURE);
    }

    /* Write the text to the file */
    int count = strlen(writeText);
    syslog (LOG_DEBUG, "Writing %s to %s", writeText, writeFile);
    int written = write (fd, writeText, count);
    if (written != count) {
        syslog (LOG_ERR, "Writing to %s failed", writeFile);   
        exit(EXIT_FAILURE);
    }

    /* Sync the file */
    if (fsync(fd) != 0){
        syslog (LOG_ERR, "Syncing %s failed", writeFile);   
        exit(EXIT_FAILURE);
    }

    /* Close the file */
    if (close(fd) != 0){
        syslog (LOG_ERR, "Closing %s failed", writeFile);   
        exit(EXIT_FAILURE);
    }
}