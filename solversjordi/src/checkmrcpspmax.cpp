#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mrcpspmax.h"
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



void checkSolution(MRCPSPMAX * instance, const vector<int> & starts, const vector<int> & modes){
	int N = instance->getNActivities();
	error = false;

	int H = INT_MIN;
	for(int i = 0; i < N+2; i++)
		if(H < starts[i]+instance->getDuration(i,modes[i]))
			H = starts[i]+instance->getDuration(i,modes[i]);

	if(H!=starts[N+1]){
		error = true;
		errstream << "Error: S_" << (N+1) << starts[N+1] << " is not the makespan=" << H << " !" << endl;
	}

	errstream << "Non-renewable resources:" << endl;
	errstream << "------------------------------------------------------------" << endl;
	for(int r = instance->getNRenewable(); r < instance->getNResources(); r++){
		int demanda = 0;
		for(int i = 0; i <= N+1; i++)
			demanda+=instance->getDemand(i,r,modes[i]);
		if(instance->getCapacity(r)<demanda){
			errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) << " has demand " << demanda << endl;
			error = true;
		}
	}
	errstream << endl;

	errstream << "Renewable resources:" << endl;
	errstream << "------------------------------------------------------------" << endl;
	vector<vector<int> > usos(instance->getNRenewable(),vector<int>(H,0));
	for(int r = 0; r < instance->getNRenewable(); r++){
		for(int i = 1; i <= N; i++)
			for(int t = starts[i]; t < starts[i]+instance->getDuration(i,modes[i]); t++)
				usos[r][t]+=instance->getDemand(i,r,modes[i]);

		for(int t = 0; t < H; t++){
			if(instance->getCapacity(r) < usos[r][t]){
				errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) <<  " has demand " << usos[r][t] << " at time " << t << endl;
				error = true;
			}
		}
	}

	errstream << endl;

	errstream << "Precedences:" << endl;
	errstream << "------------------------------------------------------------" << endl ;
	for(int i = 0; i <= N+1; i++){
		for(int j : instance->getSuccessors(i)){
			if(starts[j] < starts[i]+instance->getTimeLag(i,j,modes[i],modes[j])){
				errstream << "Error: precedence " << i << " --(" << instance->getTimeLag(i,j,modes[i],modes[j]) << ")--> " << j << " not respected" << endl;
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
	"Check if a solution of the Multi-mode Resource-Constrained Project Scheduling Problem with Minimum and Maximum Time Lags (MRCPSP/max) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MRCPSPMAX * instance = parser::parseMRCPSPMAX(pargs->getArgument(0));
	vector<int> starts,modes;

	ifstream f;
	if(pargs->getNArguments()==2){
		f.open(pargs->getArgument(1).c_str());
		if (!f.is_open()) {
			cerr << "Could not open file " << pargs->getArgument(1) << endl;
			exit(BADFILE_ERROR);
		}
	}

	istream & str = pargs->getNArguments()==2 ? f : cin;

	parser::parseMRCPSPMAXSolution(str,instance,starts,modes);
	checkSolution(instance,starts,modes);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================";
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

