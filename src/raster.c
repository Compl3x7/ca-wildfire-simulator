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

RasterPoint* raster_resize(RasterPoint *raster, int *old_alloc_size, int new_alloc_size)
{
    if (old_alloc_size != NULL)
        *old_alloc_size = new_alloc_size;
    
    return realloc(raster, new_alloc_size * sizeof(RasterPoint));
}

RasterPoint* raster_read_from_file(const char *filename, int *raster_size)
{
    FILE *raster_file = fopen(filename, "r");
    if (raster_file == NULL)
        return NULL;

    RasterPoint *raster = malloc(sizeof(RasterPoint));
    int alloc_size = 1;
    int size = 0;
    RasterPoint p;
    int read;
    while((read = fscanf(raster_file, "%f %f %d", &p.x_coordinate, &p.y_coordinate, &p.z_value)) != EOF)
    {
        if (read != 3)
        {
            free(raster);
            fclose(raster_file);
            return NULL;
        }

        if (size >= alloc_size)
            raster = raster_resize(raster, &alloc_size, alloc_size * 2);

        raster[size++] = p;
    }
    raster = raster_resize(raster, NULL, size);
    *raster_size = size;
    fclose(raster_file);
    return raster;
}

RasterBound raster_get_bound(RasterPoint *raster, int size)
{
    RasterBound bound;
    bound.min_x = raster[0].x_coordinate;
    bound.max_x = raster[0].x_coordinate;
    bound.min_y = raster[0].y_coordinate;
    bound.max_y = raster[0].y_coordinate;
    for (int i = 1; i < size; i++)
    {
        bound.min_x = raster[i].x_coordinate < bound.min_x ? raster[i].x_coordinate : bound.min_x;
        bound.max_x = raster[i].x_coordinate > bound.max_x ? raster[i].x_coordinate : bound.max_x;
        bound.min_y = raster[i].y_coordinate < bound.min_y ? raster[i].y_coordinate : bound.min_y;
        bound.max_y = raster[i].y_coordinate > bound.max_y ? raster[i].y_coordinate : bound.max_y;
    }
    return bound;
}

int raster_equal_bounds(RasterBound b1, RasterBound b2)
{
    int equal_min_x = b1.min_x == b2.min_x;
    int equal_max_x = b1.max_x == b2.max_x;
    int equal_min_y = b1.min_y == b2.min_y;
    int equal_max_y = b1.max_y == b2.max_y;
    
    return equal_min_x && equal_max_x && equal_min_y && equal_max_y;
}