//File: mysh.c
//
// Shell program satisfying Project 2 requirements
//

#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "mycd_pwd.h"
#include "mysh_cp.h"

char **mysh_splitLine(char *stringLine);
char *mysh_readLine(void);
int mysh_exec(char **param);

char *cmd_built[] = {"mycd","mypwd", "mycp"}; //built-in command

// func_builtin(): builtin function
int (*func_built[]) (char **) = { &mysh_cd, &mysh_pwd, &mycp };

int execute_subshell_command(char ***programs){
  int stat;
  char ***progs = programs;
  progs++;

  int p[2];
  pipe(p);

  //executing second program in argument
  //and piping its output
  if(progs != NULL){
    pid_t pid = fork();

    if(pid == 0){
        dup2(p[1], 1);
        close(p[0]);
        close(p[1]);
        char *prog_path = getenv(*progs[0]);
        if(!prog_path) { prog_path = *progs[0]; }
         if(execvp(prog_path, *progs) == -1){
        perror("mysh ");
        exit(EXIT_FAILURE);
      }
    }
    else if(pid < 0){
      perror("mysh ");
    }
    else{
       do {
          waitpid(pid, &stat, WUNTRACED);
       } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));
    }

  }

  char **args;
  size_t b_size = 100;
  char stringLine[b_size];
  char command[b_size];
  write(p[1], "\r\n",2);
  read(p[0], stringLine, b_size); //reading from the pipe
  close(p[0]);
  close(p[1]);
  sprintf(command, "%s %s", *programs[0], stringLine);;
  args = mysh_splitLine(command);


  if(programs != NULL){
    pid_t pid = fork();
    if(pid == 0){
        char *prog_path = getenv(*programs[0]);
        if(!prog_path) { prog_path = *programs[0]; }
         if(execvp(prog_path, args) == -1){
        perror("mysh ");
        exit(EXIT_FAILURE);
      }
    }
    else if(pid < 0){
      perror("mysh ");
    }
    else{
       do {
          waitpid(pid, &stat, WUNTRACED);
       } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));
    }
    free(args);

  }
  fflush(stdout);
  fflush(stdin);
  return 1;
}

//execute piped or subshell command
int execute_pipe_command(char ***programs){

  char ***progs = programs;
  int p[2], stat, i = 1;
  pipe(p);
  pid_t pid;
  while(i >= 0){
    pid = fork();
    if(pid == 0){

      dup2(p[i], i );
      close(p[0]);
      close(p[1]);
      char *prog_path = getenv(*progs[0]);
      if(!prog_path) { prog_path = *progs[0]; }
      if(execvp(prog_path, *progs) == -1){
        perror("mysh ");
        exit(EXIT_FAILURE);
      }
      else if(pid < 0){
        perror("mysh ");
      }
    }
     i-- ;
    progs++;
  }
  close(p[0]);
  close(p[1]);
  while(wait(&stat)){
      if( errno == ECHILD)
        break;
  }
  close(p[0]);
  close(p[1]);

  fflush(stdout);
  fflush(stdin);

  return 1;
}

//execute single command
int execute_command(char **param){
  pid_t pid, wpid;
    int stat;
    pid = fork();
    if (pid == 0) {
      char *prog_path = getenv(param[0]);
       if (!prog_path) { prog_path = param[0]; }
       if (execvp(prog_path, param) == -1) {
          perror("mysh");
          exit(EXIT_FAILURE);
       }
    }
   else if (pid < 0) {
       perror("mysh");//Error in forking
    }
   else {
    // Parent process
       do {
          wpid = waitpid(pid, &stat, WUNTRACED);
       } while (!WIFEXITED(stat) && !WIFSIGNALED(stat));
    }

   return 1;
}

