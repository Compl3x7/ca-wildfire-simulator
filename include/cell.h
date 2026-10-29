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

#ifndef CELL_H
#define CELL_H

static const double P_VEG_VALUES[4] = {0.4, -0.3, -0.6, -0.7}; //Original
// static const double P_VEG_VALUES[4] = {0.7, 0.5, 0.2, 0.1};

typedef enum LandType
{
    FOREST,
    AGRICULTURE,
    URBAN,
    WATER,
    BURNT_LAND,
    INVALID_TYPE
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

LandState determine_land_state(LandType type);

typedef struct Cell
{
    int elevation;
    LandType land_type;
    LandState land_state;
} Cell;

#endif