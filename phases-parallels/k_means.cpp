// kmeans_mpi.cpp
// Parallel K-means (MPI) — Bulk Synchronous Parallel (BSP) model
// File mode: mpiexec -np 4 k_means.exe file <max_iter>
// Generate mode: mpiexec -np 4 k_means.exe generate <k> <points_per_proc> <max_iter>
//
// Example execution: mpiexec -np 4 phases-parallels/k_means.exe generate 4 1000 50
// Example execution: mpiexec -np 4 phases-parallels/k_means.exe file 50
//
// Arguments:
//   argv[1] k               -> number of clusters
//   argv[2] points_per_proc -> number of points per process (local)
//   argv[3] max_iter        -> maximum number of iterations


#include <mpi.h>
#include <vector>
#include <iostream>
#include <random>
#include <cmath>
#include <limits>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <string>
#include <cstdio>


static const int DIM = 2;   // dimension

// Workload definitions (similar to C version)
#define WORKLOAD_A
#if defined(WORKLOAD_A)
#define WORKLOAD "A"
#define N_POINTS 10
#define N_MEANS 2
#elif defined(WORKLOAD_B)
#define WORKLOAD "B"
#define N_POINTS 1000
#define N_MEANS 10
#elif defined(WORKLOAD_C)
#define WORKLOAD "C"
#define N_POINTS 10000
#define N_MEANS 250
#else
#define WORKLOAD "A"
#define N_POINTS 10
#define N_MEANS 2
#endif

double calculate_euclidean_distance(const double* a, const double* b) {
    double dx = a[0] - b[0];
    double dy = a[1] - b[1];
    return dx*dx + dy*dy;
}

void generate_local_points(std::vector<double>& local_points, int points_per_proc, int world_rank) {
    local_points.resize(points_per_proc * DIM);
    for (int i = 0; i < points_per_proc * DIM; ++i) {
        local_points[i] = static_cast<double>(world_rank * points_per_proc + i / DIM) + (i % DIM) * 0.1;
    }
}

