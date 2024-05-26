#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include "mspsp.h"
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

void checkSolution(MSPSP * instance, const vector<int> & starts, const vector<vector<pair<int,int> > > & assignments){
	error = false;

	int makespan = starts[instance->getNActivities()+1];
	for(int i = 0; i < instance->getNActivities()+2; i++){
		if(starts[i]+instance->getDuration(i) > makespan){
			errstream << "Error: the closing activity is not the last one to finish. Makespan is " << makespan << " and activity " << (i+1) << " ends at " << (starts[i]+instance->getDuration(i)) << endl;
			error = true;
		}
	}


  errstream << "Resources:" << endl;
  errstream << "------------------------------------------------------------" << endl;

	int ** activity_skill = new int * [instance->getNActivities()+2];
	bool ** activity_resource = new bool * [instance->getNActivities()+2];
	for(int i = 0; i < instance->getNActivities()+2; i++){

		activity_resource[i] = new bool[instance->getNResources()];
		for(int k = 0; k < instance->getNResources(); k++)
            activity_resource[i][k]=false;


		activity_skill[i] = new int[instance->getNSkills()];
		for(int l = 0; l < instance->getNSkills(); l++)
            activity_skill[i][l]=instance->getDemand(i,l);

	}

	for(int i = 0; i < instance->getNActivities()+2; i++){
		for(const pair<int,int> & p : assignments[i]){
			int k = p.first;
			int l = p.second;
			if(!instance->hasSkill(k,l)){
				errstream << "Error: resource " << k <<
					" cannot perform skill " << l << " in activity " << (i+1) <<
					" since it does not master it." << endl;
				error = true;
			}

			if(activity_resource[i][k]){
				errstream << "Error: resource " << k <<
					" asked to contribute more than 1 to activity " << (i+1) << endl;
				error = true;
			}

			activity_resource[i][k] = true;
			activity_skill[i][l]--;
		}
	}

	for(int i = 0; i < instance->getNActivities()+2; i++){
		for(int l = 0; l < instance->getNSkills(); l++){
			if(activity_skill[i][l]>0){
				errstream << "Error: activity " << i
					<< " has not enough resources covering skill " << l
					<< ". Required " << instance->getDemand(i,l)
					<< ", assigned " << (instance->getDemand(i,l) - activity_skill[i][l]) << endl;
				error = true;
			}
			else if(activity_skill[i][l]<0){
				errstream << "Error: activity " << i
					<< " has too much resources covering skill " << l
					<< ". Required " << instance->getDemand(i,l)
					<< ", assigned " << (instance->getDemand(i,l) - activity_skill[i][l]) << endl;
				error = true;
			}
		}
	 }


	for(int i = 0; i < instance->getNActivities()+2; i++)
	for(int j = i+1; j < instance->getNActivities()+2; j++)
	if(i!=j)
	for(int k = 0; k < instance->getNResources(); k++)
	if(activity_resource[i][k] && activity_resource[j][k])
	if((starts[i] <= starts[j] && starts[j] < starts[i] + instance->getDuration(i))
		||(starts[j] <= starts[i] && starts[i] < starts[j] + instance->getDuration(j))){
		errstream << "Error: activities " << i << " and " << j
			<< " require resource " << k << " at a same time" << endl;
		error = true;
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

	for(int i = 0; i < instance->getNActivities(); i++){
		delete [] activity_resource[i];
		delete [] activity_skill[i];
	}

	delete [] activity_resource;
	delete [] activity_skill;
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
	"Check if a solution of the Multi-Skill Project Scheduling Problem (MSPSP) is correct."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MSPSP * instance = parser::parseMSPSP(pargs->getArgument(0));
	vector<int> starts;
	vector<vector<pair<int,int> > > assignment;

	ifstream f;
	if(pargs->getNArguments()==2){
		f.open(pargs->getArgument(1).c_str());
		if (!f.is_open()) {
			cerr << "Could not open file " << pargs->getArgument(1) << endl;
			exit(BADFILE_ERROR);
		}
	}

	istream & str = pargs->getNArguments()==2 ? f : cin;

	parser::parseMSPSPSolution(str,instance,starts,assignment);

	if(pargs->getNArguments()==2)
		f.close();

	checkSolution(instance,starts,assignment);

	cout << pargs->getArgument(0)<< ":\t" << (error ? "ERROR" : "OK") << endl;
	if(error && pargs->getBoolOption(VERBOSE)){
		cout << "============================================================";
		cout << errstream.str();
		cout << "============================================================" << endl;
    }

	return 0;
}

