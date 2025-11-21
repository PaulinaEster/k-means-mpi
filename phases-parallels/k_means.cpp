// kmeans_mpi.cpp
// Parallel K-means (MPI) — Bulk Synchronous Parallel (BSP) model
// Example execution: mpiexec -np 4 phases-parallels/k_means.exe 4 1000 50
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


static const int DIM = 2;   // dimension

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

void initialize_centroids(std::vector<double>& centroids, const std::vector<double>& local_points, 
                         int k, int points_per_proc, int world_rank, int world_size) {
    std::mt19937 rng(1234 + world_rank * 1000);
    
    // Each process chooses a random local point and sends to rank 0
    int local_idx = std::uniform_int_distribution<int>(0, points_per_proc - 1)(rng);
    
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
    
    // Rank 0 initializes centroids from gathered candidates
    if (world_rank == 0) {
        std::mt19937 rng0(12345);
        std::vector<int> idx(world_size);
        for (int i = 0; i < world_size; ++i) idx[i] = i;
        std::shuffle(idx.begin(), idx.end(), rng0);
        
        for (int c = 0; c < k; ++c) {
            int src = idx[c % world_size];
            centroids[c*2 + 0] = recvbuf[src*2 + 0];
            centroids[c*2 + 1] = recvbuf[src*2 + 1];
        }
    }
    
    // Broadcast the initialized centroids to all processes
    MPI_Bcast(centroids.data(), k*DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
}


int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Check number of arguments
    if (argc < 4) {
        if (world_rank == 0) {
            std::cerr << "You must provide 3 arguments: <k> <points_per_proc> <max_iter>" << std::endl;
        }
        MPI_Finalize();
        return 1;
    }

    int k = std::stoi(argv[1]);
    int points_per_proc = std::stoi(argv[2]);
    int max_iter = std::stoi(argv[3]);

    // Generate fixed points for each process
    std::vector<double> local_points;
    generate_local_points(local_points, points_per_proc, world_rank);
    MPI_Barrier(MPI_COMM_WORLD);

    // Initialize Centroids
    std::vector<double> centroids(k * DIM, 0.0);
    initialize_centroids(centroids, local_points, k, points_per_proc, world_rank, world_size);
    MPI_Barrier(MPI_COMM_WORLD);

    // Local variables for each process
    std::vector<int> local_assign(points_per_proc, -1);
    std::vector<double> local_sum(k * DIM, 0.0);
    std::vector<int> local_count(k, 0);

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
        // Synchronize Phase
        // ------------------------

        // Each process sends its local sums and counts to the root process (rank 0)
        MPI_Reduce(local_sum.data(), global_sum.data(), k*DIM, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(local_count.data(), global_count.data(), k, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

        // ------------------------
        // Update Centroids Phase
        // ------------------------

        if (world_rank == 0) {
            // Store previous centroids for convergence check
            prev_centroids = centroids;
            
            for (int j = 0; j < k; ++j) {
                if (global_count[j] > 0) {
                    centroids[j * DIM + 0] = global_sum[j * DIM + 0] / global_count[j];
                    centroids[j * DIM + 1] = global_sum[j * DIM + 1] / global_count[j];
                }
            }
            
            // Check for convergence
            double max_change = 0.0;
            for (int j = 0; j < k * DIM; ++j) {
                double change = std::abs(centroids[j] - prev_centroids[j]);
                max_change = std::max(max_change, change);
            }
            
            if (max_change < convergence_threshold) {
                converged = true;
                std::cout << "\nConverged after " << (iter + 1) << " iterations (max change: " 
                          << std::scientific << std::setprecision(2) << max_change << ")" << std::endl;
            }
            
            // Print intermediate results for first iteration and halfway point
            if (iter == 0 || iter == max_iter / 2) {
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

        // Broadcast updated centroids to all processes
        MPI_Bcast(centroids.data(), k*DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        
        // Broadcast convergence status to all processes
        MPI_Bcast(&converged, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
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