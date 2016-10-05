all: mysh myls mycat 

mysh: myshv9.o 
	gcc -Wall -o mysh myshv9.c
myls: myls.o
	gcc -Wall -o myls myls.c
mycat: mycat.o
	gcc -Wall -o mycat mycat.c

clean:
	rm *.o
