#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <gsl/gsl_rng.h>
#include <omp.h>

#include "raster.h"
#include "cell.h"
#include "fire_simulation.h"

void print_times(double start_s, double init_end_s, double sim_end_s, double free_end_s)
{
    double total_time = free_end_s - start_s;
    double init_time = init_end_s - start_s;
    double sim_time = sim_end_s - init_end_s;
    double free_time = free_end_s - sim_end_s;

    printf("Total time: %fs\n", total_time);
    printf("Init time: %fs\n", init_time);
    printf("Sim time: %fs\n", sim_time);
    printf("Free time: %fs\n", free_time);
}

int main(int argc, char **argv)
{
    FireSimulation sim;

    double start_s = omp_get_wtime();

    fire_simulation_init(&sim, argc, argv);

    double init_end_s = omp_get_wtime();

    fire_simulate(&sim);

    double sim_end_s = omp_get_wtime();

    fire_simulation_free(&sim);

    double free_end_s = omp_get_wtime();
    
    print_times(start_s, init_end_s, sim_end_s, free_end_s);

    return 0;
}