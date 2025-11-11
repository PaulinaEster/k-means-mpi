
#include "include/k-means/k_means.h"

int main(int argc, char* argv[]){
	points = (point*) malloc(N_POINTS * sizeof(point));
    means = (mean*) malloc(N_MEANS * sizeof(mean));
    points_cluster_verification = (int*) malloc(N_POINTS * sizeof(int));
    means_verification = (mean*) malloc(N_MEANS * sizeof(mean));

	// initial values
	initialization();

	timer_start(TIMER_TOTAL);

	// linearization of the data, if applicable
	if(timer_flag){timer_start(TIMER_LINEARIZATION);}
	if(timer_flag){timer_stop(TIMER_LINEARIZATION);}

	// memory transfers, if applicable
	if(timer_flag){timer_start(TIMER_MEMORY_TRANSFERS);}
	if(timer_flag){timer_stop(TIMER_MEMORY_TRANSFERS);}	

	// k-means
	if(timer_flag){timer_start(TIMER_COMPUTATION);}
	k_means();
	if(timer_flag){timer_stop(TIMER_COMPUTATION);}

    // memory transfers, if applicable
	if(timer_flag){timer_start(TIMER_MEMORY_TRANSFERS);}
	if(timer_flag){timer_stop(TIMER_MEMORY_TRANSFERS);}	

	// linearization of the data, if applicable
	if(timer_flag){timer_start(TIMER_LINEARIZATION);}
	if(timer_flag){timer_stop(TIMER_LINEARIZATION);}	

	timer_stop(TIMER_TOTAL);

	// checksum routine
	verification();

	// print results
	debug_results();	

	// freeing memory and stuff
	release_resources();

	execution_report((char*)"K-Means", (char*)WORKLOAD, timer_read(TIMER_TOTAL), passed_verification);

	return 0;
}

void k_means(){
	modified = 1;
    iteration_control = 0;

    while(modified){
        modified = 0;

        find_clusters();

        calculate_means();

        iteration_control++;
	    printf(" iteration_control modified %d\n", modified);
    }
}

void find_clusters(){
    for(int i = 0; i < N_POINTS; i++){
        double min_dist = (points[i].x - means[0].x) * (points[i].x - means[0].x)
                        + (points[i].y - means[0].y) * (points[i].y - means[0].y);
        int min_idx = 0;

        for(int j = 1; j < N_MEANS; j++){
            double cur_dist = (points[i].x - means[j].x) * (points[i].x - means[j].x)
                            + (points[i].y - means[j].y) * (points[i].y - means[j].y);
            if(cur_dist < min_dist){
                min_dist = cur_dist;
                min_idx = j;
            }
        }

        if(points[i].cluster != min_idx){
            points[i].cluster = min_idx;
            modified = 1;
        }
    }
}

void calculate_means(){
    for(int i = 0; i < N_MEANS; i++){
        means[i].count = 0;
        means[i].x = 0.0;
        means[i].y = 0.0;
    }

    for(int i = 0; i < N_POINTS; i++){
        int cluster = points[i].cluster;
        means[cluster].count++;
        means[cluster].x += points[i].x;
        means[cluster].y += points[i].y;
    }

    for(int i = 0; i < N_MEANS; i++){
        if(means[i].count > 0){
            means[i].x /= means[i].count;
            means[i].y /= means[i].count;
        }
    }
}
