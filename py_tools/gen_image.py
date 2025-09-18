import numpy as np
from PIL import Image

def read_grid(filename):
   with open(filename) as f:

    line = f.readline().strip("\n")
    split_line = line.split(";", 2)
    
    grid = np.empty(shape=(int(split_line[0]), int(split_line[1])), dtype=int)
    for i in range(grid.shape[0]):
        line = f.readline().strip("\n")
        split_line = line.split(";")
        for j in range(grid.shape[1]):
            values = split_line[j].split(",")
            grid[i][j] = int(values[2])

    return grid

colors = [
    (0, 0, 0),
    (0, 255, 0),
    (255, 0, 0),
    (0, 0, 0)
]

grid = read_grid("result.csv")

img = Image.new(mode="RGB", size=(grid.shape[1], grid.shape[0]))
pixels = img.load()

for i in range(grid.shape[0]):
    for j in range(grid.shape[1]):
        pixels[j, i] = colors[grid[i][j]]

img = img.resize(size=(10 * grid.shape[1], 10 * grid.shape[0]), resample=Image.Resampling.NEAREST)
img.save('result.png')