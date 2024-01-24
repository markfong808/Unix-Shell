#include "shell.h"

int main(int argc, char **argv) {

  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
}


// interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE); // holds user input
  char *hist = calloc(1, MAXLINE); // holds history

  int start = 0; // index into args array
  while (should_run) {
    printf(PROMPT); // osh>
    fflush(stdout);
    // After reading user input
    int chars_read = fetchline(&line);
    //printf("read: %s (length = %d)\n", line, chars_read);

    // ^D results in n == -1
    if (chars_read == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }
    if (equal(line, "")) {
      continue; //blank line
    }
    
    if(equal(line, "!!")){
        line = historyGet(hist);
        if(equal(line,"")){
          printf(PROMPT "No commands in history \n");
          continue;
        } else {
          printf(PROMPT "%s \n", line);
        }
      }
      
      char **args = tokenize(line);//split string into parts
      while(args[start] != NULL) {
        int end;
        bool waitfor = parse(args, start, &end); // ends with
        doCommand(args, start, end, waitfor); //execute
        start = end + 2;
      }
      start = 0; //next line
      historySet(line, hist); // remember into history
    }
    free(line);
    free(hist);
    return 0;
}

int runTests() {
  //printf("*** Running basic tests ***\n");
  char *lines[] = {"ls",
                  "ls -al",
                  "ls & whoami ;",
                  "ls > junk.txt",
                  "cat < junk.txt",
                  "ls | wc",
                  "ascii",
                  "ls -al ; whoami",
                  "ls -al ; whoami & ls -al ; whoami"
                  "ls -al | wc",
                  "ls -al | wc ; whoami & ls -1",
                  "ls -1 | cat | tac | cat | tac | wc",
                  "sleep 5 ; ls",
                  "sleep 6 & ls",
                  NULL};
  int index = 0;
  char *line = lines[index];
  while (line != NULL) {
    printf("line %s\n", line);
    char **args = tokenize(line);
    int start = 0;
    int end; // don't know things when are going to end
    while (args[start] != NULL) {
      bool waitfor = parse(args, start, &end); // whether to see a semicolon ,ampersend                                   // sign or greater ,less than sign
      doCommand(args, start, end, waitfor);
      start = end + 2;
    }
    ++index;
    line = lines[index];
  }
  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0; //size of getline's buffer
  size_t n = getline(line, &len, stdin);
  if (n > 0 && (*line)[n - 1] == '\n') {
    // replace the last character string with NULL
    (*line)[n - 1] = '\0'; //stomp '\n' with '\0'
    return n - 1;
  }
  return n; // # chars read
}

// tokenize 'line'
char **tokenize(char *line) {
  int NUMTOKS = MAXLINE / 2 + 1; //Max number of tokens
  char **tokens; //array of strings
  
  char *linecopy = malloc(strlen(line + 1));
  strcpy(linecopy, line);

  tokens = malloc(NUMTOKS * sizeof(char *));

  int toknum = 0; // token number
  char *p = strtok(linecopy, " ");
  while (p != NULL) {
    assert(toknum < NUMTOKS); // don't overflow
    tokens[toknum] = p;
    ++toknum;
    p = strtok(NULL, " ");
  }
  tokens[toknum] = NULL;     // terminate token array
  tokens[toknum + 1] = NULL; // twice
  return tokens;
}

// parse function
bool parse(char **args, int start, int *end) {
  int i = start;
  while (args[i] != NULL) {
    if (equal(args[i], ";")) {
      *end = i - 1; 
      return true;  // parent will wait for child
    } else if (equal(args[i], "&")) {
      *end = i - 1;
      return false; // parent will not wait for child
    } else {
      ++i;
    }
  }
  *end = i - 1; // end index of command
  return true;
}

int findPipe(char **args){
  int i = 0;
  while(args[i] != NULL) {
    if(equal(args[i], "|"))
      return i;
      ++i;
  }
  return -1;
}

