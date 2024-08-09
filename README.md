# ClauseRec
SAT solving framework designed to make use of deep learning models to enhance the SAT solver's performance through the modification of conflict propagation.

## Installation

The first step in the installation is to update the PYTHONPATH to include the folder containing the python executables and the CPATH to include the location of the Python.h file and enable interlanguage comunication. Make sure to change "user" to the name of your user.
```bash
export PYTHONPATH="$PYTHONPATH:/home/user/ClauseRec/ClauseRecommender/src"
export CPATH="$CPATH:/usr/include/python3.10"
```
The python version 3.10.12 is recommended, but if you are using a different python version you should also update the Makefile of the "solversjordi" folder, specifically the line "LFLAGS += -lpython3.10" to match the python library version you want to use.

### Deep learning Dependencies
If you wish to use the GNN module, you must install the following dependencies:
```bash
pip3 install torch
pip3 install torch_geometric
pip3 install torch_scatter torch_sparse
```

## Running the PRCPSP solver

The two files we have to make are the landing files of the custom prcpsp (prcpsp) and the dummy prcpsp (mrcpsp2smt). To do so we must navigate to the folder $solversjordi$.

```bash
make prcpsp
make mrcpsp2smt
```
The custom-made encoding and the dummy encoding can be run using the following commands respectively.
```bash
./custom-prcpsp instancepath.RCP
./dummy-prcpsp instancepath.RCP
```
Run the PRCPSP checkers by replacing "output.out" by the output of the PRCPSP execution.
```bash
make checkprcpsp
./bin/release/checkprcpsp -V=1 instancepath.RCP output.out
```