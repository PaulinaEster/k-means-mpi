
#include "../include/k-means/k_means.h"

void data_generator_find_clusters(){
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

void data_generator_calculate_means(){
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

int main(){
    //////////////////////////////////////////////////
    // allocate points, means, and verification values
    points = (point*) malloc(N_POINTS * sizeof(point));
    means = (mean*) malloc(N_MEANS * sizeof(mean));

    //////////////////////////////
    // initialize points and means
    for (int i = 0; i < N_POINTS; i++) {
        points[i].x = rand() % INTERVAL;
        points[i].y = rand() % INTERVAL;
        points[i].cluster = 0;
    }
    for (int i = 0; i < N_MEANS; i++) {
        means[i].x = rand() % INTERVAL;
        means[i].y = rand() % INTERVAL;
        means[i].count = 0;
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
		    fprintf(file, "%la %la %d\n", points[i].x, points[i].y, points[i].cluster);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
        // write N_MEANS
        fprintf(file, "%d\n", N_MEANS);
        // blank line
        fprintf(file, "\n");
        // write means
        for(int i = 0; i < N_MEANS; i++){
		    fprintf(file, "%la %la %d\n", means[i].x, means[i].y, means[i].count);
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
		    fprintf(file, "%d\n", points[i].cluster);
	    }
        // blank lines
        fprintf(file, "\n\n\n\n\n");
        // write N_MEANS
        fprintf(file, "%d\n", N_MEANS);
        // blank line
        fprintf(file, "\n");
        // write means' results
        for(int i = 0; i < N_MEANS; i++){
		    fprintf(file, "%la %la %d\n", means[i].x, means[i].y, means[i].count);
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
