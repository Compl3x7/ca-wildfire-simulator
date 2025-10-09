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