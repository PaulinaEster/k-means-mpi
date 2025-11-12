
#include "include/k-means/k_means.h"
#include<mpi.h>

#define ROOT 0

int main(int argc, char* argv[]){
    MPI_Init(&argc, &argv);

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    points = (Points*) malloc(sizeof(Points));
    points->cluster = (int*) malloc(N_POINTS * sizeof(int));
    points->x = (double*) malloc(N_POINTS * sizeof(double));
    points->y = (double*) malloc(N_POINTS * sizeof(double));
    means = (Means*) malloc(sizeof(Means));
    means->count = (int*) malloc(N_MEANS * sizeof(int));
    means->x = (double*) malloc(N_MEANS * sizeof(double));
    means->y = (double*) malloc(N_MEANS * sizeof(double));

    points_cluster_verification = (int*) malloc(N_POINTS * sizeof(int));
    means_verification = (mean*) malloc(N_MEANS * sizeof(mean));

	// initial values
	initialization();
    if(rank == ROOT){
        timer_start(TIMER_TOTAL); 
    }
	k_means(); 

    if(rank == ROOT){ 
        timer_stop(TIMER_TOTAL);

        // checksum routine
        verification(); 
        // print results
        debug_results();	 

        // freeing memory and stuff
        release_resources(); 

        execution_report((char*)"K-Means", (char*)WORKLOAD, timer_read(TIMER_TOTAL), passed_verification);
    } 
    MPI_Finalize();
	return 0;
}

void k_means(){
	modified = 1;
    iteration_control = 0;

    int *count_g = NULL;
    double *x_g = NULL;
    double *y_g = NULL;
    int *cluster_p = NULL; 
    double *x_p = NULL;
    double *y_p = NULL;

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int nprocs;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    count_g = (int*) malloc(N_MEANS * sizeof(int));
    x_g = (double*) malloc(N_MEANS * sizeof(double));
    y_g = (double*) malloc(N_MEANS * sizeof(double));

    cluster_p = (int*) malloc(N_POINTS * sizeof(int));
    x_p = (double*) malloc(N_POINTS * sizeof(double));
    y_p = (double*) malloc(N_POINTS * sizeof(double));

    memcpy(cluster_p, points->cluster, N_POINTS * sizeof(int));
    memcpy(x_p, points->x, N_POINTS * sizeof(double));
    memcpy(y_p, points->y, N_POINTS * sizeof(double));

    int mod_aux = 1;
    while(mod_aux){
        modified = 0;

        find_clusters(rank, nprocs, x_p, y_p, cluster_p);

        calculate_means(rank, nprocs, x_g, y_g, count_g, x_p, y_p, cluster_p);

        iteration_control++;
        MPI_Allreduce(&modified, &mod_aux, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
    }
    MPI_Reduce(cluster_p, points->cluster, N_POINTS, MPI_INT, MPI_MAX, ROOT, MPI_COMM_WORLD);
}

void find_clusters(int my_rank, int nprocs, double* x_p, double* y_p, int* cluster_p){
    for(int i = my_rank; i < N_POINTS; i+=nprocs){
        double min_dist = (x_p[i] - means->x[0]) * (x_p[i] - means->x[0])
                        + (y_p[i] - means->y[0]) * (y_p[i] - means->y[0]);
        int cluster_id = 0;

        for(int j = 1; j < N_MEANS; j++){
            double cur_dist = (x_p[i] - means->x[j]) * (x_p[i] - means->x[j])
                            + (y_p[i] - means->y[j]) * (y_p[i] - means->y[j]);
            if(cur_dist < min_dist){
                min_dist = cur_dist;
                cluster_id = j;
            }
        }

        if(cluster_p[i] != cluster_id){
            cluster_p[i] = cluster_id;
            modified = 1;
        }
    }
}

void calculate_means(int my_rank, int nprocs, double* x_, double* y_, int* count_, double* x_p, double* y_p, int* cluster_p){
    for(int i = 0; i < N_MEANS; i++){
        count_[i] = 0;
        y_[i] = 0.0;
        x_[i] = 0.0;
    }

    for(int i = my_rank; i < N_POINTS; i+=nprocs){
        int cluster = cluster_p[i];
        count_[cluster]++;
        x_[cluster] += x_p[i];
        y_[cluster] += y_p[i];
    }

    MPI_Allreduce(x_, means->x, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(y_, means->y, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    MPI_Allreduce(count_, means->count, N_MEANS, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    for(int i = 0; i < N_MEANS; i++){
        if(means->count[i] > 0){
            means->x[i] /= means->count[i];
            means->y[i] /= means->count[i];
        }
    }
}
