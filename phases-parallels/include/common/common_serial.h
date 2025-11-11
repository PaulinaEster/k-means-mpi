#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define CPU_INFO_PATH "/proc/cpuinfo"
#define NO_CPU_INFO "No info"
#define PROFILING_SLOTS 64

double start[PROFILING_SLOTS];
double elapsed[PROFILING_SLOTS];
int debug_flag;
int timer_flag;
char timer_string[2048];
char checksum_string[8192];
char cpu_name[256];

void timer_write_time(double* t);
double timer_elapsed_time();
void timer_clear(int n);
void timer_start(int n);
void timer_stop(int n);
double timer_read(int n);
void activate_debug_flag();
void activate_timer_flag();
void get_cpu_model();
void execution_report(char* application_name, char* workload, double execution_time, int passed_verification);
void setup_common();

void timer_write_time(double* t){
	static int sec = -1;
	struct timeval tv;
	gettimeofday(&tv, 0);
	if (sec < 0) sec = tv.tv_sec;
	*t = (tv.tv_sec - sec) + 1.0e-6*tv.tv_usec;
}

double timer_elapsed_time(){
	double t;
	timer_write_time(&t);
	return(t);
}

void timer_clear(int n){
	elapsed[n] = 0.0;
}

void timer_start(int n){
	start[n] = timer_elapsed_time();
}

void timer_stop(int n){
	double t, now;
	now = timer_elapsed_time();
	t = now - start[n];
	elapsed[n] += t;
}

double timer_read(int n){
	return(elapsed[n]);
}

void activate_debug_flag(){
	/* the debug flag is disabled by default */
	debug_flag = 0;

	/* activating the debug flag through a macro */
#if defined(DEBUG)
	debug_flag = 1;	
#endif
}

void activate_timer_flag(){
	/* the timer flag is disabled by default */
	timer_flag = 0;

	/* activating the timer flag through a macro */
#if defined(TIMER)
	timer_flag = 1;	
#endif
}

void get_cpu_model(){
	FILE* file = fopen((char*)CPU_INFO_PATH, "r");
	char* error = (char*)NO_CPU_INFO;
	char* line = NULL;
	char* cpu_model;
	size_t n = 0;	
	if(file == NULL){
		strcpy(cpu_name, error);
		return;
	}		
	while(getline(&line, &n, file) > 0){
		if(strstr(line, "model name")){
			cpu_model=line;
			while(*cpu_model != ':'){
				cpu_model++;
			} cpu_model++;
			while(*cpu_model == ' '){
				cpu_model++;
			}
			strtok(cpu_model, "\n");
			fclose(file);
			strcpy(cpu_name, cpu_model);
			return;
		}
	}
	fclose(file);
	strcpy(cpu_name, error);
}

void execution_report(char* application_name, char* workload, double execution_time, int passed_verification){
	printf("----------------------------------------------------------------------------\n");
	printf(" %s:\n", application_name);
	printf("\n");
	printf(" Workload                  =     %s\n", workload);	
	printf(" Execution time in seconds =     %f\n", execution_time);
	if(passed_verification == 1){
		printf(" Correctness verification  =     SUCCESSFUL\n");
	}
	else{
		printf(" Correctness verification  =     UNSUCCESSFUL\n");
	}
	printf("----------------------------------------------------------------------------\n");
	printf(" Hardware:\n");
	printf("\n");
	printf(" CPU                       =     %s\n", cpu_name);
	printf("----------------------------------------------------------------------------\n");
	printf(" Flags:\n");
	printf("\n");
	if(debug_flag){printf(" Debug flag enabled\n");}else{printf(" Debug flag disabled\n");}
	if(timer_flag){printf(" Timer flag enabled\n");}else{printf(" Timer flag disabled\n");}
	printf("----------------------------------------------------------------------------\n");	
	printf(" Correctness:\n");
	printf("\n");
	printf("%s\n", checksum_string);
	printf("----------------------------------------------------------------------------\n");
	if(timer_flag){
		printf(" Timers:\n");
		printf("\n");
		printf("%s\n", timer_string);
		printf("----------------------------------------------------------------------------\n");
	}
}

void setup_common(){
	activate_debug_flag();
	activate_timer_flag();

	/* clear timers */
	for(int i=0; i<PROFILING_SLOTS; i++){
		timer_clear(i);
	}	

	get_cpu_model();
}
