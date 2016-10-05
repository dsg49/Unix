/*
 * Implements the cat command in Unix using C.
 * Handles the following cases:
 * mycat foo.txt
 * mycat < foo.txt
 * mycat < foo.txt > foo2.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#define MAX_FILE_CHARS 255

//Function prototypes
void redirectCat(char *, char *);
void displayFile(char *);

/*
 * Reads command entered into the command line and calls
 * the appropriate functions based on redirection type.
 */
int main(int argc, char *argv[])
{
    char fileName1[MAX_FILE_CHARS];
    char fileName2[MAX_FILE_CHARS];
    int numArgs = argc;
    char readChar;
    int outRedirectFlag = 0;
    int inRedirectFlag = 0;
    int count;
    
    //Check to ensure a file argument has been entered with the command.
    //Since the program does not implement just mycat being entered.
    if (numArgs < 3)
    {
        printf("The command 'mycat' requires and arugment.\n");
        exit(0);
    }
    
    //Read command to determine if redirection is needed.
    //And if needed, what kind of redirection.
    if(numArgs > 1)
    {
        for (count = 2; count < numArgs; count++)
        {
            readChar = *argv[count];
            
            //Determine what kind of redirection is needed.
            switch(readChar)
            {
                case '>':
                    outRedirectFlag = 1;
                    break;
                case '<':
                    inRedirectFlag = 1;
                    break;
                default:
                    if(0 == strlen(fileName1))
                        strcpy(fileName1, argv[count]);
                    else
                        strcpy(fileName2, argv[count]);
            }
        }
    }
    else
    {
        printf("The command mycat requires an argument.\n");
        exit(0);
    }
    
    //Check to ensure a file argument has been entered when redirection is present.
    if ((outRedirectFlag == 1) || (inRedirectFlag == 1))
    {
        if ((0 == strlen(fileName1)) && (0 == strlen(fileName2)))
        {
            printf("An argument is required.\n");
            exit(0);
        }
    }
    
    //Call the correct function based on input from command line.
    if ((1 == outRedirectFlag) && (1 == inRedirectFlag))    //Command mycat < foo.txt > foo2.txt
    {
        redirectCat(fileName1, fileName2);
        displayFile(fileName2);     //Display file once contents have been copied.
    }
    else if((0 == outRedirectFlag) && (0 == inRedirectFlag))    //Command mycat foo.txt
        displayFile(fileName1);
    else if(1 == outRedirectFlag)       //For now can't do mycat > foo.txt
        printf("Command not applicable.\n");
    else if(1 == inRedirectFlag)        //Command mycat < foo.txt is entered
        displayFile(fileName1);
    else
        printf("Command not found");
    
    //Quit the program.
    exit(0);
}

/*
 * If the command requires redirection, the function is called to print 
 * the contents of the first file to the second file. Handles the
 * following commands: cat foo.txt > foo2.txt and 
 * cat < foo.txt > foo2.txt (Both perform the same operation).
 */
void redirectCat(char *fileName1, char *fileName2)
{
    FILE *origFile;
    FILE *fileCopy;
    char origCH;
    char newCH;
    
    //Open the files.
    origFile = fopen(fileName1, "r");
    fileCopy = fopen(fileName2, "w");   //Write to the second file entered. Overwrites if file already exists.
    
    printf("The files have been opened!\n");
    
    //Check to make sure first file entered exists.
    if(origFile == NULL)
    {
        printf("The first file entered does no exist\n");
        exit(0);
    }
    
    //Print the contents of the first file to the second file.
    while(origCH != EOF)
    {
       origCH = fgetc(origFile);
       fputc(origCH, fileCopy);
    }
    
    //Close the files.
    fclose(origFile);
    fclose(fileCopy);
}

/*
 * Prints the content of the file passed as a parameter
 * to the screen. Handles the commands: cat foo.txt
 * and cat < foo.txt (Both perform the same operation).
 */
void displayFile(char *fileName)
{
    FILE *fp;
    char ch;
    
    fp = fopen(fileName, "r");
    
    //Check to make sure file exists.
    if(fp == NULL)
    {
        printf("The file %s does not exist.\n", fileName);
        exit(0);
    }
    
    //Test to see if file was copied successfully and correctly.
    while((ch = fgetc(fp))!= EOF)
    {
        printf("%c", ch);
    }
    
    //Close the file
    fclose(fp);
}

    



