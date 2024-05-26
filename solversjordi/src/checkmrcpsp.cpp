#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mrcpsp.h"
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



void checkSolution(MRCPSP * instance, const vector<int> & starts, const vector<int> & modes){

  error = false;
  errstream << "Non-renewable resources:" << endl;
  errstream << "------------------------------------------------------------" << endl;
  for(int r = instance->getNRenewable(); r < instance->getNResources(); r++){
    int demand = 0;
    for(int i = 1; i <= instance->getNActivities(); i++)
      demand+=instance->getDemand(i,r,modes[i]);

    if(instance->getCapacity(r)<demand){
      errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) << " has demand " << demand << endl;
      error = true;
    }
  }
  errstream << endl;

  errstream << "Renewable resources:" << endl;
  errstream << "------------------------------------------------------------" << endl;
  vector<vector<int> > usages;
  usages.resize(instance->getNRenewable());

  int makespan = starts[instance->getNActivities()+1];

  for(int r = 0; r < instance->getNRenewable(); r++){
    usages[r].resize(makespan,0);
    for(int i = 1; i <= instance->getNActivities(); i++){
      int m = modes[i];
      for(int t = starts[i]; t < starts[i]+instance->getDuration(i,m) && t < makespan; t++)
        usages[r][t]+=instance->getDemand(i,r,m);
    }
    for(int t = 0; t < makespan; t++){
      if(instance->getCapacity(r) < usages[r][t]){
        errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) <<  " has demand " << usages[r][t] << " at time " << t << endl;
        error = true;
      }
    }
  }

  errstream << endl;

  errstream << "Precedences:" << endl;
  errstream << "------------------------------------------------------------" << endl ;
  for(int i = 0; i < instance->getNActivities()+2; i++){
      int m = modes[i];
      for(int suc : instance->getSuccessors(i)){
         if(starts[suc] < starts[i]+instance->getDuration(i,m)){
           errstream << "Error: precedence " << i << " ---> " << suc << " not respected" << endl;
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
	"Check if a solution of the Multi-mode Resource-Constrained Project Scheduling Problem (MRCPSP) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MRCPSP * instance = parser::parseMRCPSP(pargs->getArgument(0));
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


	parser::parseMRCPSPSolution(str,instance,starts,modes);
	checkSolution(instance,starts,modes);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================";
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

