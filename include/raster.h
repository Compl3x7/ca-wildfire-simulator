#ifndef RASTER_H
#define RASTER_H

typedef struct RasterPoint
{
    float x_coordinate;
    float y_coordinate;
    int z_value;
} RasterPoint;

RasterPoint *raster_read_from_file(const char *filename, int *raster_size);

#endif