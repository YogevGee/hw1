#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include "exec_utils.h"

//
// run_external:
//   argv[]     = parsed arguments
//   background = 1 if ends with &, 0 otherwise
//
// RETURNS:
//   Foreground -> 0 on success, -1 on error
//   Background -> child PID (>0) on success, -1 on error
//

int run_external(char *argv[], int background) {
    pid_t pid = fork();

    if (pid < 0) {
        // fork() failed
        fprintf(stderr, "hw1shell: fork failed, errno is %d\n", errno);
        return -1;
    }

    if (pid == 0) {
        // ---- CHILD PROCESS ----
        execvp(argv[0], argv);

        // execvp never returns unless error occurred
        fprintf(stderr, "hw1shell: execvp failed, errno is %d\n", errno);
        _exit(1); // exit child
    }

    // ---- PARENT PROCESS ----
    if (background == 1) {
        // do NOT wait → just return child PID
        return pid;
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "hw1shell: waitpid failed, errno is %d\n", errno);
        return -1;
    }

    // Check child's exit code. If execvp failed, child returns exit(1).
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        return -1;
    }

    return 0;

}

