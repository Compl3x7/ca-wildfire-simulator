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

#define DATASET_COUNT 2

#define HELP_FLAG "--help"
#define DEM_FLAG "--dem"
#define LAND_FLAG "--clc"
#define SCALE_FLAG "--scale"
#define GENS_FLAG "--gens"
#define SEED_FLAG "--seed"
#define N_THREADS_FLAG "--n-threads"
#define OUTPUT_FLAG "--out"

#define DEFAULT_GENS 100
#define DEFAULT_THREADS 2

#define SQRT_2 1.41421356237
#define PI 3.14159265359
#define P_H 0.58
//#define ALPHA 0.078
#define ALPHA 0.727
#define C_1 0.045
#define C_2 0.131

void print_usage()
{
    fprintf(stderr, "Usage: ./wildfire %s <file> %s <file> %s <meters> [OPTIONS]\n\n", DEM_FLAG, LAND_FLAG, SCALE_FLAG);

    fprintf(stderr, "REQUIRED ARGUMENTS:\n");
    fprintf(stderr, "\t%s <file>         Digital Elevation Model in XYZ format\n", DEM_FLAG);
    fprintf(stderr, "\t%s <file>         Land cover data in XYZ format\n", LAND_FLAG);
    fprintf(stderr, "\t%s <meters>     Cell size in meters (e.g., 25.0)\n\n", SCALE_FLAG);
    
    fprintf(stderr, "OPTIONAL ARGUMENTS:\n");
    fprintf(stderr, "\t%s <N>           Number of time steps (default: 100)\n", GENS_FLAG);
    fprintf(stderr, "\t%s <value>       Random seed (default: current time)\n", SEED_FLAG);
    fprintf(stderr, "\t%s <N>      Number of threads to use (default: 2)\n", N_THREADS_FLAG);
    fprintf(stderr, "\t%s <file>         Output CSV file (default: None)\n\n", OUTPUT_FLAG);
    
    exit(1);
}

int valid_args(FireSimulation *sim, char **elevation_raster_file_name, char **land_cover_raster_file_name)
{
    return elevation_raster_file_name != NULL && land_cover_raster_file_name != NULL 
        && sim->scale > 0 && sim->generations > 0 && sim->base_seed >= 0 && sim->n_threads > 0;
}

void parse_args(FireSimulation *sim, int argc, char **argv, char **elevation_raster_file_name, char **land_cover_raster_file_name)
{
    *elevation_raster_file_name = NULL;
    *land_cover_raster_file_name = NULL;
    sim->scale = 0.00;
    sim->generations = DEFAULT_GENS;
    sim->base_seed = time(NULL);
    sim->n_threads = DEFAULT_THREADS;
    sim->output_file = NULL;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], HELP_FLAG) == 0 && i + 1 < argc)
            print_usage();
        if (strcmp(argv[i], DEM_FLAG) == 0 && i + 1 < argc)
            *elevation_raster_file_name = argv[++i];
        else if (strcmp(argv[i], LAND_FLAG) == 0 && i + 1 < argc)
            *land_cover_raster_file_name = argv[++i];
        else if (strcmp(argv[i], SCALE_FLAG) == 0 && i + 1 < argc)
            sim->scale = strtof(argv[++i], NULL);
        else if (strcmp(argv[i], GENS_FLAG) == 0 && i + 1 < argc)
            sim->generations = atoi(argv[++i]);
        else if (strcmp(argv[i], SEED_FLAG) == 0 && i + 1 < argc)
            sim->base_seed = strtoul(argv[++i], NULL, 10);
        else if (strcmp(argv[i], N_THREADS_FLAG) == 0 && i + 1 < argc)
            sim->n_threads = atoi(argv[++i]);
        else if (strcmp(argv[i], OUTPUT_FLAG) == 0 && i + 1 < argc)
            sim->output_file = argv[++i];
        else
            print_usage();
    }

    if (!valid_args(sim, elevation_raster_file_name, land_cover_raster_file_name))
        print_usage();
}

