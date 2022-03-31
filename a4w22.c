#include <stdio.h>
#include <string.h>

#define MAXLINE 500
#define MAXLINES 100
#define MAXJOBS 30

int monitorTime;
int NITER;
int debug = 1; // To see debug messages debug value = 1 else debug value = 0
char inputlines[MAXLINES][MAXLINE];
int linesize[MAXLINES];
int num_lines = 0;
char resource_line[MAXLINE];
char job_lines[MAXJOBS][MAXLINE];
int num_jobs = 0;

int main(int argv, char* argc[]){

	if (argv != 4){
		printf("a4w22 require three arguments: usage ./a4w22 inputfile monitorTime NITER\n");
		return 0;
	}

	if (debug)	printf("Command line arguments:\n%s %s %s %s\n", argc[0], argc[1], argc[2], argc[3]);

	sscanf(argc[2], "%d", &monitorTime);
	if (debug) printf("MonitorTime: %d\n", monitorTime);
	sscanf(argc[3], "%d", &NITER);
	if (debug) printf("NITER: %d\n", NITER);

	// Opening inputfile for read
	FILE* file = fopen(argc[1], "r");

	if (!file){ printf("Unable to open: %s\n", argc[1]); return 0;}

	char line[MAXLINE];

	memset(line, 0, MAXLINE);
	while(fgets(line, MAXLINE, file)){
		if (strlen(line) > 1){
			if (line[0] != '#'){
				if (line[0] == 'j' && line[1] == 'o' && line[2] == 'b')	strcpy(job_lines[num_jobs++], line);
				if (line[0] == 'r' && line[1] == 'e' && line[2] == 's') strcpy(resource_line, line);
				strcpy(inputlines[num_lines], line);
				linesize[num_lines] = (int) strlen(inputlines[num_lines]);
				num_lines++;
				if (debug)	printf("length:%d %s", linesize[num_lines-1], inputlines[num_lines-1]);
			}
		}
		memset(line, 0, MAXLINE);
	}

	if (debug){
		printf("Jobs:\n");
		for (int i = 0; i < num_jobs; i++) printf("%s", job_lines[i]);
		printf("Resources:\n");
		printf("%s", resource_line);
	}

	fclose(file);

	return 0;
}