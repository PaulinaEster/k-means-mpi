
#include "include/k-means/k_means.h"

void data_generator_find_clusters(){
    for(int i = 0; i < N_POINTS; i++){
        double min_dist = (points->x[i] - means->x[0]) * (points->x[i] - means->x[0])
                        + (points->y[i] - means->y[0]) * (points->y[i] - means->y[0]);
        int min_idx = 0;

        for(int j = 1; j < N_MEANS; j++){
            double cur_dist = (points->x[i] - means->x[j]) * (points->x[i] - means->x[j])
                            + (points->y[i] - means->y[j]) * (points->y[i] - means->y[j]);
            if(cur_dist < min_dist){
                min_dist = cur_dist;
                min_idx = j;
            }
        }

        if(points->cluster[i] != min_idx){
            points->cluster[i] = min_idx;
            modified = 1;
        }
    }
}

void data_generator_calculate_means(){
    for(int i = 0; i < N_MEANS; i++){
        means->count[i] = 0;
        means->x[i] = 0.0;
        means->y[i] = 0.0;
    }

    for(int i = 0; i < N_POINTS; i++){
        int cluster = points->cluster[i];
        means->count[cluster]++;
        means->x[cluster] += points->x[i];
        means->y[cluster] += points->y[i];
    }

    for(int i = 0; i < N_MEANS; i++){
        if(means->count[i] > 0){
            means->x[i] /= means->count[i];
            means->y[i] /= means->count[i];
        }
    }
}

int main(){
    //////////////////////////////////////////////////
    // allocate points, means, and verification valuespoints->cluster = (int*) malloc(N_MEANS * sizeof(int));
    points = (Points*) malloc(sizeof(Points));
    points->cluster = (int*) malloc(N_MEANS * sizeof(int));
    points->x = (double*) malloc(N_MEANS * sizeof(double));
    points->y = (double*) malloc(N_MEANS * sizeof(double));
    means = (Means*) malloc(sizeof(Means));
    means->count = (int*) malloc(N_MEANS * sizeof(int));
    means->x = (double*) malloc(N_MEANS * sizeof(double));
    means->y = (double*) malloc(N_MEANS * sizeof(double));

    //////////////////////////////
    // initialize points and means
    for (int i = 0; i < N_POINTS; i++) {
        points->x[i] = rand() % INTERVAL;
        points->y[i] = rand() % INTERVAL;
        points->cluster[i] = 0;
    }
    for (int i = 0; i < N_MEANS; i++) {
        means->x[i] = rand() % INTERVAL;
        means->y[i] = rand() % INTERVAL;
        means->count[i] = 0;
    }

    ///////////////////////////
    // write data set in a file
    FILE* file;
    char file_name[64];
	sprintf(file_name, "data.%s.txt", (char*)WORKLOAD);
    file = fopen(file_name, "wt");
    if(file == NULL){
        printf("Error when trying to open a file!\n");
        exit(-1);
    }
    else{
        // write N_POINTS
        fprintf(file, "%d\n", N_POINTS);
        // blank line
        fprintf(file, "\n");	
        // write points
        for(int i = 0; i < N_POINTS; i++){
		    fprintf(file, "%la %la %d\n", points->x[i], points->y[i], points->cluster[i]);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
        // write N_MEANS
        fprintf(file, "%d\n", N_MEANS);
        // blank line
        fprintf(file, "\n");
        // write means
        for(int i = 0; i < N_MEANS; i++){
		    fprintf(file, "%la %la %d\n", means->x[i], means->y[i], means->count[i]);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
    }
    fclose(file);

    //////////////
    // run k-means
    modified = 1;
    iteration_control = 0;
    while(modified){
        modified = 0;
        data_generator_find_clusters();
        data_generator_calculate_means();
        iteration_control++;
    }

    ///////////////////////////////////
    // write data set results in a file
    file = fopen(file_name, "at");
    if(file == NULL){
        printf("Error when trying to open a file!\n");
        exit(-1);
    }
    else{
        // write N_POINTS
        fprintf(file, "%d\n", N_POINTS);
        // blank line
        fprintf(file, "\n");	
        // write points
        for(int i = 0; i < N_POINTS; i++){
		    fprintf(file, "%d\n", points->cluster[i]);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
        // write N_MEANS
        fprintf(file, "%d\n", N_MEANS);
        // blank line
        fprintf(file, "\n");
        // write means' results
        for(int i = 0; i < N_MEANS; i++){
		    fprintf(file, "%la %la %d\n", means->x[i], means->y[i], means->count[i]);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
        // iterations to converge
        fprintf(file, "%d\n", iteration_control);
    }
    fclose(file);

    free(points);
    free(means);
    return 0;
}
