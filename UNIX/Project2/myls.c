#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

void myls(int options, int argnum, char **file_name){
     struct dirent **namelist;
        int n;
        //checking if not directory
        if((n = scandir(file_name[argnum], &namelist, 0, alphasort)) < 0 ){
            //checking if file
            if( access(file_name[argnum], F_OK) != -1){
                //checking for option -l
                if((options & 1) != 0){       
                    struct stat statbuf;
                    stat(file_name[argnum], &statbuf);
                    char date_mod[20];
                    strftime(date_mod, 20, "%b %d %H:%M", localtime(&(statbuf.st_mtime)));

                    printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                    printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                    printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                    printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
                    printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                    printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                    printf("%2ld ", (long)statbuf.st_nlink);
                    printf("%5s ", getpwuid(statbuf.st_uid)->pw_name);
                    printf("%6s ", getgrgid(statbuf.st_gid)->gr_name);
                    printf("%6lld ", (long long)statbuf.st_size);
                    printf("%10s ", date_mod);
                    printf("%s\n", file_name[argnum]);
                }
                else {
                    printf("%s  ", file_name[argnum]);
                    if(argnum == 0)
                        printf("\n");
                }
               
            }
            else{
                printf("myls: cannot access %s: No such file or directory\n", file_name[argnum]);
                exit(1);
            }
        }
        else{
            int i;
            //iterating through files in directory
            for( i = 0; i < n; i++){
                //compare if file begins with '.' unless the option -a is set 
                if(((namelist[i]->d_name)[0] != '.' ) || (options & 2) != 0 ){
                    //check option for  option -l 
                    if((options & 1) != 0){
                        struct stat statbuf;
                        stat(namelist[i]->d_name, &statbuf);
                        char date_mod[20];
                        strftime(date_mod, 20, "%b %d %H:%M", localtime(&(statbuf.st_mtime)));

                        printf( (S_ISDIR(statbuf.st_mode)) ? "d" : "-");
                        printf( (statbuf.st_mode & S_IRUSR) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWUSR) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXUSR) ? "x" : "-");
                        printf( (statbuf.st_mode & S_IRGRP) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWGRP) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXGRP) ? "x" : "-");
                        printf( (statbuf.st_mode & S_IROTH) ? "r" : "-");
                        printf( (statbuf.st_mode & S_IWOTH) ? "w" : "-");
                        printf( (statbuf.st_mode & S_IXOTH) ? "x" : "-");
                        printf("%2ld ", (long)statbuf.st_nlink);
                        printf("%5s ", getpwuid(statbuf.st_uid)->pw_name);
                        printf("%6s ", getgrgid(statbuf.st_gid)->gr_name);
                        printf("%6lld ", (long long)statbuf.st_size);
                        printf("%10s ", date_mod);
                        printf("%s\n", namelist[i]->d_name);
                    }
                    else {
                        printf("%s  ", namelist[i]->d_name);
                        if( i == (n-1))
                            printf ("\n");
                    }
                
                    free(namelist[i]);
                }
            }

            free(namelist);
        }
        //recursive call to next file in character array
        if(argnum > 0)
            myls(options, --argnum, file_name);

}

int main(int argc, char *argv[]){
    
    int c;
    extern int optind;
    extern char *optarg;

    int lflag = 0;
    int aflag = 0;
    int options = 0;
   
    while((c = getopt(argc, argv, "la")) > 0){
      
        switch(c){

            case 'l':
                lflag = 1;
                options |= lflag;
                break;
            case 'a':
                aflag = 2;
                options |= aflag;
                break;
            case '?':
                exit(1);
                break;
        }
    }

    //no file given
    if( (argc - optind -1) < 0){
        char *file[] = {"."}; //default current directory
        myls(options, 0, file);
    }
    else //at least one file given
        myls(options, (argc - optind - 1), &argv[optind]);

    exit(0);
}