int read_data(const char *elevation_raster_file_name, const char *land_cover_raster_file_name, RasterPoint **elevation_raster, RasterPoint **land_cover_raster)
{
    int elevation_raster_size;
    int land_cover_raster_size;
    *elevation_raster = raster_read_from_file(elevation_raster_file_name, &elevation_raster_size);
    if (elevation_raster == NULL)
    {
        fprintf(stderr, "Error while reading from %s\n", elevation_raster_file_name);
        exit(1);
    }
    *land_cover_raster = raster_read_from_file(land_cover_raster_file_name, &land_cover_raster_size);
    if (land_cover_raster == NULL)
    {
        fprintf(stderr, "Error while reading from %s\n", land_cover_raster_file_name);
        free(elevation_raster);
        exit(1);
    }

    if (elevation_raster_size != land_cover_raster_size)
    {
        fprintf(stderr, "Raster file sizes do not correspond\n");
        free(elevation_raster);
        free(land_cover_raster);
        exit(1);
    }
    return elevation_raster_size; //elevation_raster_size == land_cover_raster_size
}

void compute_grid_size(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    RasterBound elevation_bound = raster_get_bound(elevation_raster, size);
    RasterBound land_cover_bound = raster_get_bound(land_cover_raster, size);
    if (!raster_equal_bounds(elevation_bound, land_cover_bound))
    {
        fprintf(stderr, "Raster file bounds do not match\n");
        free(elevation_raster);
        free(land_cover_raster);
        exit(1);
    }

    sim->min_x = elevation_bound.min_x;
    sim->min_y = elevation_bound.min_y;
    sim->grid_height = (int) ((elevation_bound.max_y - sim->min_y) / sim->scale) + 1;
    sim->grid_width = (int) ((elevation_bound.max_x - sim->min_x) / sim->scale) + 1;
    //TODO Question mark, covers all cases?
    if (sim->grid_height * sim->grid_width != size)
    {
        fprintf(stderr, "Computed grid size doesn't correspond to the size of given raster files, possible scale mismtach\n");
        free(elevation_raster);
        free(land_cover_raster);
        exit(1);
    }
}

void alloc_rngs(FireSimulation *sim)
{
    sim->rand = gsl_rng_alloc(gsl_rng_mt19937);
    gsl_rng_set(sim->rand, sim->base_seed);
    sim->thread_rand = malloc(sim->n_threads * sizeof(gsl_rng*));
    for (int i = 0; i < sim->n_threads; i++)
        sim->thread_rand[i] = gsl_rng_alloc(gsl_rng_mt19937);
}

void free_rngs(FireSimulation *sim)
{
    gsl_rng_free(sim->rand);
    for (int i = 0; i < sim->n_threads; i++)
        gsl_rng_free(sim->thread_rand[i]);
    free(sim->thread_rand);
}

void alloc_grids(FireSimulation *sim)
{
    sim->curr_grid = malloc(sim->grid_height * sizeof(Cell*));
    sim->next_grid = malloc(sim->grid_height * sizeof(Cell*));
    for (int i = 0; i < sim->grid_height; i++)
    {
        sim->curr_grid[i] = malloc(sim->grid_width * sizeof(Cell));
        sim->next_grid[i] = malloc(sim->grid_width * sizeof(Cell));
    }
}

void free_grids(FireSimulation *sim)
{
    for (int i = 0; i < sim->grid_height; i++)
    {
        free(sim->curr_grid[i]);
        free(sim->next_grid[i]);
    }
    free(sim->curr_grid);
    free(sim->next_grid);
}

void fire_simulation_free(FireSimulation *sim)
{
    free_rngs(sim);
    free_grids(sim);
}

//TODO Needs check ?
void populate_grids_elevation(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    for (int i = 0; i < size; i++)
    {
        int elevation_i = sim->grid_height - 1 - (int) (elevation_raster[i].y_coordinate - sim->min_y) / sim->scale;
        int elevation_j = (int) (elevation_raster[i].x_coordinate - sim->min_x) / sim->scale;
        sim->curr_grid[elevation_i][elevation_j].elevation = elevation_raster[i].z_value;
        sim->next_grid[elevation_i][elevation_j].elevation = elevation_raster[i].z_value;
    }
}

