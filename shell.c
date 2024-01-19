#include "shell.h"

int main(int argc, char **argv) {
  
  if (argc == 2 && equal(argv[1], "--interactive")) {
    return interactiveShell();
  } else {
    return runTests();
  }
  
}
//Global for bookmark history
char history[MAX_HIS_SIZE][MAXLINE];
int count = 0;

bool concurrenly = false;


// interactive shell to process commands
int interactiveShell() {
  bool should_run = true;
  char *line = calloc(1, MAXLINE);
  while (should_run) {
    printf(PROMPT);
    fflush(stdout);
    //After reading user input
    int n = fetchline(&line);
    printf("read: %s (length = %d)\n", line, n);
    
    //history command bookmark
    if(equal(line,"!!")){
      runPrevCommand();   
    }
    addTobookmark(line);
    
    // Display bookmarks
    displayBookmarks();
    
    // ^D results in n == -1
    if (n == -1 || equal(line, "exit")) {
      should_run = false;
      continue;
    }
    if (equal(line, "")) {
      continue;
    }
    
  processLine(line);
    
 }
  free(line);
  return 0;
}

void addTobookmark(char *line) {
    if(count < MAX_HIS_SIZE){
      strncpy(history[count],line,MAXLINE - 1);
      history[count][MAXLINE - 1] = '\0';
      count = (count + 1) % MAXLINE;
    }
}
void displayBookmarks() {
    printf("Bookmarks:\n");
    for (int i = 0; i < count; i++) {
      if(history[i][0] != '\0'){
        printf("%d. %s\n", i + 1, history[i]);
      }
    }
}
void runPrevCommand(){
   if(count == 0 || history[count - 1][0] == '\0'){
    printf("No previos command\n");   
   } else{
    printf("Run previous command : %s\n",history[count - 1]);        
    // Implement the code to execute the command here
    system(history[count - 1]);
   }
}
void processLine(char *line) { 
  
  printf("processing line: %s\n",line);
  char *args[25];
  int i = 0;
  parse_input(line,args);
  
char current_directory[130];
if (getcwd(current_directory, sizeof(current_directory)) != NULL) {
    //printf("Current working directory: %s\n", current_directory);
} else {
    perror("getcwd failed");
}
    bool concurrenly = false;
    int pipeLoc;
    //int i;
   //name a file descriptor
    int out_file = -1;
    int in_file =  -1;
    for(int i = 0; args[i] != NULL; i++){
      if(equal(args[i], ">")){
        out_file = i + 1;
      }
      if(equal(args[i], "<")){
        in_file = i + 1;
      }
      if(equal(args[i], "&")){//mean there are second command set
        concurrenly = true;
        args[i] = NULL;  // Remove the "&" from args
        
      }   
      // if(equal(args[i], "|")){
      //   pipeLoc = i;
      //   break;
      // }
      
      
    } //for loop end
  

    
  pid_t p1 = fork();
  if (p1 == 0) {
  // Child process  
  //input redirection
    if (in_file != -1 && args[in_file] != NULL) {
        char input_path[PATH_MAX];
        snprintf(input_path, PATH_MAX, "%s/junk.txt", current_directory); //docker container permission problem
        int redirect_fd = open(input_path, O_RDONLY);
        dup2(redirect_fd, STDIN_FILENO); // Connect file descriptor to stdin
        close(redirect_fd);
        args[in_file - 1] = NULL; //reset the flag back to -1
    }
    
    //output redirection
    if(out_file != -1 && args[out_file] != NULL){
    int redirect_fd = open(strcat(current_directory, "/junk.txt"), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU); 
    dup2(redirect_fd,STDOUT_FILENO); //connect file descriptor into stdout
    close(redirect_fd);
    args[out_file - 1] = NULL;
  }
    execvp(args[0],args);  
  }else {
// Parent process 
    //printf("[%d] %d\n", p1, p1);
    int status;
    waitpid(p1, &status, 0); // Wait for the specific child process to finish
    if (WIFEXITED(status)) {
            // Child process exited normally
            //printf("[%d] +  Done                    ls\n", p1);
        }
    if(concurrenly){
        execvp(args[2], args);
    }
    return 0;
  }

    // if (concurrenly) {
    // pid_t p2 = fork();
    // if (p2 == 0) {
    //   if(args[2] != NULL){
    //     // Child process for the second command
    //     execvp(args[2], args); 
    //   }       
    // } 
    
   }
        
  //parent process end
 
  


int runTests() {
  printf("*** Running basic tests ***\n");
  char lines[7][MAXLINE] = {
      "ls",      "ls -al", "ls & whoami ;", "ls > junk.txt", "cat < junk.txt",
      "ls | wc", "ascii"};
  for (int i = 0; i < 7; i++) {
    printf("* %d. Testing %s *\n", i + 1, lines[i]);
    processLine(lines[i]);
  }

  return 0;
}

// return true if C-strings are equal
bool equal(char *a, char *b) { return (strcmp(a, b) == 0); }

// read a line from console
// return length of line read or -1 if failed to read
// removes the \n on the line read
int fetchline(char **line) {
  size_t len = 0;
  size_t n = getline(line, &len, stdin);
  if (n > 0) {
    //replace the last character string with NULL
    (*line)[n - 1] = '\0';
  }
  return n;
}

void parse_input(char *input,char *tokens[]){
    bool concurrenly = false;
    int args_count = 0;
    // char *tokens[25];
    char *pch = strtok(input," ");//ex:return "ls"
    // char *ampersend = strtok(input,"&");
    while(pch != NULL){
      // char *ampersend = strtok(input,"&");
      if(equal(pch,"&")){
        concurrenly = true;
        // break;
      }
      printf("Token [%d]: %s\n", args_count, pch);
      tokens[args_count++] = pch;
      pch = strtok(NULL," ");
    }   
      tokens[args_count] = NULL; // Null-terminate the array
  }
  
// void callpipe(char *args,int pipeLoc){
//   //create pipe
//   enum {READ,WRITE};
//   int fd[2];
//   pipe(fd);
//   int pid = fork();
//   if(pid == 0){
//     close(fd[READ]);
//     dup2(fd[WRITE],STDOUT_FILENO);
//     args[pipeLoc] = NULL;
//     execvp(args[0],args);  
//   }else{
//     close(fd[WRITE]);
//     dup2(fd[READ],STDIN_FILENO);
//     execvp(args[pipeLoc + 1], &args[pipeLoc + 1]);
//   }
  
// }