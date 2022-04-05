#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

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

struct res{
	char res_names[MAXRESOURCES][MAXLINE];
	int res_vals[MAXRESOURCES];
	int available[MAXRESOURCES];
	int holds[MAXRESOURCES];
	int res_max;
	pthread_mutex_t f_lock;
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

struct res resources;

int counters[MAXJOBS] = {0};
// 0: New || 1: WAIT || 2: RUN || 3: IDLE
int job_status[MAXJOBS] = {3};
int WAIT = 1, RUN = 2, IDLE = 3;

pthread_t thread_id[MAXJOBS];

struct timeval start, current;

int debug = 1; // To see debug messages debug value = 1 else debug value = 0

unsigned long timedifference_msec(struct timeval start, struct timeval current){
	return (current.tv_sec - start.tv_sec) * 1000000 + current.tv_usec - start.tv_usec;
}

// Job Thread Function
void* thread_function(void* arg){
	int index = *(int *) arg;
	if (debug) printf("Inside Thread %d\n", index);
	pthread_exit(NULL);
}

// Monitor Thread Function
void* monitor_funtion(void* arg){
	int index = *(int *) arg;
	if (debug) printf("Inside Monitor Thread\n");
	// TODO: Need to be careful here
	return NULL;
}

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

	gettimeofday(&start, 0);

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
		}
		printf("Resources:\n");
		printf("%s\n", resource_line);
	}

	// Processing each job line
	for (int i = 0; i < num_jobs; i++){
		if (debug) printf("Job number: %d\n", i);
		int num_words = 0;
		jobs[i].num_res = 0;
		char* token = strtok(job_lines[i], " ");
		char job_res[MAXRESOURCES][MAXLINE]; int counter = 0, temp = 0;
		while( token != NULL ){
			if (num_words == 1) {
				strcpy(jobs[i].name, token);
				if (debug) printf("name: %s\n", jobs[i].name);
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
			}
			token = strtok(NULL, " ");
			num_words++;
		}
		for (int j = 0; j < counter; j++){
			char* ne_token  = strtok(job_res[j], ":");
			strcpy(jobs[i].res_names[jobs[i].num_res], ne_token);
			ne_token = strtok(NULL, ":");
			int temp1 = 0; sscanf(ne_token, "%d", &temp1);
			jobs[i].res_vals[jobs[i].num_res++] = temp1;
			if (debug) printf("%s:%d\n", jobs[i].res_names[jobs[i].num_res-1], jobs[i].res_vals[jobs[i].num_res-1]);
		}
	}

	char* token = strtok(resource_line, " ");
	char temp_res[MAXJOBS][MAXLINE]; int counter = 0;
	if (debug) printf("%s\n", token);
	token = strtok(NULL, " "); 
	while(token != NULL){
		strcpy(temp_res[counter], token);
		if (debug) printf("%s\n", temp_res[counter]);
		counter++;
		token = strtok(NULL, " ");
	}
	for (int j = 0; j < counter; j++){
		char* ne_token = strtok(temp_res[j], ":");
		strcpy(resources.res_names[resources.res_max], ne_token);
		ne_token = strtok(NULL, ":");
		int temp1 = 0; sscanf(ne_token, "%d", &temp1);
		resources.res_vals[resources.res_max] = resources.available[resources.res_max] = temp1;
		resources.holds[resources.res_max] = 0;
		int t1 = resources.res_max;
		resources.res_max++;
		if (debug){
			printf("Name: %s, Value: %d, available: %d, holds: %d\n", resources.res_names[t1], resources.res_vals[t1], resources.available[t1], resources.holds[t1]);
		}
	}
	if (debug) printf("Max resources: %d\n", resources.res_max);

	int args[MAXJOBS];

	// Assigning a thread to each task
	for (int i = 0; i < num_jobs; i++){
		args[i] = i;
		int err = pthread_create(&thread_id[i], NULL, thread_function, &args[i]);
		if (err != 0) printf("Failed to create a thread %d\n", i);
	}

	// TODO: Need to make sure when the monitor thread is printing thread status is not being modified
	// Creating Monitor Thread
	args[num_jobs] = num_jobs;
	int err = pthread_create(&thread_id[num_jobs], NULL, monitor_funtion, &args[num_jobs]);
	if (err != 0) printf("Failed to create monitor thread\n");

	for (int i = 0; i < num_jobs; i++){
		err = pthread_join(thread_id[i], NULL);
		if (err != 0) printf("Can't join with thread %d\n", i);
		if (debug) printf("Thread %d exited\n", i);
	}

	gettimeofday(&current, 0);
	printf("Running time= %lu msec\n", timedifference_msec(start, current));
	return 0;
}