//TODO Needs check ?
void populate_grids_land_cover(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    for (int i = 0; i < size; i++)
    {
        int land_cover_i = sim->grid_height - 1 - (int) (land_cover_raster[i].y_coordinate - sim->min_y) / sim->scale;
        int land_cover_j = (int) (land_cover_raster[i].x_coordinate - sim->min_x) / sim->scale;
        LandType type = clc_to_land_type(land_cover_raster[i].z_value);
        if (type == INVALID_TYPE)
        {
            fprintf(stderr ,"Invalid land cover value on land cover raster file\n");
            free(elevation_raster);
            free(land_cover_raster);
            fire_simulation_free(sim);
            exit(1);
        }

        sim->curr_grid[land_cover_i][land_cover_j].land_type = type;
        sim->next_grid[land_cover_i][land_cover_j].land_type = type;
        sim->curr_grid[land_cover_i][land_cover_j].land_state = determine_land_state(type);
        sim->next_grid[land_cover_i][land_cover_j].land_state = determine_land_state(type);
    }
}

void populate_grids(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    populate_grids_elevation(sim, elevation_raster, land_cover_raster, size);
    populate_grids_land_cover(sim, elevation_raster, land_cover_raster, size);
}

//Aux struct
typedef struct Index2D
{
    int i;
    int j;
} Index2D;

Index2D* get_ignitable_points(FireSimulation *sim, int *points_size)
{
    int alloc_size = sim->grid_height * sim->grid_width;
    int size = 0;
    Index2D *valid_points = malloc(alloc_size * sizeof(Index2D));
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            if (sim->curr_grid[i][j].land_state == UNIGNITED)
            {
                valid_points[size].i = i;
                valid_points[size].j = j;
                size++;
            }

    valid_points = realloc(valid_points, size * sizeof(Index2D));
    *points_size = size;
    return valid_points;
}

void swap(Index2D *points, int i, int j)
{
    Index2D temp = points[i];
    points[i] = points[j];
    points[j] = temp;
}

void set_random_ignition_points(FireSimulation *sim, int n_points)
{
    int size;
    Index2D *valid_points = get_ignitable_points(sim, &size);

    for (int i = 0; i < n_points; i++)
    {
        int random = gsl_rng_uniform_int(sim->rand, size);
        Index2D selected = valid_points[random];
        sim->curr_grid[selected.i][selected.j].land_state = BURNING;
        sim->next_grid[selected.i][selected.j].land_state = BURNING;
        swap(valid_points, random, --size);
    }
    free(valid_points);
}

void fire_simulation_init(FireSimulation *sim, int argc, char **argv)
{
    char *elevation_raster_file_name;
    char *land_cover_raster_file_name;
    parse_args(sim, argc, argv, &elevation_raster_file_name, &land_cover_raster_file_name);

    RasterPoint *elevation_raster;
    RasterPoint *land_cover_raster;
    int size = read_data(elevation_raster_file_name, land_cover_raster_file_name, &elevation_raster, &land_cover_raster);

    compute_grid_size(sim, elevation_raster, land_cover_raster, size);
    alloc_rngs(sim);
    alloc_grids(sim);
    populate_grids(sim, elevation_raster, land_cover_raster, size);
    set_random_ignition_points(sim, 1);
    
    free(elevation_raster);
    free(land_cover_raster);
}

double compute_p_distance(Index2D from, Index2D to)
{
    return from.i != to.i && from.j != to.j ? 1 / SQRT_2 : 1;
}

double compute_p_slope(FireSimulation *sim, Index2D from, Index2D to)
{
    double distance = from.i != to.i && from.j != to.j ? SQRT_2 * sim->scale : sim->scale;
    double ax = ALPHA * ((double) (sim->curr_grid[to.i][to.j].elevation - sim->curr_grid[from.i][from.j].elevation) / distance);
    double slope_factor = ax / sqrt(1 + ax * ax);
    if (ax < 0)
        slope_factor *= 0.5;

    return slope_factor + 1; 
    //return exp(ALPHA * theta); //Original where ALPHA = 0.078
}

double degree_to_radian(double_t degree_angle)
{
    return degree_angle * PI / 180;
}

