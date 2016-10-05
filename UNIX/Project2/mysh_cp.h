/* Implements the cp command in Unix using C.
 * Supports following listed command syntax:
 *
 * // Copies the contents of a_file to b_file.
 * // If b_file doesn't exist, it is created.
 * // Or if b_file does exist, it is overwritten
 * // with a_file's contents.
 * mycp a_file b_file
 *
 * // Copies the contents of a_dir to b_dir. Final
 * // outcome is a directory "a_dir" in b_dir. If
 * // directory b_dir doesn't exist, it is created.
 * mycp -R a_dir b_dir
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

int copySuccess;

/*
* Displays correct format of command
*/
void printUsage()
{
   printf("mycp command usage is as follows:\n");
   printf("mycp [-R] source target\n");
}

/*
* Checks if correct option given
* Returns: 1 if correct option, else -1 in error
*/
int checkOption(const char* arg)
{
   char option[] = "-R";

   if (strcmp(option, arg) == 0)
      return 1;
   else
      return -1;
}

/*
 * Checks if is directory
 * Returns: 0 if file, 1 for directory, -1 if an
 * error occurs when checking for status
 */
int checkIfDirectory(const char* arg)
{
   struct stat buffer;

   if (stat(arg, &buffer) == -1)
   {
      perror("error reading status of path\n");
      return -1;
   }

   if (S_ISREG(buffer.st_mode))
      return 0;

   if (S_ISDIR(buffer.st_mode))
      return 1;
}


/*
 * Copies a file
 * Returns: 1 for successful copy,
            0 for failed copy
 */
int copyFile(const char* a_Path, const char* b_Path)
{
   int source;
   int target;
   int in;

   struct stat oldMode;

   // Read status of a_Path
   if (stat(a_Path, &oldMode) == -1)
   {
      perror("stat error for source");
      return 0;
   }

   // Open source
   if ( (source = open(a_Path, O_RDONLY)) == -1)
   {
      perror("open error for source");
      return 0;
   }

   // Create new file at b_Path
   if ( (target = creat(b_Path, oldMode.st_mode)) == -1)
   {
      perror("error - can't create target");
      close(source);
      return 0;
   }

   // Change file permissions to oldMode
   if (fchmod(target, oldMode.st_mode) == -1)
   {
      printf("error file permissions change");
      return 0;
   }

   char buffer[PATH_MAX];

   // Read and write from source to target
   while ( (in = read(source, buffer, PATH_MAX)) > 0)
   {
      if (write(target, buffer, in) != in)
      {
         perror("error writing to target");
         close(source);
         close(target);
         return 0;
      }
   }

   if (close(source) != 0)
   {
      perror("error closing source");
      return 0;
   }
   if (close(target) != 0)
   {
      perror("error closing target");   
      return 0;
   }
 
   return 1;
}

/*
 * Get the file name and store in name[]
 */
void getFileName(char* fSource, char* name)
{
   int i;
   int j = strlen(fSource);
   int k = 0;

   for (i = j - 1; i >= 0; i--)
   {
      if (fSource[i] == '/')
         break;
   }

   for (i++; i < j; i++)
   {
      name[k] = fSource[i];
      k++;
   }

   // Add null terminator to name[]
   name[k] = '\0';
}

/*
 * Copies files and sub-folders from a_Path to b_Path
 * Returns: 1 if successful, 0 if error occurs
 */
void copyDir(const char* a_Path, const char* b_Path)
{
   char source[PATH_MAX+1]; //store path of a_Path
   char target[PATH_MAX+1]; // store path of b_Path
   char fName[PATH_MAX+1]; // store file name

   DIR* dirPtr;
   struct dirent* direntPtr;
   
   // Check whether source is a directory
   int check = checkIfDirectory(a_Path);

   strcpy(source, a_Path);
   strcpy(target, b_Path);

   if (check == 0) // Source is a reg file
   {
      getFileName(source, fName);
//printf("Target is %s\n", target);
      strcat(target, "/");
//printf("Target is %s\n", target);
      strcat(target, fName);
//printf("Target is %s\n", target);
      copyFile(source, target);
      return;
   }
   else if (check == 1) // Source is a dir
   {
      getFileName(source, fName);
      strcat(target, "/");
      strcat(target, fName);

      // If file name is either current dir
      // or parent dir, return
      if (strcmp(fName, ".")== 0 
          || strcmp(fName, "..")== 0)
         return;

      struct stat oldMode;

      if (stat(a_Path, &oldMode) == -1)
      {
         perror("stat error");
         return;
      }

      // Create the directory copy with appropriate permissions
      mkdir(target, oldMode.st_mode);
      chmod(target, oldMode.st_mode);

      dirPtr = opendir(source);

      while (1)
      {
         direntPtr = readdir(dirPtr);

         if (direntPtr == NULL)
            break;
         else
         {
            strcpy(source, a_Path);
            strcat(source, "/");
            strcat(source, direntPtr->d_name);
            copyDir(source, target);
         }
      }

      copySuccess = 0;
      closedir(dirPtr);
      return;
   }
   else
   {
      rmdir(b_Path); // remove created target dir
      return;
   }
}

/*
 * Primary function for cp command.
 *
 */
int mycp(char** argv)
{
   int argc = 0;

   argc = sizeof(argv);

//printf("argv0 is %s\n", argv[0]);
//printf("arg1 is %s\n", argv[1]);
//printf("arg2 is %s\n", argv[2]);
//printf("arg3 is %s\n", argv[3]);
//printf("argc is %d\n", argc);

   // Copies from a_file to b_file
   if (argc == 4 && argv[3] == NULL)
      copyFile(argv[1], argv[2]);
   else if (argc == 4 && checkOption(argv[1]) == 1)
   { // Copies from dir1 to dir2

      // Creates target if doesn't exist prior
      struct stat st;

      if (stat(argv[3], &st) == -1)
         mkdir(argv[3], 0775);

      copyDir(argv[2], argv[3]);

      // If copyDir() fails, remove target dir
      if (copySuccess != 0)
      {
         rmdir(argv[3]);
         return 1;
      }
   }
   else // Error in command format
   {
      printUsage();
      return 1;
   }

   return 1;
}
