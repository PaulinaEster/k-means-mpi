
#include "../include/k-means/k_means.h"
#include<mpi.h>

#define ROOT 0

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	points = (point*) malloc(N_POINTS * sizeof(point));
    // means = (mean*) malloc(N_MEANS * sizeof(mean));

    count = (int*) malloc(N_MEANS * sizeof(int));
    x = (double*) malloc(N_MEANS * sizeof(double));
    y = (double*) malloc(N_MEANS * sizeof(double));

    points_cluster_verification = (int*) malloc(N_POINTS * sizeof(int));
    means_verification = (mean*) malloc(N_MEANS * sizeof(mean));

	// initial values
	initialization();
    // if(rank == ROOT){
    //     timer_start(TIMER_TOTAL); 
    // }
	k_means(); 

    if(rank == ROOT){ 
        timer_stop(TIMER_TOTAL);

        // checksum routine
        verification();
	    printf("[%d] verification\n", rank);

        // print results
        debug_results();	
	    printf("[%d] debug_results\n", rank);

        // freeing memory and stuff
        release_resources();
	    printf("[%d] release_resources\n", rank);

        execution_report((char*)"K-Means", (char*)WORKLOAD, timer_read(TIMER_TOTAL), passed_verification);
    }
	printf("[%d] Finalize\n", rank);
    MPI_Finalize();
	return 0;
}

void k_means(){
	modified = 1;
    iteration_control = 0;

    int *count_g = NULL;
    double *x_g = NULL;
    double *y_g = NULL;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    count_g = (int*) malloc(N_MEANS * sizeof(int));
    x_g = (double*) malloc(N_MEANS * sizeof(double));
    y_g = (double*) malloc(N_MEANS * sizeof(double));
    int mod_aux = 1;
    while(mod_aux){
        modified = 0;

        find_clusters(rank, nprocs);

        calculate_means(rank, nprocs, x_g, y_g, count_g);

        iteration_control++;
	    printf("[%d] iteration_control modified %d\n", rank, modified);
        MPI_Allreduce(&modified, &mod_aux, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    }
	printf("[%d] SAINDO\n", rank);
}

void find_clusters(int my_rank, int nprocs){
    for(int i = my_rank; i < N_POINTS; i+=nprocs){
        double min_dist = (points[i].x - x[my_rank]) * (points[i].x - x[my_rank])
                        + (points[i].y - y[my_rank]) * (points[i].y - y[my_rank]);
        int cluster_id = my_rank;

        for(int j = 1; j < N_MEANS; j++){
            double cur_dist = (points[i].x - x[j]) * (points[i].x - x[j])
                            + (points[i].y - y[j]) * (points[i].y - y[j]);
            if(cur_dist < min_dist){
                min_dist = cur_dist;
                cluster_id = j;
            }
        }

        if(points[i].cluster != cluster_id){
            points[i].cluster = cluster_id;
            modified = 1;
        }
    }
}

void calculate_means(int my_rank, int nprocs, double* x_, double* y_, int* count_){
    for(int i = 0; i < N_MEANS; i++){
        count_[i] = 0;
        y_[i] = 0.0;
        x_[i] = 0.0;
    }

    for(int i = my_rank; i < N_POINTS; i+=nprocs){
        int cluster = points[i].cluster;
        count_[cluster]++;
        x_[cluster] += points[i].x;
        y_[cluster] += points[i].y;
    }

    MPI_Allreduce(x_, x, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(y_, y, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(count_, count, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    for(int i = 0; i < N_MEANS; i++){
        if(count[i] > 0){
            x[i] /= count[i];
            y[i] /= count[i];
        }
    }
}
