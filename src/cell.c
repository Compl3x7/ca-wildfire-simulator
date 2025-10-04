#include <stdio.h>

#include "cell.h"

#define CLC_URBAN_CUTOFF 11
#define CLC_AGRICUTLTURE_CUTOFF 22
#define CLC_FOREST_CUTOFF 29
#define CLC_WATER_CUTOFF 44
#define CLC_BURNT 33
#define CLC_MIN_VALUE 1
#define CLC_MAX_VALUE 44

LandType clc_to_land_type(int clc_value)
{
    if (clc_value < CLC_MIN_VALUE || clc_value > CLC_MAX_VALUE)
        return INVALID_TYPE;

    if (clc_value == CLC_BURNT)
        return BURNT_LAND;

    if (clc_value <= CLC_URBAN_CUTOFF)
        return URBAN;
    if (clc_value <= CLC_AGRICUTLTURE_CUTOFF)
        return AGRICULTURE;
    if (clc_value <= CLC_FOREST_CUTOFF)
        return FOREST;

    return WATER;
}

double get_p_veg_value(LandType type)
{
    return P_VEG_VALUES[type];
}

LandState determine_land_state(LandType type)
{
    if (type == WATER)
        return UNIGNITABLE;
    else if (type == BURNT_LAND)
        return BURNT;
    else
        return UNIGNITED;
}