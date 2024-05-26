#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "rcpspt.h"
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

void checkSolution(RCPSPT * instance, const vector<int> & starts){

  error = false;

  errstream << "Resources:" << endl;
  errstream << "------------------------------------------------------------" << endl;
  vector<vector<int> > usages;
  usages.resize(instance->getNResources());

  int makespan = starts[instance->getNActivities()+1];
  if(makespan > instance->getTimeHorizon()){
	errstream << "Error: makespan " << makespan << " is bigger than the time horizon " << instance->getTimeHorizon() << endl;
        error = true;
        makespan = instance->getTimeHorizon();
  }

  for(int r = 0; r < instance->getNResources(); r++){
    usages[r].resize(makespan,0);
    for(int i = 1; i <= instance->getNActivities(); i++){
      for(int t = starts[i]; t < starts[i]+instance->getDuration(i) && t < makespan; t++)
        usages[r][t]+=instance->getDemand(i,r,t-starts[i]);
    }
    for(int t = 0; t < makespan; t++){
      if(instance->getCapacity(r,t) < usages[r][t]){
        errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r,t) <<  " at time " << t << " has demand " << usages[r][t] << " at this time" << endl;
        error = true;
      }
    }
  }

  errstream << endl;

  errstream << "Precedences:" << endl;
  errstream << "------------------------------------------------------------" << endl ;
  for(int i = 0; i < instance->getNActivities()+2; i++){
      for(int suc : instance->getSuccessors(i)){
         if(starts[suc] < starts[i]+instance->getDuration(i)){
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
	"Check if a solution of the Resource-Constrained Project Scheduling Problem with Time-Dependent Recource Capacities and Requests (RCPSP/t) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	RCPSPT * instance = parser::parseRCPSPT(pargs->getArgument(0));
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

	parser::parseRCPSPTSolution(str,instance,starts);
	checkSolution(instance,starts);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================" << endl;
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

