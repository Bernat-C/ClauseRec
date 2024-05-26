#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "assert.h"
#include "prcpsp.h"
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



void checkSolution(PRCPSP * instance, const vector<int> & starts, const vector<int> &ends, const vector<vector<int>> &is_exec){

  errstream << "Renewable resources:" << endl;
  errstream << "------------------------------------------------------------" << endl;
  vector<vector<int> > usages;
  usages.resize(instance->getNRenewable());

  int makespan = starts[instance->getNActivities()+1];

  for(int r = 0; r < instance->getNRenewable(); r++){
    usages[r].resize(makespan,0);

    for(int t = 0; t < makespan; t++){
      usages[r][t] = 0;
      for(int i = 1; i <= instance->getNActivities(); i++){
        if(is_exec[i][t]==1)
          usages[r][t]+=instance->getDemand(i,r);
      }
      if(instance->getCapacity(r) < usages[r][t]){
        errstream << "Error: resource " << r << " with capacity " << instance->getCapacity(r) <<  " has demand " << usages[r][t] << " at time " << t << endl;
        for(int i = 1; i <= instance->getNActivities(); i++)
          if(is_exec[i][t]==1)
            errstream << i << " utilitza" << instance->getDemand(i,r) << std::endl;
        error = true;
      }
    }
  }
  
  errstream << endl;

  errstream << "Execucio:" << endl;
  errstream << "------------------------------------------------------------" << endl ;
  for(int i = 0; i < instance->getNActivities()+2; i++){
      int count = 0;
      for(int t = 0; t< makespan; t++){
         count += is_exec[i][t];
      }
      if(count < instance->getDuration(i)){
        errstream << "Error: activity " << i << " is running for " << count << " times and should be running for " << instance->getDuration(i) << " times." << endl;
        error = true;
      }
  }

  errstream << endl;

  errstream << "Precedences:" << endl;
  errstream << "------------------------------------------------------------" << endl ;
  for(int i = 0; i < instance->getNActivities()+2; i++){
      for(int suc : instance->getSuccessors(i)){
         if(instance->getDuration(i) > 0 && starts[suc] <= ends[i]){
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
	"Check if a solution of the Preemptive Resource-Constrained Project Scheduling Problem (PRCPSP) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	PRCPSP * instance = parser::parsePRCPSPfromRCP(pargs->getArgument(0));
	vector<int> starts,modes,ends;
  vector<vector<int>> is_exec;

	ifstream f;
	if(pargs->getNArguments()==2){
		f.open(pargs->getArgument(1).c_str());
		if (!f.is_open()) {
			cerr << "Could not open file " << pargs->getArgument(1) << endl;
			exit(BADFILE_ERROR);
		}
	}

	istream & str = pargs->getNArguments()==2 ? f : cin;


	parser::parsePRCPSPSolution(str,instance,starts,modes,ends,is_exec);
	checkSolution(instance,starts,ends,is_exec);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << " in makespan " << starts[starts.size()-1] << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================" << endl;
		cout << errstream.str();
		cout << "============================================================" << endl;
  }

	return 0;
}

