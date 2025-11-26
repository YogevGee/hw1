#include "exec_utils.h"
#include <stdio.h>   // printf, fprintf
#include <stdlib.h>  // exit
#include <unistd.h>  // fork, execvp
#include <sys/wait.h> // waitpid
#include <errno.h> // errno
#include <signal.h>  // kill (optional, for background error handling)

// Print: hwshell: <name> failed, errno is <errno>
void print_syscall_error(const char *name)
{
    fprintf(stderr, "hwshell: %s failed, errno is %d\n", name, errno);
}

int run_foreground(char *argv[], const char *cmdline)
{
    (void)cmdline;


    pid_t pid = fork();
    if (pid < 0) 
    {
        print_syscall_error("fork");
        return -1;
    } 
    if (pid == 0)
    {
        // Child process
        execvp(argv[0], argv);
        
         // If execvp returns, an error occurred
        fprintf(stderr, "hwshell: invalid command\n");
        exit(1);
        
    } 

    // Parent process
    int status;
    if (waitpid(pid, &status, 0) < 0) 
        {
            print_syscall_error("waitpid");
            return -1;
        }

    return 0;
}

int run_background(char *argv[], const char *cmdline)
{
    pid_t pid = fork();
    if (pid <0)
    {
        print_syscall_error("fork");
        return -1;
    }

    if (pid == 0)
    {
        // Child process
        execvp(argv[0], argv);
        
        // If execvp returns, an error occurred
        fprintf(stderr, "hwshell: invalid command\n");
        exit(1);
    }
    
    if (jobs_add(pid, cmdline) < 0)
    {
        fprintf(stderr, "hwshell: too many background commands running\n");
        return -1;
    }
    printf("hwshell: pid %d started\n",pid);

    return 0;

  }