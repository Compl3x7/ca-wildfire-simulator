/*
    Wildfire simulator using Probabilistic Cellular Automata (PCA)
    Copyright (C) 2025-2026  Alexandre Rodrigues

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    double wind_angle;
    double wind_velocity;
    int generations;
    int n_threads;
    gsl_rng **thread_rand;
    char *output_file;
} FireSimulation;

void fire_simulation_init(FireSimulation *sim, int argc, char **argv);

void fire_simulate(FireSimulation *sim);

void fire_simulation_free(FireSimulation *sim);

#endif