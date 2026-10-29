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

#ifndef RASTER_H
#define RASTER_H

typedef struct RasterPoint
{
    float x_coordinate;
    float y_coordinate;
    int z_value;
} RasterPoint;

RasterPoint* raster_read_from_file(const char *filename, int *raster_size);

typedef struct RasterBound
{
    float min_x;
    float max_x;
    float min_y;
    float max_y;
} RasterBound;

RasterBound raster_get_bound(RasterPoint *raster, int size);

int raster_equal_bounds(RasterBound b1, RasterBound b2);

#endif