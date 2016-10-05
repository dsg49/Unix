/* File:   mycd_pwd.h
  Description: Implementation of mysh_cd and mysh_pwdfor functions
*/

#include <unistd.h>
#include <stdio.h>

// change directory function that takes pointers to list of argumetnts where param[0] is "mycd" and
// param[1] is target directory. It returns 1 to continue.
int mysh_cd (char **param){
    if (param[1] == NULL) 
       fprintf(stderr, "mycd expects an argument \n");
    else
       if (chdir(param[1]) != 0)  perror("mysh");
   return 1;
}

//mysh_pwd: Gets the path to the current working directory using function getcwd() and then prints 
//out the string to the terminal.  
int  mysh_pwd(char **param){
   size_t size = 100;
   char currWorkDir[size];
   if(getcwd(currWorkDir, sizeof(currWorkDir)) == NULL){
     return -1;
    perror("mysh");
   }
   printf("%s\n",currWorkDir);
   return 1;
}


