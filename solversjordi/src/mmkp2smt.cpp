#include <iostream>
#include "parser.h"
#include "errors.h"
#include "basiccontroller.h"
#include "mmkpencoding.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;


void createLP(MMKP * instance){

	/*
	cout << "Maximize" << endl;
	for(int i = 0; i < instance->n; i++)
	{
		for(int j = 0; j < instance->l[i]; j++)
			cout << " + " << instance->v[i][j] << "x_"<<i<<"_"<<j;
		cout << endl;
	}
	cout << endl,
	*/

	cout << "Maximize 1" << endl;

	cout << "Subject To" << endl;

	//Exactly one
	for(int i = 0; i < instance->n; i++)
	{
		for(int j = 0; j < instance->l[i]; j++)
			cout << " + " << "x_"<<i<<"_"<<j;
		cout << " = " << 1 << endl << endl;
	}

	cout << endl;


	//PB
	for(int k = 0; k < instance->m; k++){
		for(int i = 0; i < instance->n; i++)
		{
			for(int j = 0; j < instance->l[i]; j++)
				cout << " + " << instance->r[i][j][k] << "x_"<<i<<"_"<<j;
			cout << endl;
		}
		cout << " <= " << instance->R[k] << endl << endl;
	}

	cout << "Binary" << endl;
	for(int i = 0; i < instance->n; i++)
	{
		for(int j = 0; j < instance->l[i]; j++)
			cout << "x_"<<i<<"_"<<j << " ";
		cout << endl;
	}

	cout << "End" << endl;
}

int main(int argc, char **argv) {

	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{
	arguments::arg("filename","Instance file name.")
	},
	1,

	//Program options
	{
	// arguments::sop("O","opt-encoding",OPTI_ENCODING,"amomdd",
	// {"none","maxsat", "smdd", "amomdd", "amopbswc", "amopbgt", "amopbbm", "amopbgpw"},
	// "Encoding of AMO-PBs use. Default: amomdd."),

	arguments::bop("R","reduce-eo",REDUCE_EO,true,
	"Reduce the PB constraints due to EO constraints. Default: 1."),

	arguments::bop("O","objective",OBJECTIVE,true,
	"Encode the objective funcion. Default: 1."),

	arguments::bop("E","print-essence-prime",PRINT_ESSENCE,false,
	"If true, print the instance in essense prime format and exit. Default: 0."),

	arguments::bop("L","create-lp",CREATE_LP,false,
	"Output the linear program to stdout. Default: 0."),
	},
	"Solve the Multichoice Multi-dimensional Knapsack Problem (MMKP)."
	);


	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	MMKP * instance = parser::parseMMKP(pargs->getArgument(0));
	instance->shuffle();
	//cout << *instance << endl;
	//exit(0);

	if(pargs->getBoolOption(PRINT_ESSENCE))
		instance->printESSENCEPrimeInstance();
	else if(pargs->getBoolOption(CREATE_LP))
		createLP(instance);
	else{
		MMKPEncoding * encoding = new MMKPEncoding(instance,pargs,sargs);

		BasicController c(sargs,encoding,false,instance->getLB(),instance->getUB());
		c.run();
	}
	return 0;
}

