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