#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define CLC_URBAN_CUTOFF 11
#define CLC_AGRICUTLTURE_CUTOFF 22
#define CLC_FOREST_CUTOFF 29
#define CLC_WATER_CUTOFF 44
#define CLC_BURNT 33
#define CLC_MIN_VALUE 1
#define CLC_MAX_VALUE 44

#define P_H 0.58
#define SQRT_2 1.41421356237
#define ALPHA 0.078

typedef struct RasterPoint
{
    float x_cordinate;
    float y_cordinate;
    int z_value;
} RasterPoint;

RasterPoint new_raster_point(float x_cordinate, float y_cordinate, int z_value)
{
    RasterPoint p;
    p.x_cordinate = x_cordinate;
    p.y_cordinate = y_cordinate;
    p.z_value = z_value;
    return p;
}

RasterPoint* read_raster_from_file(const char *filename, int *raster_size)
{
    FILE *raster_file = fopen(filename, "r");
    if (raster_file == NULL)
    {
        printf("%s: File not found/Error while opening file\n", filename);
        exit(0);
    }

    RasterPoint *raster_buffer = malloc(sizeof(RasterPoint));
    int alloc_size = 1;
    int size = 0;
    float x_coordinate;
    float y_coordinate;
    int z_value;
    int read;
    while((read = fscanf(raster_file, "%f %f %d", &x_coordinate, &y_coordinate, &z_value)) != EOF)
    {
        if (read != 3)
        {
            printf("%s: File format doesn't correspond to a xyz raster file\n", filename);
            free(raster_buffer);
            fclose(raster_file);
            exit(0);
        }

        raster_buffer[size++] = new_raster_point(x_coordinate, y_coordinate, z_value);
        if (size >= alloc_size)
        {
            alloc_size *= 2;
            raster_buffer = realloc(raster_buffer, alloc_size * sizeof(RasterPoint));
        }
    }
    raster_buffer = realloc(raster_buffer, size * sizeof(RasterPoint));
    *raster_size = size;
    fclose(raster_file);
    return raster_buffer;
}

typedef enum LandType
{
    FOREST,
    AGRICULTURE,
    URBAN,
    WATER,
    BURNT_LAND
} LandType;

const float P_VEG_VALUES[4] = {0.4, -0.3, -0.6, -0.7};

LandType clc_to_land_type(int clc_value)
{
    if (clc_value == CLC_BURNT)
        return BURNT_LAND;

    if (clc_value <= CLC_URBAN_CUTOFF)
        return URBAN;
    if (clc_value <= CLC_AGRICUTLTURE_CUTOFF)
        return AGRICULTURE;
    if (clc_value <= CLC_FOREST_CUTOFF)
        return FOREST;
//  if (clc_value <= CLC_WATER_CUTOFF)

    return WATER;
}

typedef enum LandState
{
    UNIGNITABLE,
    UNIGNITED,
    BURNING,
    BURNT
} LandState;

typedef struct Cell
{
    int elevation;
    LandType land_type;
    LandState land_state;
} Cell;

typedef struct FireSimulation
{
    unsigned int base_seed;
    int grid_height;
    int grid_width;
    float min_x;
    float min_y;
    float scale;
    Cell **curr_grid;
    Cell **next_grid;
    int generations;
} FireSimulation;

// TODO Write usage message and improve logic and error handling
void parse_args(FireSimulation *sim, int argc, char **argv, char **elevation_raster_file_name, char **land_cover_raster_file_name)
{
    *elevation_raster_file_name = NULL;
    *land_cover_raster_file_name = NULL;
    sim->scale = 0.00;
    sim->generations = 100;
    sim->base_seed = time(NULL);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 && i + 1 < argc)
            *elevation_raster_file_name = argv[++i];
        else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc)
            *land_cover_raster_file_name = argv[++i];
        else if (strcmp(argv[i], "--scale") == 0 && i + 1 < argc)
            sim->scale = atof(argv[++i]);
        else if (strcmp(argv[i], "--gens") == 0 && i + 1 < argc)
            sim->generations = atoi(argv[++i]);
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
            sim->base_seed = atoi(argv[++i]);
        else
        {
            printf("Correct usage message placeholder\n");
            exit(0);
        }
    }

    if (elevation_raster_file_name == NULL || land_cover_raster_file_name == NULL 
        || sim->scale <= 0 || sim->generations <= 0 || sim->base_seed < 0)
    {
        printf("Correct usage message placeholder\n");
        exit(0); 
    }
}

