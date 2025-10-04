#include <stdio.h>
#include <stdlib.h>

#include "raster.h"

RasterPoint raster_new_point(float x_coordinate, float y_coordinate, int z_value)
{
    RasterPoint p;
    p.x_coordinate = x_coordinate;
    p.y_coordinate = y_coordinate;
    p.z_value = z_value;
    return p;
}

RasterPoint* raster_read_from_file(const char *filename, int *raster_size)
{
    FILE *raster_file = fopen(filename, "r");
    if (raster_file == NULL)
        return NULL;

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
            free(raster_buffer);
            fclose(raster_file);
            return NULL;
        }

        raster_buffer[size++] = raster_new_point(x_coordinate, y_coordinate, z_value);
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