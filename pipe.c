#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>

char** tokenize(char *buffer) {
    char **tokens = (char **) malloc(80 * sizeof(char*));
    int toknum = 0;
    char * bufferCopy = (char *) malloc(strlen(buffer));
    strcpy(bufferCopy, buffer);
    char *p = strtok(bufferCopy, " ");//make a copy
    while (p != NULL) {
        printf("Token: %s\n", p);
        tokens[toknum] = p;
        toknum++;
        p = strtok(NULL, " ");
    }
    tokens[toknum] = NULL;
    return tokens;
}

int childWithPipe(char** args, int pipeLoc) {
    // create a pipe
    enum {READ, WRITE};
    int fd[2];
    pipe(fd);
    // make a fork
    int pid = fork();
    if (pid == 0) {
        // child execute "ls -al" with dup2 to STDOUT/pipewrite
        close(fd[READ]);
        dup2(fd[WRITE], STDOUT_FILENO);
        args[pipeLoc] = NULL;
        execvp(args[0], args);
    } else {
        // parent execute "wc" with dup2 to STDIN/pipepread
        close(fd[WRITE]);
        dup2(fd[READ], STDIN_FILENO);
        execvp(args[pipeLoc + 1], &args[pipeLoc + 1]);
    }
    return 0;
}
void doCommand(char** args) {
    // find where the pipe is
    
    int pipeLocation;
    int i;
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            break;
        }
    }
    pipeLocation = i;//store the location  " | "
    printf("pipe is at args[%d]\n", pipeLocation);
    int pid = fork();
    if (pid == 0) {
        int ret = childWithPipe(args, pipeLocation);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
}
int main() {
    char* buffer = "ls -al | wc";
    char **args = tokenize(buffer);
    doCommand(args);
}