#include <stdio.h>
#include <string.h>

#define MAXLINE 500
#define MAXLINES 100
#define MAXJOBS 30
#define MAXRESOURCES 10

struct job{
	char name[MAXLINE];
	int busyTime;
	int idleTime;
	char res_names[MAXRESOURCES][MAXLINE];
	int res_vals[MAXRESOURCES];
	int num_res;
};

int monitorTime;
int NITER;
char inputlines[MAXLINES][MAXLINE];
int linesize[MAXLINES];
int num_lines = 0;
char resource_line[MAXLINE];
char job_lines[MAXJOBS][MAXLINE];
int num_jobs = 0;
struct job jobs[MAXJOBS];

int debug = 1; // To see debug messages debug value = 1 else debug value = 0

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
			}
		}
		memset(line, 0, MAXLINE);
	}
	fclose(file);

	if (debug){
		printf("Jobs:\n");
		for (int i = 0; i < num_jobs; i++){
			printf("%s\n", job_lines[i]);
			// char * token = strtok(job_lines[i], " ");

			// while( token != NULL){
			// 	printf("%s \n", token);
			// 	token = strtok(NULL, " ");
			// }
		}
		printf("Resources:\n");
		printf("%s\n", resource_line);
		// char* token = strtok(resource_line, " ");
		// while (token != NULL){
		// 	printf("%s \n", token);
		// 	token = strtok(NULL, " ");
		// }
	}

	// Processing each job line
	for (int i = 0; i < num_jobs; i++){
		printf("Job number: %d\n", i);
		int num_words = 0;
		jobs[i].num_res = 0;
		char* token = strtok(job_lines[i], " ");
		char job_res[MAXRESOURCES][MAXLINE]; int counter = 0, temp = 0;
		while( token != NULL ){
			if (num_words == 1) {
				strcpy(jobs[i].name, token);
				if (debug) printf("name: %s\n", jobs[i].name);
				// jobs[i].name = token;
			}
			if (num_words == 2){
				temp = 0;
				sscanf(token, "%d", &temp);
				jobs[i].busyTime = temp;
				if (debug) printf("busyTime: %d\n", temp);
			}
			if (num_words == 3){
				temp = 0;
				sscanf(token, "%d", &temp);
				jobs[i].idleTime = temp;
				if (debug) printf("idleTime: %d\n", temp);
			}
			if (num_words >= 4){
				strcpy(job_res[counter], token);
				counter++;
				// if (debug) printf("%s \n", job_res[counter-1]);
			}
			token = strtok(NULL, " ");
			num_words++;
		}
		for (int j = 0; j < counter; j++){
			char* ne_token  = strtok(job_res[j], ":");
			// jobs[i].res_names[jobs[i].num_res] = ne_token;
			strcpy(jobs[i].res_names[jobs[i].num_res], ne_token);
			ne_token = strtok(NULL, ":");
			int temp1 = 0; sscanf(ne_token, "%d", &temp1);
			jobs[i].res_vals[jobs[i].num_res++] = temp1;
			if (debug) printf("%s:%d\n", jobs[i].res_names[jobs[i].num_res-1], jobs[i].res_vals[jobs[i].num_res-1]);
		}
	}
	return 0;
}


			// switch(num_words){
			// 	case 1:
			// 		jobs[i].name = token;
			// 		break;
			// 	case 2:
			// 		temp = 0;
			// 		sscanf(token, "%d", &temp);
			// 		jobs[i].busyTime = temp;
			// 		if (debug) printf("busyTime: %d\n", temp);
			// 		break;
			// 	case 3:
			// 		temp = 0;
			// 		sscanf(token, "%d", &temp);
			// 		jobs[i].idleTime = temp;
			// 		if (debug) printf("idleTime: %d\n", temp);
			// 		break;
			// 	default:
			// 		// job_res[counter++] = token;
			// 		strcpy(job_res[counter], token);
			// 		counter++;
			// 		if (debug) printf("%s \n", job_res[counter-1]);
			// 		break;
			// }