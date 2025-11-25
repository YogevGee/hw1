#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAXLINE 1024
#define MAXARGS 64
#define MAXBGS 4

struct bg_job {
    pid_t pid;
    char command[MAXLINE];
    int active;
};

struct bg_job jobs[MAXBGS];

extern int run_external(char *argv[], int background); // implemented by Partner

/* helper: trim whitespace from end */
static void rtrim(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r' || s[n-1]==' ' || s[n-1]=='\t')) {
        s[n-1] = '\0';
        n--;
    }
}

/* find an empty slot for background job */
int add_bg_job(pid_t pid, const char *cmd) {
    for (int i = 0; i < MAXBGS; ++i) {
        if (!jobs[i].active) {
            jobs[i].active = 1;
            jobs[i].pid = pid;
            strncpy(jobs[i].command, cmd, MAXLINE-1);
            jobs[i].command[MAXLINE-1] = '\0';
            return 0;
        }
    }
    return -1; // no slot
}

int has_free_bg_slot() {
    for (int i = 0; i < MAXBGS; ++i) {
        if (!jobs[i].active)
            return 1;
    }
    return 0;
}

void print_jobs() {
    for (int i = 0; i < MAXBGS; ++i) {
        if (jobs[i].active) {
            printf("%d\t%s\n", (int)jobs[i].pid, jobs[i].command);
        }
    }
}

void reap_finished_bg() {
    for (int i = 0; i < MAXBGS; ++i) {
        if (!jobs[i].active) continue;
        pid_t r = waitpid(jobs[i].pid, NULL, WNOHANG);
        if (r > 0) {
            printf("hw1shell: pid %d finished\n", (int)jobs[i].pid);
            jobs[i].active = 0;
        } else if (r == -1) {
            // error from waitpid
            fprintf(stderr, "hw1shell: waitpid failed, errno is %d\n", errno);
            jobs[i].active = 0;
        }
    }
}

int handle_cd(char *argv[]) {
    // cd with exactly 2 args: argv[0] == "cd", argv[1] is directory
    if (!argv[1]) {
        fprintf(stderr, "hw1shell: invalid command\n");
        return -1;
    }
    if (chdir(argv[1]) != 0) {
        fprintf(stderr, "hw1shell: invalid command\n");
        return -1;
    }
    return 0;
}

int main() {
    char line[MAXLINE];
    char *argv[MAXARGS+1];
    for (int i = 0; i < MAXBGS; ++i) jobs[i].active = 0;

    while (1) {
        printf("hw1shell$ ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            // EOF or error
            if (feof(stdin)) {
                printf("\n");
                break;
            }
            continue;
        }

        rtrim(line);
        if (strlen(line) == 0) {
            // empty command, show prompt again
            reap_finished_bg();
            continue;
        }

        // detect background '&' at end
        int background = 0;
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '&') {
            background = 1;
            // remove the & and any trailing spaces
            line[len-1] = '\0';
            rtrim(line);
        }

        // split into argv[]
        int argc = 0;
        char *tok = strtok(line, " \t");
        while (tok != NULL && argc < MAXARGS) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \t");
        }
        argv[argc] = NULL;
        if (argc == 0) {
            reap_finished_bg();
            continue;
        }

        // handle internal commands
        if (strcmp(argv[0], "exit") == 0) {
            // wait for all background processes
            for (int i = 0; i < MAXBGS; ++i) {
                if (jobs[i].active) {
                    waitpid(jobs[i].pid, NULL, 0);
                    jobs[i].active = 0;
                }
            }
            // exit cleanly
            return 0;
        } else if (strcmp(argv[0], "cd") == 0) {
            handle_cd(argv);
            reap_finished_bg();
            continue;
        } else if (strcmp(argv[0], "jobs") == 0) {
            print_jobs();
            reap_finished_bg();
            continue;
        }

          // ============================
        // External commands
        // ============================

        // Check background slots BEFORE running (Requirement #6)
        if (background && !has_free_bg_slot()) {
            printf("hw1shell: too many background commands running\n");
            reap_finished_bg();
            continue;   // DO NOT run run_external(), DO NOT fork
        }

        // Now it is safe to fork/exec
        int ret = run_external(argv, background);

        if (background) {
            // Background job: ret must be > 0 (PID)
            if (ret > 0) {
                if (add_bg_job((pid_t)ret, argv[0]) != 0) {
                    // Should be impossible because we already checked above
                    printf("hw1shell: too many background commands running\n");
                } else {
                    printf("hw1shell: pid %d started\n", ret);
                }
            } else {
                // execvp or fork failure in child
                printf("hw1shell: invalid command\n");
            }
        } else {
            // Foreground job: ret == 0 on success, <0 on error
            if (ret < 0) {
                printf("hw1shell: invalid command\n");
            }
        }

        // Reap background jobs at end of loop (Requirement #12)
        reap_finished_bg();
    }
}
