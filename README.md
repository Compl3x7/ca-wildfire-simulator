# Wildfire simulator using Probabilistic Cellular Automata (PCA)

A C implementation of a wildfire spread simulator based on the Probabilistic Cellular Automaton (PCA) model of Alexandridis et al. and other related studies, with parallel execution via OpenMP.

## Overview

This simulator models wildfire propagation across a heterogeneous landscape using a probabilistic cellular automaton. A cell's state may evolve stochastically or deterministically: the `UNIGNITED` → `BURNING` transition is probabilistic, with ignition probability derived from vegetation type, slope, wind, and neighboring states, while the `BURNING` → `BURNT` transition is deterministic, following the Alexandridis et al. model.

## Features

- PCA wildfire model based on Alexandridis et al. and related studies (see [Model reference](#model-reference))
- Support for `.xyz` raster files for elevation data (SRTM) and land cover data (CORINE)
- State transitions driven by vegetation type, slope, wind (global vector), and neighborhood
- Parallelized execution using OpenMP
- Output simulation results to `.csv` files
- Python tools for visualizing simulation output

## Project structure

```
ca-wildfire-simulator/
├── build/          # Compiled binaries
├── demo_data/      # Sample input data for demo use
├── include/        # Header (.h) files
├── py_tools/       # Python helper scripts (e.g. gen_image.py)
├── src/            # C source code
│   └── ...         # PCA core, cell states, raster structs...
├── Makefile
└── README.md
```

`build/` is created automatically during the first build.

## Dependencies

| Dependency | Type     | Purpose                                         |
|------------|----------|-------------------------------------------------|
| `gcc`      | Build    | C compiler (C99+)                               |
| GNU Make   | Build    | Build automation (via `Makefile`)               |
| GSL        | Runtime  | Numerical routines and random number generation |
| GSL CBLAS  | Runtime  | BLAS backend used by GSL                        |
| OpenMP     | Runtime  | Shared-memory parallelization                   |
| `libm`     | Runtime  | Standard C math library                         |
| Python 3   | Optional | Running scripts in `py_tools/` (visualization)  |

## Build

The included Makefile supports the following targets:

```bash
make            # Build the simulator (same as make all)
make clean      # Remove all binaries and the build directory
```

## Usage

Run the simulator `wildfire` located in `build/` with the required arguments. Use `--help` for the full list.

```
$ ./build/wildfire --help
Usage: ./wildfire --dem <file> --clc <file> --w-angle <degrees> --w-speed <m/s> --scale <meters> [OPTIONS]

REQUIRED ARGUMENTS:
        --dem <file>            Digital Elevation Model in XYZ format
        --clc <file>            Land cover data in XYZ format
        --w-angle <degrees>     Wind angle [0-360] with respect to the horizontal east axis
        --w-speed <m/s>         Wind speed in meters per second
        --scale <meters>        Cell size in meters (e.g., 25.0)

OPTIONAL ARGUMENTS:
        --gens <N>              Number of time steps (default: 100)
        --seed <value>          Random seed (default: system time)
        --n-threads <N>         Number of threads to use (default: 2)
        --out <file>            Output CSV file (default: no output written)
```

The `demo_data/` directory includes some sample data ready for use. A minimal example run:

```
$ ./build/wildfire --dem demo_data/demo_dem_25m.xyz --clc demo_data/demo_clc_25m.xyz --w-angle 0 --w-speed 7.5 --scale 25
Total time: 0.295359s
Init time: 0.228275s
Sim time: 0.067069s
Free time: 0.000015s
```

The program outputs execution times for the total execution and its initialization, simulation, and finalization stages. These measurements are useful for benchmarking the model across configurations.

## Using your own data

The simulator accepts any Digital Elevation Model (DEM) and land cover (CLC) raster that is provided in the expected `.xyz` format, provided both rasters share the same grid, that is, the same origin, Coordinate Reference System (CRS), cell size (scale), and dimensions. Otherwise, the program exits with a corresponding error message. As a rule of thumb, verify that both rasters share the same grid before running the simulator. The following steps briefly outline how to obtain, align, export, and run the simulator with custom data.

### 1. Digital elevation model (DEM)

Obtain a DEM for your area (e.g., [SRTM](https://earthexplorer.usgs.gov)).

### 2. Land cover (CLC)

Obtain land cover data for your area from [CORINE](https://land.copernicus.eu/en/products/corine-land-cover). The current version only supports CORINE land cover data.

### 3. Aligning and exporting to `.xyz` format

Geographic Information Systems (GIS) software such as [QGIS](https://www.qgis.org/download/) can aid in both aligning and exporting the files. The `.xyz` files should follow the whitespace-separated format `<x> <y> <value>` for each line and contain no header. Ensure the scales also match, by either upscaling the raster with the larger cell size or downscaling the other.

### 4. Running with your data

```
./build/wildfire --dem path/to/dem.xyz --clc path/to/clc.xyz --w-angle 90 --w-speed 5.0 --scale 100
```

Set `--scale` according to the cell size of your rasters (in meters).

## Demo data sources

The data in `demo_data/` is a small, arbitrarily chosen subset of the following sources, at a scale of 25 m:

- [Portugal DEM](https://www.fc.up.pt/pessoas/jagoncal/dems/)
- [CLC 2018](https://land.copernicus.eu/en/products/corine-land-cover/clc2018)

## Model reference

This project builds on the following studies:

- [Alexandridis et al. (2008), *A cellular automata model for forest fire spread prediction: The case of the wildfire that swept through Spetses Island in 1990*](https://doi.org/10.1016/j.amc.2008.06.046)
- [Alexandridis et al. (2011), *Wildland fire spread modelling using cellular automata: evolution in large-scale spatially heterogeneous environments under fire suppression tactics*](https://doi.org/10.1071/WF09119)
- [Freire & DaCamara (2018), *Using cellular automata to simulate wildfire propagation and to assist in fire management*](https://doi.org/10.5194/nhess-19-169-2019)
- [Mota (2019), *Modelling propagation of wildfires using cellular automata*](https://fenix.tecnico.ulisboa.pt/downloadFile/563345090417473/Thesis_Barbara_Mota_final.pdf)

## Future work (unordered, subject to change)

- [ ] **GUI** - implement a graphical user interface.
- [ ] **Ensemble simulations** - support ensembles of simulations to obtain probabilistic burn maps.
- [ ] **Contiguous grid storage** - optimize grids to be stored as contiguous 1D arrays, rather than the current 2D approach.
- [ ] **GPU acceleration** - parallelize the model on GPUs with CUDA/HIP.
- [ ] **I/O performance** - initial I/O is currently a bottleneck.
- [ ] **Input initial state** - allow the user to supply an initial grid state, rather than relying on a random selection of ignition points.
- [ ] **Wind model** - implement wind with a per-cell wind vector to support real wind data, possibly including wind changes over time.
- [ ] **Model realism** - improve overall model realism, in tandem with the wind model improvements.

## License

This project is licensed under the GNU General Public License v3.0 (GPLv3). See [LICENSE](LICENSE) for details.
