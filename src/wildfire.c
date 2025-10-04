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

int main(int argc, char **argv)
{
    FireSimulation sim;

    fire_simulation_init(&sim, argc, argv);

    fire_simulate(&sim);

    fire_simulation_free(&sim);
    
    return 0;
}