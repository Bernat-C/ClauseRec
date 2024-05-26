#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mmkp.h"
#include "parser.h"
#include "errors.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;
using namespace arguments;

bool error;
stringstream errstream;


/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	VERBOSE
};


void checkSolution(MMKP * instance, const vector<int> & solution){
  error = false;
  for(int i = 0; i < instance->m; i++){
    int demand = 0;
    for(int j = 0; j < instance->n; j++)
		if(solution[j] >= 0)
			demand+=instance->r[j][solution[j]][i];

    if(demand > instance->R[i]){
      errstream << "Error: knapsack capacity of dimension " << (i+1) << ", which is "<< instance->R[i] << ", is surpassed with demand " << demand << endl;
      error = true;
    }
  }
  errstream << endl;
}


int main(int argc, char **argv) {

	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{
	arguments::arg("instance","Instance file path."),
	arguments::arg("solution","Solution file path.")
	},
	1,

	//Program options
	{
	arguments::bop("V","verbose",VERBOSE,false,
	" : show the identified errors. Default 0.")
	},
	"Check if a solution of the Multichoice Multi-dimensional Knapsack Problem (MMKP) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MMKP * instance = parser::parseMMKP(pargs->getArgument(0));
	vector<int> solution;


	ifstream f;
	if(pargs->getNArguments()==2){
		f.open(pargs->getArgument(1).c_str());
		if (!f.is_open()) {
			cerr << "Could not open file " << pargs->getArgument(1) << endl;
			exit(BADFILE_ERROR);
		}
	}

	istream & str = pargs->getNArguments()==2 ? f : cin;

	parser::parseMMKPSolution(str,instance,solution);

	checkSolution(instance,solution);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================";
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

