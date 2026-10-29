#    Wildfire simulator using Probabilistic Cellular Automata (PCA)
#    Copyright (C) 2025-2026  Alexandre Rodrigues
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.

import sys
import numpy as np
from PIL import Image, ImageColor
from enum import IntEnum

class LandState(IntEnum):
    UNIGNITABLE = 0,
    UNIGNITED = 1,
    BURNING = 2,
    BURNT = 3

class LandType(IntEnum):
    FOREST = 0
    AGRICULTURE = 1
    URBAN = 2
    WATER = 3
    BURNT_LAND = 4


def read_grid(filename):
   with open(filename) as f:

    line = f.readline().strip("\n")
    split_line = line.split(";", 2)
    
    #For now no altitude visualization
    grid = np.empty(shape=(int(split_line[0]), int(split_line[1]), 2), dtype=int)
    for i in range(grid.shape[0]):
        line = f.readline().strip("\n")
        split_line = line.split(";")
        for j in range(grid.shape[1]):
            values = split_line[j].split(",")
            grid[i][j][0] = int(values[1])
            grid[i][j][1] = int(values[2])

    return grid

def define_color(values):
    if (values[1] == LandState.BURNING.value):
        return (255, 0, 0) #RED
    if (values[1] == LandState.BURNT.value):
        return (0, 0, 0) #BLACK
    
    if (values[0] == LandType.FOREST.value):
        return (0, 128, 0) #DARK GREEN
    if (values[0] == LandType.AGRICULTURE.value):
        return (0, 255, 0) #GREEN
    if (values[0] == LandType.URBAN.value):
        return (64, 64, 64) #DARK GRAY
    if (values[0] == LandType.WATER.value):
        return (0, 128, 255) #DARK CYAN

#TODO Argument parsing
    
grid = read_grid(sys.argv[1])
scaling_factor = int(sys.argv[3])

img = Image.new(mode="RGB", size=(grid.shape[1], grid.shape[0]))
pixels = img.load()

for j in range(grid.shape[1]):
    for i in range(grid.shape[0]):
        pixels[j, i] = define_color(grid[i][j])

img = img.resize(size=(scaling_factor * grid.shape[1], scaling_factor * grid.shape[0]), resample=Image.Resampling.NEAREST)
img.save(sys.argv[2])