#include "systemcalls.h"

#include <sys/types.h>
#include <unistd.h> 
#include <sys/wait.h> 
#define _XOPEN_SOURCE
#include <stdlib.h> 
#include <sys/stat.h>
#include <fcntl.h> 

static bool wait_for_child (pid_t pid){

    int status;

    /* Wait for specified child */
    if (waitpid (pid, &status, 0) == -1) {
        perror("wait");
        return false;

    } else {
        /* Determine success or failure from exit status */
        if (WIFEXITED (status)) {
            if (WEXITSTATUS (status) > 0) {
                return false;
            } else {
                return true;
            }
        }
        else { 
            // Consider un-exited processes a failure.
            return false;
        }
    }
}

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/

    int status  = system(cmd);

    if (!status) {
        return true;
    } else {
        return false;
    }
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;

    bool retval;

    /* Fork, Exec, Wait */
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        retval = false;

    } else if (!pid) {
        /* Run command in child */
        execv (command[0], command);
        exit(EXIT_FAILURE);

    } else {
        retval = wait_for_child(pid);
    }

    va_end(args);

    return retval;

}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

    bool retval;

    /* Open file */
    int fd = open(outputfile, O_WRONLY|O_TRUNC|O_CREAT, 0644);
    if (fd < 0) { 
        // open failure
        va_end(args);
        return false;
    }

    /* Fork, Exec, Wait */
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        retval = false;
        
    } else if (!pid) {
        /* Replace stdin with outputfile, then exec the command */
        if (dup2(fd, 1) < 0) { 
            perror("dup");
            exit(EXIT_FAILURE);
        }
        close(fd);
        execv (command[0], command);
        exit(EXIT_FAILURE); // always a failure if exec* returns

    } else {
        retval = wait_for_child(pid);
    }

    va_end(args);

    return retval;

}