void read_points_from_file(std::vector<double>& all_points, std::vector<double>& initial_centroids, 
                          int& total_points, int& k, int world_rank) {
    if (world_rank == 0) {
        char file_name[64];
        sprintf(file_name, "data.%s.txt", WORKLOAD);
        
        FILE* file = fopen(file_name, "r");
        if (file == NULL) {
            std::cerr << "Error: Could not open file " << file_name << std::endl;
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        // Read number of points
        if (fscanf(file, "%d", &total_points) != 1) {
            std::cerr << "Error reading number of points" << std::endl;
            fclose(file);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        all_points.resize(total_points * DIM);
        
        // Read points (x, y coordinates, ignore cluster assignment)
        for (int i = 0; i < total_points; i++) {
            double x, y;
            int cluster;
            if (fscanf(file, "%la %la %d", &x, &y, &cluster) != 3) {
                std::cerr << "Error reading point " << i << std::endl;
                fclose(file);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            all_points[i * DIM + 0] = x;
            all_points[i * DIM + 1] = y;
        }
        
        // Read number of means
        int temp_k;
        if (fscanf(file, "%d", &temp_k) != 1) {
            std::cerr << "Error reading number of means" << std::endl;
            fclose(file);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        k = temp_k;
        
        initial_centroids.resize(k * DIM);
        
        // Read initial centroids
        for (int i = 0; i < k; i++) {
            double x, y;
            int count;
            if (fscanf(file, "%la %la %d", &x, &y, &count) != 3) {
                std::cerr << "Error reading centroid " << i << std::endl;
                fclose(file);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            initial_centroids[i * DIM + 0] = x;
            initial_centroids[i * DIM + 1] = y;
        }
        
        fclose(file);
    }
    
    // Broadcast the data to all processes
    MPI_Bcast(&total_points, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    if (world_rank != 0) {
        all_points.resize(total_points * DIM);
        initial_centroids.resize(k * DIM);
    }
    
    MPI_Bcast(all_points.data(), total_points * DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(initial_centroids.data(), k * DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void distribute_points(const std::vector<double>& all_points, std::vector<double>& local_points,
                      int total_points, int world_rank, int world_size, int& points_per_proc) {
    points_per_proc = total_points / world_size;
    int remainder = total_points % world_size;
    
    // Calculate start and end indices for this process
    int start_idx = world_rank * points_per_proc + std::min(world_rank, remainder);
    int local_count = points_per_proc + (world_rank < remainder ? 1 : 0);
    
    local_points.resize(local_count * DIM);
    
    // Copy points for this process
    for (int i = 0; i < local_count; i++) {
        local_points[i * DIM + 0] = all_points[(start_idx + i) * DIM + 0];
        local_points[i * DIM + 1] = all_points[(start_idx + i) * DIM + 1];
    }
    
    points_per_proc = local_count; // Update to actual local count
}

void initialize_centroids(std::vector<double>& centroids, const std::vector<double>& local_points, 
                         int k, int points_per_proc, int world_rank, int world_size) {
    std::mt19937 rng(1234 + world_rank * 1000);
    
    // Each process chooses a random local point and sends to rank 0
    int local_idx = std::uniform_int_distribution<int>(0, points_per_proc - 1)(rng);
    
    // Set candidates (x, y coordinates) of each process
    double candidate[2] = {
        local_points[local_idx * 2 + 0],
        local_points[local_idx * 2 + 1]
    };
    
    std::vector<double> recvbuf;
    if (world_rank == 0) recvbuf.resize(world_size * DIM);
    
    // Gather all candidates at rank 0
    MPI_Gather(candidate, DIM, MPI_DOUBLE,
               (world_rank == 0 ? recvbuf.data() : nullptr), DIM, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    
    // Rank 0 initializes centroids randomly from gathered candidates
    if (world_rank == 0) {
        std::mt19937 rng0(12345); // seed
        std::vector<int> idx(world_size); // array containing the num of processes
        for (int i = 0; i < world_size; ++i) idx[i] = i;
        std::shuffle(idx.begin(), idx.end(), rng0); // shuffle array
        
        for (int c = 0; c < k; ++c) {
            int src = idx[c % world_size];           // pick process from the shuffled array
            centroids[c*2 + 0] = recvbuf[src*2 + 0]; // Copy x-coordinate
            centroids[c*2 + 1] = recvbuf[src*2 + 1]; // Copy y-coordinate
        }
    }
    
    // Broadcast the initialized centroids to all processes
    MPI_Bcast(centroids.data(), k*DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}

void initialize(int argc, char** argv, int world_rank, int world_size,
                int& k, int& points_per_proc, int& max_iter,
                std::vector<double>& local_points, std::vector<double>& centroids) {
    
    // Check number of arguments
    if (argc < 2) {
        if (world_rank == 0) {
            std::cerr << "Usage: " << argv[0] << " <mode> [args...]" << std::endl;
            std::cerr << "  mode 'file' <max_iter>: Read from data file" << std::endl;
            std::cerr << "  mode 'generate' <k> <points_per_proc> <max_iter>: Generate data" << std::endl;
        }
        MPI_Finalize();
        exit(1);
    }

    std::string mode = argv[1];

    if (mode == "file") {
        if (argc < 3) {
            if (world_rank == 0) {
                std::cerr << "File mode requires: <max_iter>" << std::endl;
            }
            MPI_Finalize();
            exit(1);
        }
        
        max_iter = std::stoi(argv[2]);
        
        // Read data from file
        std::vector<double> all_points;
        std::vector<double> initial_centroids;
        int total_points;
        
        read_points_from_file(all_points, initial_centroids, total_points, k, world_rank);
        
        // Distribute points among processes
        distribute_points(all_points, local_points, total_points, world_rank, world_size, points_per_proc);
        
        // Use centroids from file
        centroids = initial_centroids;
        
        if (world_rank == 0) {
            std::cout << "Loaded " << total_points << " points and " << k << " centroids from file" << std::endl;
            std::cout << "Points per process: " << points_per_proc << std::endl;
        }
        
    } else if (mode == "generate") {
        if (argc < 5) {
            if (world_rank == 0) {
                std::cerr << "Generate mode requires: <k> <points_per_proc> <max_iter>" << std::endl;
            }
            MPI_Finalize();
            exit(1);
        }
        
        k = std::stoi(argv[2]);
        points_per_proc = std::stoi(argv[3]);
        max_iter = std::stoi(argv[4]);
        
        // Generate fixed points for each process
        generate_local_points(local_points, points_per_proc, world_rank);
        
        // Initialize Centroids
        centroids.resize(k * DIM, 0.0);
        initialize_centroids(centroids, local_points, k, points_per_proc, world_rank, world_size);
        
    } else {
        if (world_rank == 0) {
            std::cerr << "Unknown mode: " << mode << std::endl;
        }
        MPI_Finalize();
        exit(1);
    }
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int k, points_per_proc, max_iter;
    std::vector<double> local_points;
    std::vector<double> centroids;

    // Initialize data and parameters
    initialize(argc, argv, world_rank, world_size, k, points_per_proc, max_iter, local_points, centroids);
    
    MPI_Barrier(MPI_COMM_WORLD);

    // Local variables for each process
    std::vector<int> local_assign(points_per_proc, -1);
    std::vector<double> local_sum(k * DIM, 0.0); // Sum of coordinates for each cluster
    std::vector<int> local_count(k, 0); // Count how many points each process has assigned to each cluster locally

    // Global (for rank 0)
    std::vector<double> global_sum(k * DIM, 0.0);
    std::vector<int> global_count(k, 0);
    
    // For convergence detection
    std::vector<double> prev_centroids(k * DIM, 0.0);
    const double convergence_threshold = 1e-6; // max change less than 0.000001 units
    bool converged = false;
    int iterations_completed = 0;

    // Max iterations control
    for (int iter = 0; iter < max_iter && !converged; ++iter) {
        iterations_completed = iter + 1;
        // Reset local sums and counts each iteration
        std::fill(local_sum.begin(), local_sum.end(), 0.0);
        std::fill(local_count.begin(), local_count.end(), 0);
        
        // ------------------------
        // Assign Phase
        // ------------------------

        // Each process assigns points to the nearest centroid
        for (int i = 0; i < points_per_proc; ++i) {
            double min_dist = std::numeric_limits<double>::max();
            int min_idx = -1;

            for (int j = 0; j < k; ++j) {
                double dist = calculate_euclidean_distance(
                    &local_points[i * DIM], &centroids[j * DIM]);
                if (dist < min_dist) {
                    min_dist = dist;
                    min_idx = j;
                }
            }

            local_assign[i] = min_idx;
            local_sum[min_idx * DIM + 0] += local_points[i * DIM + 0];
            local_sum[min_idx * DIM + 1] += local_points[i * DIM + 1];
            local_count[min_idx]++;
        }

        // ------------------------
        // Synchronize Phase (All-to-All Broadcast)
        // ------------------------

        // All processes exchange their local sums and counts with each other using Allreduce
        MPI_Allreduce(local_sum.data(), global_sum.data(), k*DIM, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        MPI_Allreduce(local_count.data(), global_count.data(), k, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        // ------------------------
        // Update Centroids Phase (All Processes)
        // ------------------------

        // Perform centroid update and convergence check
        // Store previous centroids for convergence check
        prev_centroids = centroids;
        
        for (int j = 0; j < k; ++j) {
            if (global_count[j] > 0) {
                centroids[j * DIM + 0] = global_sum[j * DIM + 0] / global_count[j];
                centroids[j * DIM + 1] = global_sum[j * DIM + 1] / global_count[j];
            }
        }
        
        // All processes check for convergence
        double max_change = 0.0;
        for (int j = 0; j < k * DIM; ++j) {
            double change = std::abs(centroids[j] - prev_centroids[j]);
            max_change = std::max(max_change, change);
        }
        
        if (max_change < convergence_threshold) {
            converged = true;
            if (world_rank == 0) {
                std::cout << "\nConverged after " << (iter + 1) << " iterations (max change: " 
                          << std::scientific << std::setprecision(2) << max_change << ")" << std::endl;
            }
        }
        
        // Only rank 0 prints intermediate results for first iteration and halfway point
        if (world_rank == 0 && (iter == 0 || iter == max_iter / 2)) {
            std::cout << "\n-------- Iteration " << (iter + 1) << " Results --------" << std::endl;
            for (int j = 0; j < k; ++j) {
                std::cout << "Centroid " << j << ": ("
                          << std::fixed << std::setprecision(2)
                          << centroids[j * DIM + 0] << ", "
                          << centroids[j * DIM + 1] << ")" << std::endl;
            }
            std::cout << std::endl;
        }
    }

    
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        std::cout << "\n-------- Final Results --------" << std::endl;
        if (converged) {
            std::cout << "Algorithm converged successfully!" << std::endl;
        } else {
            std::cout << "Maximum iterations reached without convergence." << std::endl;
        }
        std::cout << "Number of iterations completed: " << iterations_completed << std::endl;
        for (int j = 0; j < k; ++j) {
            std::cout << "Centroid " << j << ": ("
                      << std::fixed << std::setprecision(2)
                      << centroids[j * DIM + 0] << ", "
                      << centroids[j * DIM + 1] << ")" << std::endl;
        }
    }

    MPI_Finalize();
    return 0;
}