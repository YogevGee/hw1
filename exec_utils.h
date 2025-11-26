#ifndef EXEC_UTILS_H
#define EXEC_UTILS_H

#include <sys/types.h> 

// Declared in jobs.c, We only use it here.
int jobs_add(pid_t pid, const char *cmdline);

// Run a command in the foreground (shell waits for it to finish).
int run_foreground(char *argv[], const char *cmdline);

// Run a command in the background (shell does NOT wait).
int run_background(char *argv[], const char *cmdline);

// Print an error for a failed system call, including errno.
void print_syscall_error(const char *name);

#endif //EXEC_UTILS_H