//parses programs using delimiter '|'  or '$('
char ***parse_programs(char **param){
  int j =0, k = 0 ;  // positions
  int b_size = 100;
  size_t program_size = 20; // max programs
  char ***programs;
  programs = malloc(program_size * sizeof(char**));
   if (!programs) {
    fprintf(stderr, "mysh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  programs[j] = malloc(b_size * sizeof(char*));
  if(!programs[j]){
    fprintf(stderr, "mysh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  char *tok; // pointer to token
  char **args = param;
  while(*args != NULL){
    //printf("args is %s\n", *args);
    if(strchr(*args, '|') || strstr(*args, "$(") || strchr(*args, ')')){
      char *ptr;
      if((ptr = (strstr(*args, "$(")))){
        *ptr = '|';
        memmove(ptr+1, ptr+2, strlen(ptr));
      }
       if((ptr = (strchr(*args, ')')))){
          *ptr = '|';
        }
      if(*args[0] == '|'){  //case where '|' is first char
        programs[j][k] = NULL;
        j++;
        k = 0;
        programs[j] = malloc(b_size * sizeof(char*)); //allocating space for found program
        tok = strtok_r(*args, "|", &ptr);
        while(tok != NULL){ //case where char arr contains multiple '|' char
          programs[j][k++] = tok; //assigning tok program to program j
          tok = strtok_r(NULL, "|", &ptr);
          if(tok != NULL){
            j++;
            k = 0;
            programs[j] = malloc(b_size * sizeof(char*));
            programs[j][k++] = tok;  //assigning tok program  to program j
          }
        }
      }
      else{ //case where '|' is not first char
          tok = strtok_r(*args, "|", &ptr);
          programs[j][k++] = tok;
          programs[j][k] = NULL;
          j++;
          k = 0;
          programs[j] = malloc(b_size * sizeof(char*)); //allocating space for found program
          tok = strtok_r(NULL, "|", &ptr);
          while(tok != NULL){  //case where char arr contains multiple '|' char
            programs[j][k++] = tok;
            tok = strtok_r(NULL, "|", &ptr);
            if(tok != NULL){
              j++;
              k = 0;
              programs[j] = malloc(b_size * sizeof(char*));
              programs[j][k++] = tok;
            }
          }
      }
      //increment arg
      args++;
    }
    else{
      programs[j][k++] = *args; //assigns program and attr to program j
      args++;
    }
  }
  programs[j++][k] = NULL;
  programs[j] = NULL;
  return programs;
}

void free_programs(char ***programs){
  int j = 0;
  while(programs[j] !=NULL){
    free(programs[j]);
    programs[j++] = NULL;
  }
  free(programs);
}

// mysh_launch(): It takes null terminated lists of arguments (param) and launchs
// a program and wait for it to terminate and finally returns 1, to continue execution.
int mysh_launch(char **param)
{
  //checking for pipe character
  const char pipe = '|';
  const char *subshell = "$(";
  char **c = param;
  int found_mult_comm;
  int command_type;
  while(*c){
    if(strchr(*c, pipe) || strstr(*c, subshell)){
      if(strstr(*c, subshell)){command_type = 0;}
      else{command_type = 1;}
      found_mult_comm = 1;
      break;
    }
    c++;
  }
  //executing pipe_commands
  if(found_mult_comm){
    int returncode;
    char ***programs = parse_programs(param);

    if(command_type){
      returncode = execute_pipe_command(programs);
    }
    else{
      returncode = execute_subshell_command(programs);
    }

    free_programs(programs);
    return returncode;
  }
    //executing single command
   return  execute_command(param);


}

// mysh_exec(): by taking parameters of NUll terminated list of arguments it
// executes shell builtin or launch program and return 1 if the shell should
// continue running, or 0 if it should terminate.
int mysh_exec(char **param)
{
  int i;
  if (param[0] == NULL) {
    return 1; // return for an empty command entered
  }
  for(i = 0 ; i < 3; i++){
    if (strcmp(param[0], cmd_built[i]) == 0){
         return (*func_built[i])(param);
       }
    }
  return mysh_launch(param);
}

// mysh_readLine(): Reads a line of input string from stdin and
// returns the string
char *mysh_readLine(void)
{
  char *stringLine = NULL;
  size_t b_size = 0;
  getline(&stringLine, &b_size, stdin);
  return stringLine;
}



// mysh_splitLine(): splits a string of argument(param) into tokens
// and returns Null-terminated array of tokens.
#define T_DELIM " \t\r\n\a" //defining delimetters
char **mysh_splitLine(char *stringLine)
{
  int i = 0;  // position
  int b_size = 100;
  char *tok; // pointer to token
  char **ptr_tok; // pointer to array of tokens
  ptr_tok = malloc(b_size * sizeof(char*)); //dynamically allocating memory
  if (!ptr_tok) {
    fprintf(stderr, "mysh: allocation error\n");
    exit(EXIT_FAILURE);
  }
  tok = strtok(stringLine, T_DELIM);
  while (tok != NULL) {
    ptr_tok[i] = tok;
    i++;
    if (i >= b_size) {
      b_size += 100;
      ptr_tok = realloc(ptr_tok, b_size * sizeof(char*));
      if (!ptr_tok) {
        fprintf(stderr, "mysh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
    tok = strtok(NULL, T_DELIM);
  }
  ptr_tok[i] = NULL;
  return ptr_tok;
}


//define environment variables
//myshv4 implementation
void set_env_variables(){
    char buf[100];
   realpath("myls",buf);
   setenv("myls",buf, 0);
   realpath("mycat", buf);
   setenv("mycat", buf, 0);

}

//Main/drive function
int main(int argc, char **argv)
{

   //set up external program path
  //myshv4 implemenation
   set_env_variables();

   char *s_line;
   char **arg;
   int status;

   // Loop to read a string of command, sparse the string, and
   // execute the command
   do {
      printf("mysh$ ");
      s_line = mysh_readLine();
      arg = mysh_splitLine(s_line);
      status = mysh_exec(arg);
      free(s_line);
      free(arg);
    } while (status);

   return EXIT_SUCCESS;
}

