#include <stdio.h>
#include <string.h>

#define MAXLINE 500

char* inputFile;
int monitorTime;
int NITER;
int debug = 1; // To see debug messages debug value = 1 else debug value = 0

int main(int argv, char* argc[]){

	if (argv != 4){
		printf("a4w22 require three arguments: usage ./a4w22 inputfile monitorTime NITER\n");
		return 0;
	}

	if (debug)	printf("Command line arguments:\n%s %s %s %s\n", argc[0], argc[1], argc[2], argc[3]);

	// Opening inputfile for read
	FILE* file = fopen(argc[1], "r");

	if (!file){ printf("Unable to open: %s\n", argc[1]); return 0;}

	char line[MAXLINE];

	memset(line, 0, MAXLINE);
	while(fgets(line, MAXLINE, file)){
		printf("%s\n", line);
		memset(line, 0, MAXLINE);
	}

	fclose(file);

	return 0;
}