#include "../common/common_serial.h"

#if defined(WORKLOAD_A)
#define WORKLOAD "A"
#define N_POINTS 10
#define N_MEANS 2
#define INTERVAL 25
#elif defined(WORKLOAD_B)
#define WORKLOAD "B"
#define N_POINTS 1000
#define N_MEANS 10
#define INTERVAL 50
#elif defined(WORKLOAD_C)
#define WORKLOAD "C"
#define N_POINTS 10000
#define N_MEANS 250
#define INTERVAL 100
#elif defined(WORKLOAD_D)
#define WORKLOAD "D"
#define N_POINTS 100000
#define N_MEANS 1000
#define INTERVAL 200
#elif defined(WORKLOAD_E)
#define WORKLOAD "E"
#define N_POINTS 250000
#define N_MEANS 2500
#define INTERVAL 1000
#elif defined(WORKLOAD_F)
#define WORKLOAD "F"
#define N_POINTS 500000
#define N_MEANS 5000
#define INTERVAL 5000
#elif defined(WORKLOAD_G)
#define WORKLOAD "G"
#define N_POINTS 1000000
#define N_MEANS 10000
#define INTERVAL 25000
#elif defined(WORKLOAD_H)
#define WORKLOAD "H"
#define N_POINTS 5000000
#define N_MEANS 50000
#define INTERVAL 50000
#else
#define WORKLOAD_A
#define WORKLOAD "A"
#define N_POINTS 10
#define N_MEANS 2
#define INTERVAL 25
#endif

// relative tolerance for floating-point comparison
#define RELATIVE_TOLERANCE 1e-10  
// relative tolerance: allows small rounding differences in floating-point operations
// chosen as 1e-10 because double-precision (IEEE-754) arithmetic can differ by ~1e-15 per operation,
// and summing many terms (like in pi calculation) can accumulate small errors.
// 1e-10 is a conservative margin to account for differences between sequential and parallel execution.

// absolute tolerance for very small values
#define ABSOLUTE_TOLERANCE 1e-14  
// absolute tolerance: ensures very small values near zero are compared safely
// chosen as 1e-14 to handle the case where the reference or computed value is very close to zero,
// preventing false failures due to tiny absolute differences that are numerically insignificant.

#define TIMER_TOTAL 0
#define TIMER_LINEARIZATION 1
#define TIMER_MEMORY_TRANSFERS 2
#define TIMER_COMPUTATION 3

// structs
typedef struct{
	int cluster;
	double x;
	double y;
} point;

typedef struct{
	int count;
	double x;
	double y;
} mean;

// global variables
int iteration_control;
int modified;
point* points;
mean* means;
int* points_cluster_verification;
mean* means_verification;
int passed_verification;

// k-means
void k_means();
void find_clusters();
void calculate_means();

// other function prototypes
void initialization();
void verification();
void debug_results();
void release_resources();

void initialization(){
	// setup common stuff
	setup_common();

    FILE* file;

	char file_name[64];
	sprintf(file_name, "data.%s.txt", (char*)WORKLOAD);

	file = fopen(file_name, "r");

    if(file == NULL){
        printf("Error when trying to open the data set!\n");
        exit(-1);
    }
    else{	
	    // read N_POINTS
        int temp_n_points;
	    if(!fscanf(file, "%d", &temp_n_points)){exit(-1);}
        // read points
        for(int i = 0; i < N_POINTS; i++){
		    if(!fscanf(file, "%la", &points[i].x)){exit(-1);}
            if(!fscanf(file, "%la", &points[i].y)){exit(-1);}
			if(!fscanf(file, "%d", &points[i].cluster)){exit(-1);}
	    }

		// read N_MEANS
        int temp_n_means;
	    if(!fscanf(file, "%d", &temp_n_means)){exit(-1);}
        // read points
        for(int i = 0; i < N_MEANS; i++){
		    if(!fscanf(file, "%la", &means[i].x)){exit(-1);}
            if(!fscanf(file, "%la", &means[i].y)){exit(-1);}
			if(!fscanf(file, "%d", &means[i].count)){exit(-1);}
	    }

		// read N_POINTS (verification values)
	    if(!fscanf(file, "%d", &temp_n_points)){exit(-1);}
        // read points
        for(int i = 0; i < N_POINTS; i++){
			if(!fscanf(file, "%d", &points_cluster_verification[i])){exit(-1);}
	    }

		// read N_MEANS (verification values)
	    if(!fscanf(file, "%d", &temp_n_means)){exit(-1);}
        // read points
        for(int i = 0; i < N_MEANS; i++){
		    if(!fscanf(file, "%la", &means_verification[i].x)){exit(-1);}
            if(!fscanf(file, "%la", &means_verification[i].y)){exit(-1);}
			if(!fscanf(file, "%d", &means_verification[i].count)){exit(-1);}
	    }		

        // read iteration_control
        if(!fscanf(file, "%d\n", &iteration_control)){exit(-1);}
    }

    fclose(file);
}

int passed_auxiliary_verification(double result, double result_reference_value){
    // flexible correctness check
    // absolute difference between a calculated and a reference value
    double absolute_difference = fabs(result - result_reference_value);
    // dynamic relative tolerance based on the number of points
    double dynamic_relative_tolerance = RELATIVE_TOLERANCE * sqrt((double)N_POINTS);
    // tolerance = max(absolute_tolerance, dynamic_relative_tolerance * max(|result|, |result_reference_value|))
    double tolerance = fmax(ABSOLUTE_TOLERANCE, dynamic_relative_tolerance * fmax(fabs(result), fabs(result_reference_value)));
    // if difference exceeds tolerance, mark verification as failed
    if(absolute_difference > tolerance){
		passed_verification = 0;
		return 0;
	}
	else{
		return 1;
	}
}

