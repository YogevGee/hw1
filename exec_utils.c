#include "exec_utils.h"
#include <stdio.h>     //fprintf
#include <stdlib.h>    //exit
#include <unistd.h>    //fork,execvp
#include <sys/wait.h>  //waitpid
#include <errno.h>     //errno

//print: hw1shell: <name> failed, errno is <errno>
void print_syscall_error(const char *name)
{
    fprintf(stderr, "hw1shell: %s failed, errno is %d\n", name, errno);
}


int run_external(char *argv[], int background)
{
    pid_t pid = fork();
    if (pid < 0) {
        //fork failed
        print_syscall_error("fork");
        return -1;
    }

    if (pid == 0) {
        //child process
        execvp(argv[0], argv);

        //if execvp returns, an error occurred
        fprintf(stderr, "hw1shell: invalid command\n");
        print_syscall_error("execvp");
        exit(1);
    }

    //parent process
    if (background) {
        //dont wait; main() will store pid in its bg table
        return pid;
    } else {
        int status;
        if (waitpid(pid, &status, 0) < 0) {
            print_syscall_error("waitpid");
            return -1;
        }
        return 0;
    }
}