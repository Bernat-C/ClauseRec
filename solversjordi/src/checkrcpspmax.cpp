#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "rcpspmax.h"
#include "errors.h"
#include "parser.h"
#include "arguments.h"
#include "solvingarguments.h"

using namespace std;
using namespace arguments;

bool error;
stringstream errstream;


/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	VERBOSE
};



void checkSolution(RCPSPMAX * instance, const vector<int> & starts){
	int N = instance->getNActivities();
	error = false;

	int H = INT_MIN;
	for(int i = 0; i < N+2; i++)
		if(H < starts[i]+instance->getDuration(i))
			H = starts[i]+instance->getDuration(i);

	if(H!=starts[N+1]){
		error = true;
		errstream << "Error: S_" << (N+1) << starts[N+1] << " is not the makespan=" << H << " !" << endl;
	}

	errstream << "Resources:" << endl;
	errstream << "------------------------------------------------------------" << endl;
	vector<vector<int> > usages(instance->getNResources(),vector<int>(H,0));
	for(int r = 0; r < instance->getNResources(); r++){
		for(int i = 1; i <= N; i++)
			for(int t = starts[i]; t < starts[i]+instance->getDuration(i); t++)
				usages[r][t]+=instance->getDemand(i,r);

		for(int t = 0; t < H; t++){
			if(instance->getCapacity(r) < usages[r][t]){
				errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) <<  " has demand " << usages[r][t] << " at time " << t << endl;
				error = true;
			}
		}
	}

	errstream << endl;

	errstream << "Precedences:" << endl;
	errstream << "------------------------------------------------------------" << endl ;
	for(int i = 0; i <= N+1; i++){
		for(int j : instance->getSuccessors(i)){
			if(starts[j] < starts[i]+instance->getTimeLag(i,j)){
				errstream << "Error: precedence A_" << i << " --(" << instance->getTimeLag(i,j) << ")--> A_" << j << " not respected" << endl;
				error = true;
			}
		}
	}
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
	"Check if a solution of the Resource-Constrained Project Scheduling Problem with Minimun and Maximum Time Lags (RCPSP/max) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	RCPSPMAX * instance = parser::parseRCPSPMAX(pargs->getArgument(0));
	vector<int> starts;

	ifstream f;
	if(pargs->getNArguments()==2){
		f.open(pargs->getArgument(1).c_str());
		if (!f.is_open()) {
			cerr << "Could not open file " << pargs->getArgument(1) << endl;
			exit(BADFILE_ERROR);
		}
	}

	istream & str = pargs->getNArguments()==2 ? f : cin;

	parser::parseRCPSPMAXSolution(str,instance,starts);
	checkSolution(instance,starts);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================";
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