//execute the 2 command in 'args connected by a pipe
//Ex: args = {"ls" "-a", "|", "wc"}
//and pipe = 2, we will perform the command "ls -a | wc"
//split off the command for the parent from 'args'
//Ex: parentArgs = {"ls", "-a"}
//the parent will write, via a pipe, to the child
int doPipe(char **args, int pipei) {
  for (int i =0; i < pipei; ++i) {
    //printf("doPipe token: %s \n",args[i]);
  }
  // printf("Before allocating for parentargs \n");
  char **parentArgs = malloc(MAXLINE * sizeof(char *));
  // printf("Allocated %d bytes for parentArgs, MAXLINE \n");
  assert(parentArgs);
  
  for(int i = 0;i < pipei; ++i)
     parentArgs[i] = args[i];
     
     //Create a fork to connect parent to child
     
     int fd[2];
     int ret = pipe(fd);
     assert(ret != -1);
     
     //now fork, to create the child process
     
     ret = fork();
     assert(ret != -1);
     
     if(ret == 0) { //child
       close(fd[WR]); // won't write
       dup2(fd[RD],STDERR_FILENO);
       int nextPipe = findPipe(&args[pipei + 1]);
       if(nextPipe != -1){
        return doPipe(&args[pipei + 1], nextPipe);
       } else {
        execvp(args[pipei + 1],&args[pipei + 1]);
        return -1; //exec failed
       }
    } else {     //parent
      close(fd[RD]); //won't read
      dup2(fd[WR],STDOUT_FILENO);
      execvp(parentArgs[0], parentArgs);
      return -1; //failed
    }
    return 0;
}

// Do command function
void doCommand(char **args, int start, int end, bool waitfor) {
  
  char **newargs = malloc(MAXLINE * sizeof(char *));
  // printf("Alllocated %d bytes for newargs \n", sizeof(args) * sizeof(char *))
  int i = start;
  for (; i <= end; ++i)
    newargs[i - start] = args[i];
  newargs[i - start] = NULL;
  int pid = fork();
  assert(pid >= 0);

  if (pid == 0) { // child
    int ret = child(newargs);
    if (ret == -1) {
      printf(PROMPT "Invalid command \n");
      free(newargs);
      return;
    }
  } else { // in parent,with pid = child 's PID
    if (waitfor) {
      int status;
      waitpid(pid, &status, 0);
    }
  }
  free(newargs);
}
//Execute a child process.After removing any IO redirection from 'args
//the 'command' and 'args' are passed to the 'execvp' syscall. Return -1
//on failure. On success, does not return to caller.

int child(char **args) {
  int i = 0;
  while(args[i] != NULL) {
    //printf("child token &d: %s \n, i, args[i]");
    if(equal(args[i], ">")){
      int fd = open(args[i + 1],O_RDWR | O_CREAT, 0664);
      if (fd == -1)
        return fd;
      remIO(args,&i); //remove IO redirection
      int ret = dup2(fd, STDOUT_FILENO); // send stdout to file
      if(ret = -1)
      return ret; // failure
      execvp(args[0],args);
      return -1;      
    } else if (equal(args[i], "<")) {
      int fd = open(args[i + 1],O_RDONLY);
      if (fd == -1)
      return fd;
      remIO(args,&i); //remove IO redirection
      int ret = dup2(fd, STDIN_FILENO); //pull stdin from file
      if(ret == -1)
        return ret; //failure
      execvp(args[0],args);
      return -1;     
    } else if (equal(args[i], "|")){ //pipe
      args[i] = NULL;
      int fd[2];
      int ret = pipe(fd);
      assert(ret != -1);
      ret = fork();
      assert(ret != -1);
      if(ret == 0) { //child
        close(fd[RD]); // won't read
        dup2(fd[WR],STDOUT_FILENO);
        execvp(args[0], args);
        return -1; //failed
      } else { //parent
        close(fd[WR]); // won't write
        dup2(fd[RD], STDIN_FILENO);
        return child(&args[i + 1]);
      }
    } else {
      ++i;
    }
  }
  execvp(args[0],args);
  return -1;
}


// void printAsciiArt() {
//   printf("  |\\_/|        ****************************    (\\_/)\n");
//   printf(" / @ @ \\       *  \"Purrrfectly pleasant\"  *   (='.'=)\n");
//   printf("( > º < )      *       Poppy Prinz        *   (\")_(\")\n");
//   printf(" `>>x<<´       *   (pprinz@example.com)   *\n");
//   printf(" /  O  \\       ****************************\n");
// }

char *historyGet(char *hist) { return hist; }

// remember 'line' into the history buffer
void historySet(char *line, char *hist) {
  if (equal(line, "!!"))
    return; // don't remember !!
  if (equal(line, ""))
    return; // don't remember blank line
  strcpy(hist, line);
}

// Remove IO redirection from 'args' at index 'chevron'
// args = {"ls" , ">", "junk.txt", NULL} then reIO
void remIO(char **args, int *chevron) {
  int i = *chevron;
  while (args[i + 2] != NULL) {
    args[i] = args[i + 2];
    ++i;
  }
  args[i] = NULL;
  *chevron = i; // update caller 's index
}