void verification(){
	passed_verification = 1;

	int correct_points = 0;
	int correct_means = 0;

	// verification for each point
	for(int i = 0; i < N_POINTS; i++){
		if(points[i].cluster == points_cluster_verification[i]){
            correct_points++;
        }
		else{
			passed_verification = 0;
		}
	}

	// verification for each mean
	for(int i = 0; i < N_MEANS; i++){
		int is_mean_correct = 0;

		if(means[i].count == means_verification[i].count){
			is_mean_correct += 1;
		}		
		is_mean_correct += passed_auxiliary_verification(means[i].x, means_verification[i].x);
		is_mean_correct += passed_auxiliary_verification(means[i].y, means_verification[i].y);
				
		if(is_mean_correct == 3){
			correct_means++;
		}
		else{
			passed_verification = 0;
		}
	}
    
    char checksum_string_aux[2048];
    checksum_string[0] = '\0';
	// correct points
	sprintf(checksum_string_aux, "                    %20s  %20s\n", "total points", "correct points");
	strcat(checksum_string, checksum_string_aux);
	sprintf(checksum_string_aux, "                    %20d  %20d\n", N_POINTS, correct_points);
	strcat(checksum_string, checksum_string_aux);

	// correct means
	sprintf(checksum_string_aux, "                    %20s  %20s\n", "total means", "correct means");
	strcat(checksum_string, checksum_string_aux);
	sprintf(checksum_string_aux, "                    %20d  %20d", N_MEANS, correct_means);
	strcat(checksum_string, checksum_string_aux);
    
	char timer_string_aux[256];	
	sprintf(timer_string_aux, "%25s\t%20s\t%20s\n", "Timer", "Time (s)", "Percentage");
	strcpy(timer_string, timer_string_aux);
	sprintf(timer_string_aux, "%25s\t%20f\t%19.2f%%\n", "linearization", timer_read(TIMER_LINEARIZATION), (timer_read(TIMER_LINEARIZATION)*100/timer_read(TIMER_TOTAL)));
	strcat(timer_string, timer_string_aux);
	sprintf(timer_string_aux, "%25s\t%20f\t%19.2f%%\n", "memory_transfers", timer_read(TIMER_MEMORY_TRANSFERS), (timer_read(TIMER_MEMORY_TRANSFERS)*100/timer_read(TIMER_TOTAL)));
	strcat(timer_string, timer_string_aux);
	sprintf(timer_string_aux, "%25s\t%20f\t%19.2f%%", "timer_computation", timer_read(TIMER_COMPUTATION), (timer_read(TIMER_COMPUTATION)*100/timer_read(TIMER_TOTAL)));
	strcat(timer_string, timer_string_aux);
}

void debug_results(){
    if(debug_flag){
        FILE* file = fopen("kmeans.debug.dat", "w");
        if (!file) { 
            printf("Erro ao abrir arquivo de debug!\n");
            return; 
        }

        // header
        fprintf(file, "======= K-MEANS DEBUG RESULTS =======\n\n");

        // iteration info
        fprintf(file, "Total iterations: %d\n\n", iteration_control);

        // points
        fprintf(file, "Points (cluster assignment):\n");
        fprintf(file, "Index    Computed Cluster    Reference Cluster\n");
        for(int i = 0; i < N_POINTS; i++){
            fprintf(file, "%5d    %16d    %16d\n", 
                    i, points[i].cluster, points_cluster_verification[i]);
        }
        fprintf(file, "\n");

        // means
        fprintf(file, "Means (x, y, count):\n");
        fprintf(file, "Index    Computed (x, y, count)          Reference (x, y, count)\n");
        for(int i = 0; i < N_MEANS; i++){
            fprintf(file, "%5d    %10.6f %10.6f %5d    %10.6f %10.6f %5d\n", 
                    i, 
                    means[i].x, means[i].y, means[i].count, 
                    means_verification[i].x, means_verification[i].y, means_verification[i].count);
        }
        fprintf(file, "\n");

        // summary of correctness
        int correct_points = 0;
        int correct_means = 0;

        for(int i = 0; i < N_POINTS; i++){
            if(points[i].cluster == points_cluster_verification[i]){
                correct_points++;
            }
        }

        for(int i = 0; i < N_MEANS; i++){
            int is_mean_correct = 0;

			if(means[i].count == means_verification[i].count){
                is_mean_correct += 1;
            }
            is_mean_correct += passed_auxiliary_verification(means[i].x, means_verification[i].x);
            is_mean_correct += passed_auxiliary_verification(means[i].y, means_verification[i].y);
            
            if(is_mean_correct == 3){
                correct_means++;
            }
        }

        fprintf(file, "Correct points: %d / %d\n", correct_points, N_POINTS);
        fprintf(file, "Correct means:  %d / %d\n", correct_means, N_MEANS);
        fprintf(file, "Overall verification: %s\n", passed_verification ? "PASSED" : "FAILED");

        fclose(file);
    }
}

void release_resources(){
    free(points);
	free(means);
	free(points_cluster_verification);
	free(means_verification);
}