// Very hardcoded where wind speed = 5.0 and wind dir = 180º
double compute_wind_angle(FireSimulation *sim, Index2D from, Index2D to)
{
    double wind_angle = 180;
    double wind_angle_rad = degree_to_radian(wind_angle);

    int discrete_x_len = (to.j - from.j);
    int discrete_y_len = (to.i - from.i);
    if (discrete_x_len == 0)
        return discrete_y_len > 0 ? PI * 0.5 : 1.5 * PI;

    double x_len = (double) discrete_x_len * sim->scale;
    double y_len = (double) discrete_y_len * sim->scale;
    double theta = atan2(y_len, x_len);
    if (theta < 0)
        theta += 2 * PI;

    return wind_angle_rad - theta;
}

float compute_p_wind(FireSimulation *sim, Index2D from, Index2D to)
{
    double wind_velocity = 5.0;

    double theta = compute_wind_angle(sim, from, to);
    double f_t = exp(wind_velocity * C_2 * (cos(theta) - 1));
    return exp(C_1 * wind_velocity) * f_t;
}

float compute_p_burn(FireSimulation *sim, Index2D from, Index2D to)
{
    double p_veg = get_p_veg_value(sim->curr_grid[to.i][to.j].land_type);
    double p_distance = compute_p_distance(from, to);
    double p_slope = compute_p_slope(sim, from, to);
    double p_wind = compute_p_wind(sim, from, to);

    return p_veg * p_distance * p_slope * p_wind;
    //return P_H * (1.0 + p_veg) * p_slope * p_wind; //Orignal with different P_VEG values
}

int in_bounds(FireSimulation *sim, Index2D index)
{
    return index.i >= 0 && index.i < sim->grid_height && 
            index.j >= 0 && index.j < sim->grid_width;
}

void spread_fire(FireSimulation *sim, Index2D from, gsl_rng *rand)
{
    Index2D to;
    for (to.i = from.i - 1; to.i <= from.i + 1; to.i++)
        for (to.j = from.j - 1; to.j <= from.j + 1; to.j++)
            if (in_bounds(sim, to) && sim->curr_grid[to.i][to.j].land_state == UNIGNITED)
            {
                double p_burn = compute_p_burn(sim, from, to);
                double random = gsl_rng_uniform(rand);
                if (random < p_burn) 
                    #pragma omp atomic write
                    sim->next_grid[to.i][to.j].land_state = BURNING;
            }
}

void update_grid(FireSimulation *sim)
{
    #pragma omp parallel for num_threads(sim->n_threads)
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            sim->curr_grid[i][j] = sim->next_grid[i][j];
}

void simulate_step(FireSimulation *sim)
{
    unsigned long disturbance = gsl_rng_uniform_int(sim->rand, ~0u);
    #pragma omp parallel num_threads(sim->n_threads)
    {
        gsl_rng *local_rand = sim->thread_rand[omp_get_thread_num()];

        #pragma omp for nowait
        for (int i = 0; i < sim->grid_height; i++)
        {
            gsl_rng_set(local_rand, disturbance + i);
            for (int j = 0; j < sim->grid_width; j++)
                if (sim->curr_grid[i][j].land_state == BURNING)
                {
                    Index2D from = {i, j}; //Compile error otherwise, omp??
                    spread_fire(sim, from, local_rand);
                    sim->next_grid[i][j].land_state = BURNT;
                }                
        }
    }

    update_grid(sim);
}

void write_grid_to_file(FireSimulation *sim, const char *filename)
{
    FILE *f_out = fopen(filename, "w");
    if (f_out == NULL)
    {
        fprintf(stderr, "%s: Error while creating file\n", filename);
        fire_simulation_free(sim);
        exit(1);
    }

    fprintf(f_out, "%d;%d\n", sim->grid_height, sim->grid_width);
    for (int i = 0; i < sim->grid_height; i++)
    {
        fprintf(f_out, "%d,%d,%d", sim->curr_grid[i][0].elevation, sim->curr_grid[i][0].land_type, sim->curr_grid[i][0].land_state);
        for (int j = 1; j < sim->grid_width; j++)
            fprintf(f_out, ";%d,%d,%d", sim->curr_grid[i][j].elevation, sim->curr_grid[i][j].land_type, sim->curr_grid[i][j].land_state);
        fprintf(f_out, "\n");
    }

    fflush(f_out);
    fclose(f_out);
}

void fire_simulate(FireSimulation *sim)
{
    for (int i = 0; i < sim->generations; i++)
        simulate_step(sim);

    if (sim->output_file != NULL)
        write_grid_to_file(sim, sim->output_file);
}