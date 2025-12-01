#ifndef EXEC_UTILS_H
#define EXEC_UTILS_H

#include <sys/types.h> 

//declared in jobs.c,only used here.
int jobs_add(pid_t pid, const char *cmdline);

//run a command in the foreground (shell waits for it to finish).
int run_foreground(char *argv[], const char *cmdline);

//run a command in the background (shell doesnt wait).
int run_background(char *argv[], const char *cmdline);

//print an error for a failed system call, including errno.
void print_syscall_error(const char *name);

#endif //EXEC_UTILS_H
