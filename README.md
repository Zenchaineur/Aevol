# Mini-Aevol : A mini-application based on the Aevol simulator

A reduced version (from a model and implementation point of view) of Aevol.

DO NOT USE IT TO SIMULATE BIOLOGICAL RESULTS ! See [http://www.aevol.fr](http://www.aevol.fr) for that !

It must be used only to test HPC optimization of the code (parallel, vector, porting to new architecture...).

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes.

### Prerequisites
You will need a unix base system. Sorry for Windows users, maybe try with [Cygwin](http://www.cygwin.com/)

You will also need to install zlib (and its headers):
+ On Debian, Ubuntu, Mint, ... (apt): `zlib1g-dev`
+ On Arch (pacman): `zlib`
+ On Fedora (dnf, rpm): `zlib-devel`

To use cuda implementation, you will need to install the cuda-toolkit:
+ On all distributions: `cuda`

### Compilation

The compilation is straightforward
```
mkdir build
cd build
cmake ..
make
```
It will produced the executable micro_aevol_cpu.

If CUDA toolkit is available on your system, you can build the software with GPU support
```
cmake .. -DUSE_CUDA=on
make
```
It will produced the executable micro_aevol_gpu.

## Running a simulation

A help is given to explain the different parameters when using option `-H` or `--help`.

Basically, you must create a directory to store the simulation files (backup/checkpointing and stats files) and then run the simulation
```
mkdir simulation_example_1
cd simulation_example_1
PATH/TO/micro_aevol_cpu
```

You can also resume a simulation from a backup/checkpointing file (for example, resuming from the generation 1000):
```
cd simulation_example_1
PATH/TO/micro_aevol_cpu -r 1000
```

## Model and Implementation

These [slides](/presentation/slides.pdf) give a short presentation of the model and the purpose of this project can

## Authors

* **Jonathan Rouzaud-Cornabas** - *Initial work*
* **Laurent Turpin**

For the authors of Aevol software, see [http://www.aevol.fr](http://www.aevol.fr)

## License

This project is licensed under the GPLv2 License


## Links

profiler gpu nivdea : https://developer.nvidia.com/nsight-systems
profiler intel vtune : https://www.intel.com/content/www/us/en/develop/documentation/get-started-with-vtune/top.html 

## gprofile
Setup of gprofile
```
mkdir build 
cd build 
cmake -DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg ..
make
```
The result of running gprofile on the default code can be found in the file `gprofil_default_code_analysis.txt`.
When using `omg parallel for` the `-pg` compiler flags should not be used, as this significantly increases the overhead when creating threads.
Therefore use `cmake -DUSE_OMP=On ..`. To evaluate the performance of the program in release mode use `-DCMAKE_BUILD_TYPE=Release`


## version explanation
- v1_0: base version of Aevol as provided by the project instructor
- v1_1: in function `Dna:terminator_at(int pos)` the `length()` of the `_seq` array function is called multiple times in a for loop, while the array length stays constant. Therefore the array length is computed and stored once before the for loop. The total computing time is reduced by about 15%
- parallelization_run_a_step_sebastian (sebastian, branch): In this branch the for loop which updates the simulation in the function run_a_step in ExpManager is parallelized with `pragma omp parallel for`. Initially the performance was lower, due to the overhead of cloning the threads. We then realized this depends on the used cmake compiler flags We used `-DCMAKE_CXX_FLAGS=-pg -DCMAKE_EXE_LINKER_FLAGS=-pg -DCMAKE_SHARED_LINKER_FLAGS=-pg`. This decreased performance only appears when using the `-pg` flag. When compiling without these compiler flags, so compiling with `cmake -DUSE_OMP=On ..`, one can see an improvement. The computation time is roughly decreased about 30% in comparison to the old computation.
- parallelization_promoter_at (branch, Florian) : Profiling results showed that `Dna::promoter_at(int)` was taking a considerable part of
the program's execution time. As a result we tried to parallelize it with OpenMP and see if it could help. The execution time of the function itself was divided by 2, but the overhead caused by thread cloning was too important. Even though we maximized variable sharing, the total execution time went higher than before. Furthermore, the higher the number of threads was, the more time we lost to thread cloning.
Thus, we do not keep this modification.

## Todos

### Florian
* Search for optimizations on `Organism::compute_RNA` and `Organism::search_start_protein`

### Sebastian
* I also try to have a look at `Organism::compute_RNA`. My goal is to improve this function not by using OpenMP but instead restructuring the whole approach of the function. For example only letting it check for new terminators when the values itself changed.

### Mario
* Search for optimizations on `Organism::compute_phenotype` and `Organism::compute_fitness`

## Tests

### `bin/timetest.sh`

Usage :
```bash
bin/timetest.sh -n 500 # any argument given to the script will be forwarded to the Aevol executable
```

This script requires a GNU-compliant version of the `time` command. It is most frequently located at `/usr/bin/time` and is the default location used by the script.
If it is elsewhere, you can override the default value like this :
```bash
TIME_EXEC=/path/to/time bin/timetest.sh
```

In a similar way, you can adjust the number of executions for every branch :
```bash
AMOUNT_TESTS=2 bin/timetest.sh
```

You can also combine these options.
