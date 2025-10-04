#ifndef FIRE_SIMULATION_H
#define FIRE_SIMULATION_H

#include <gsl/gsl_rng.h>
#include "cell.h"

typedef struct FireSimulation
{
    unsigned long base_seed;
    gsl_rng *rand;
    int grid_height;
    int grid_width;
    float min_x;
    float min_y;
    float scale;
    Cell **curr_grid;
    Cell **next_grid;
    int generations;
    int n_threads; 
} FireSimulation;

void fire_simulation_init(FireSimulation *sim, int argc, char **argv);

void fire_simulate(FireSimulation *sim);

void fire_simulation_free(FireSimulation *sim);

#endif