// TODO Improve raster's validation
void compute_grid_size(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    float elevation_min_x = elevation_raster[0].x_cordinate;
    float elevation_max_x = elevation_raster[0].x_cordinate;
    float elevation_min_y = elevation_raster[0].y_cordinate;
    float elevation_max_y = elevation_raster[0].y_cordinate;
    float land_cover_min_x = land_cover_raster[0].x_cordinate;
    float land_cover_max_x = land_cover_raster[0].x_cordinate;
    float land_cover_min_y = land_cover_raster[0].y_cordinate;
    float land_cover_max_y = land_cover_raster[0].y_cordinate;
    for (int i = 1; i < size; i++)
    {
        RasterPoint elevation_curr = elevation_raster[i];
        RasterPoint land_cover_curr = land_cover_raster[i];
        if (elevation_curr.x_cordinate < elevation_min_x)
            elevation_min_x = elevation_curr.x_cordinate;
        else if (elevation_curr.x_cordinate > elevation_max_x)
            elevation_max_x = elevation_curr.x_cordinate;

        if (elevation_curr.y_cordinate < elevation_min_y)
            elevation_min_y = elevation_curr.y_cordinate;
        else if (elevation_curr.y_cordinate > elevation_max_y)
            elevation_max_y = elevation_curr.y_cordinate;

        if (land_cover_curr.x_cordinate < land_cover_min_x)
            land_cover_min_x = land_cover_curr.x_cordinate;
        else if (land_cover_curr.x_cordinate > land_cover_max_x)
            land_cover_max_x = land_cover_curr.x_cordinate;

        if (land_cover_curr.y_cordinate < land_cover_min_y)
            land_cover_min_y = land_cover_curr.y_cordinate;
        else if (land_cover_curr.y_cordinate > land_cover_max_y)
            land_cover_max_y = land_cover_curr.y_cordinate;
    }

    if (elevation_min_x != land_cover_min_x || elevation_max_x != land_cover_max_x ||
        elevation_min_y != land_cover_min_y || elevation_max_y != land_cover_max_y)
    {
        printf("Raster file boundaries do not correspond\n");
        free(elevation_raster);
        free(land_cover_raster);
        exit(0);
    }

    sim->grid_height = (int) ((elevation_max_x - elevation_min_x) / sim->scale) + 1;
    sim->grid_width = (int) ((elevation_max_y - elevation_min_y) / sim->scale) + 1;
    sim->min_x = elevation_min_x;
    sim->min_y = elevation_min_y;
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

// TODO Improve raster's validation
void populate_grids(FireSimulation *sim, RasterPoint *elevation_raster, RasterPoint *land_cover_raster, int size)
{
    if (sim->grid_height * sim->grid_width != size)
    {
        printf("Computed grid size doesn't correspond to the size of given raster files\n");
        free(elevation_raster);
        free(land_cover_raster);
        free_grids(sim);
        exit(0);
    }

    for (int i = 0; i < size; i++)
    {
        RasterPoint elevation_curr = elevation_raster[i];
        RasterPoint land_cover_curr = land_cover_raster[i];
        int elevation_j = (int) (elevation_curr.x_cordinate - sim->min_x) / sim->scale;
        int elevation_k = sim->grid_width - 1 - (int) (elevation_curr.y_cordinate - sim->min_y) / sim->scale;
        int land_cover_j = (int) (land_cover_curr.x_cordinate - sim->min_x) / sim->scale;
        int land_cover_k = sim->grid_width - 1 - (int) (land_cover_curr.y_cordinate - sim->min_y) / sim->scale;

        // Since indices might not correspond we change values manually insted of using a new_cell func
        sim->curr_grid[elevation_j][elevation_k].elevation = elevation_curr.z_value;

        if (land_cover_curr.z_value < CLC_MIN_VALUE || land_cover_curr.z_value > CLC_MAX_VALUE)
        {
            printf("Invalid land cover value on land cover raster file\n");
            free(elevation_raster);
            free(land_cover_raster);
            free_grids(sim);
            exit(0);
        }

        LandType type = clc_to_land_type(land_cover_curr.z_value);
        sim->curr_grid[land_cover_j][land_cover_k].land_type = type;
        if (type == WATER)
            sim->curr_grid[land_cover_j][land_cover_k].land_state = UNIGNITABLE;
        else if (type == BURNT_LAND)
            sim->curr_grid[land_cover_j][land_cover_k].land_state = BURNT;
        else
            sim->curr_grid[land_cover_j][land_cover_k].land_state = UNIGNITED;
    }
}

// Aux struct
typedef struct Point
{
    int i;
    int j;
} Point;

void set_random_ignition_point(FireSimulation *sim)
{
    // TODO VLA BAD
    int size = 0;
    Point valid_points[sim->grid_height * sim->grid_width];
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            if (sim->curr_grid[i][j].land_state == UNIGNITED)
            {
                valid_points[size].i = i;
                valid_points[size].j = j;
                size++;
            }

    int random = rand() % size;
    Point selected = valid_points[random];
    sim->curr_grid[selected.i][selected.j].land_state = BURNING;
}

void copy_to_next_grid(FireSimulation *sim)
{
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            sim->next_grid[i][j] = sim->curr_grid[i][j];
}

void init_fire_simulation(FireSimulation *sim, int argc, char **argv)
{
    char *elevation_raster_file_name;
    char *land_cover_raster_file_name;
    parse_args(sim, argc, argv, &elevation_raster_file_name, &land_cover_raster_file_name);

    int elevation_raster_size;
    int land_cover_raster_size;
    RasterPoint *elevation_raster = read_raster_from_file(elevation_raster_file_name, &elevation_raster_size);
    RasterPoint *land_cover_raster = read_raster_from_file(land_cover_raster_file_name, &land_cover_raster_size);
    if (elevation_raster_size != land_cover_raster_size)
    {
        printf("Raster file sizes do not correspond\n");
        free(elevation_raster);
        free(land_cover_raster);
        exit(0);
    }
    int size = elevation_raster_size; // elevation_raster_size == land_cover_raster_size

    srand(sim->base_seed);    
    compute_grid_size(sim, elevation_raster, land_cover_raster, size);
    alloc_grids(sim);
    populate_grids(sim, elevation_raster, land_cover_raster, size);
    set_random_ignition_point(sim);
    copy_to_next_grid(sim);
    
    //Free temp buffers
    free(elevation_raster);
    free(land_cover_raster);
}

int in_bounds(FireSimulation *sim, int i, int j)
{
    return i >= 0 && i < sim->grid_height && 
            j >= 0 && j < sim->grid_width;
}

float compute_p_slope(FireSimulation *sim, int i, int j, int k, int l)
{
    float distance = (i != k && j != l) ? SQRT_2 * sim->scale : sim->scale;
    float theta = atan((float) (sim->curr_grid[k][l].elevation - sim->curr_grid[i][j].elevation) / distance);
    return exp(ALPHA * theta);
}

float compute_p_burn(FireSimulation *sim, int i, int j, int k, int l)
{
    float p_veg = P_VEG_VALUES[sim->curr_grid[k][l].land_type];
    float p_slope = compute_p_slope(sim, i, j, k, l);
    float p_wind = 1.0; //No effect by now

    return P_H * (1.0 + p_veg) * p_slope * p_wind;
}

void spread_fire(FireSimulation *sim, int i, int j)
{
    for (int k = i - 1; k <= i + 1; k++)
        for (int l = j - 1; l <= j + 1; l++)
            if (in_bounds(sim, k, l) && sim->curr_grid[k][l].land_state == UNIGNITED && sim->next_grid[k][l].land_state == UNIGNITED)
            {
                float p_burn = compute_p_burn(sim, i, j, k, l);
                float random = (float) rand() / RAND_MAX;
                if (random < p_burn) 
                    sim->next_grid[k][l].land_state = BURNING;
            }
}

void update_grid(FireSimulation *sim)
{
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            sim->curr_grid[i][j] = sim->next_grid[i][j];
}

void simulate(FireSimulation *sim)
{
    for (int i = 0; i < sim->grid_height; i++)
        for (int j = 0; j < sim->grid_width; j++)
            if (sim->curr_grid[i][j].land_state == BURNING)
            {
                spread_fire(sim, i, j);
                sim->next_grid[i][j].land_state = BURNT;
            }

    update_grid(sim);
}

void print_grid(FireSimulation *sim)
{
    for (int i = 0; i < sim->grid_height; i++)
    {
        for (int j = 0; j < sim->grid_width; j++)
            printf("[%d]", sim->curr_grid[i][j].land_state);
        printf("\n");
    }
}

void write_grid_to_file(FireSimulation *sim, char *filename)
{
    FILE *f_out = fopen(filename, "w");
    if (f_out == NULL)
    {
        printf("%s: Error while creating file\n", filename);
        free_grids(sim);
        exit(0);
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

int main(int argc, char **argv)
{
    FireSimulation sim;

    // TODO Implement file output logic

    init_fire_simulation(&sim, argc, argv);

    // print_grid(&sim);
    // printf("\n");

    for (int i = 0 ; i < sim.generations; i++)
        simulate(&sim);

    write_grid_to_file(&sim, "results/seed_0_gens_default.csv");

    // print_grid(&sim);
    // printf("\n");
    
    free_grids(&sim);

    return 0;
}