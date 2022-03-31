#include <stdio.h>
#include <string.h>

#define MAXLINE 500
#define MAXLINES 100

char* inputFile;
int monitorTime;
int NITER;
int debug = 1; // To see debug messages debug value = 1 else debug value = 0
char inputlines[MAXLINES][MAXLINE];
int num_lines = 0;

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
		if (strlen(line) > 1){
			if (line[0] != '#'){
				strcpy(inputlines[num_lines], line);
				num_lines++;
				if (debug)	printf("length:%lu %s", strlen(line), inputlines[num_lines-1]);
			}
		}
		memset(line, 0, MAXLINE);
	}

	fclose(file);

	return 0;
}