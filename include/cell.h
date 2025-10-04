#ifndef CELL_H
#define CELL_H

//static const double P_VEG_VALUES[4] = {0.4, -0.3, -0.6, -0.7}; //Original
static const double P_VEG_VALUES[4] = {0.7, 0.5, 0.2, 0.1};

typedef enum LandType
{
    FOREST,
    AGRICULTURE,
    URBAN,
    WATER,
    BURNT_LAND
} LandType;

LandType clc_to_land_type(int clc_value);

double get_p_veg_value(LandType type);

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

#endif