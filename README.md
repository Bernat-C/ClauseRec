# ClauseRec
SAT solving framework to use deep learning models to enhance a SAT solver performance by modifying conflict propagation.

Update the PYTHONPATH to include the folder "ClauseRecommender/src". You can do it using the following command. Make sure to change "user" to the name of your user.
```python
export PYTHONPATH='/home/user/ClauseRec/ClauseRecommender/src'
```
Update the CPATH to include the folder where the desired Python installation you want to use is located:
```python
export CPATH=":/usr/include/python3.10"
```
If you are using a different python version you should also update the Makefile of the "solversjordi" folder, specifically the line "LFLAGS += -lpython3.10" to match the python library version you want to use. Then you can make the executables by doing:
```python
cd solversjordi
make DEBUG=0 -j 16 prcpsp
```
And finally execute any RCP instance using
```python
./bin/release/prcpsp2dimacs instancepath.RCP -s=maple --use-assumptions=0 --print-nonoptimal=0 -o=ub
```
