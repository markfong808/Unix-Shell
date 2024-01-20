#ifndef SHELL_H
#define SHELL_H

#include <assert.h>  // assert
#include <fcntl.h>   // O_RDWR, O_CREAT
#include <stdbool.h> // bool
#include <stdio.h>   // printf, getline
#include <stdlib.h>  // calloc
#include <string.h>  // strcmp
#include <unistd.h>  // execvp
#include <sys/wait.h>
#include <sys/stat.h>


#define MAXLINE 80
#define PROMPT "osh> "
#define MAX_HIS_SIZE 10
#define PATH_MAX 100
#define RD 0
#define WR 1
void checkFlags(char *line);
bool equal(char *a, char *b);
int fetchline(char **line);
int interactiveShell();
int runTests();
void processLine(char *line);
void addTobookmark(char *line);
void displayBookmarks();
void runPrevCommand();
void parse_input();
void split_commands();
void callpipe();
void printAsciiArt(); 
int main();
#endif