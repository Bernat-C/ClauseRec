#include <string>
#include <vector>
#include <iostream>
#include "parser.h"
#include "errors.h"
#include "util.h"
#include "basiccontroller.h"
#include "clinicsencoding.h"
#include "clinicstimeencoding.h"
#include "clinicstaskencoding.h"
#include "clinicseventhandler.h"
#include "solvingarguments.h"

using namespace std;
using namespace util;

/*
 * Enumeration of all the accepted program arguments
 */
enum ProgramArg {
	ENCODING,
	CHART_PATH,
	ACTIVITY
};


int main(int argc, char **argv) {

	Arguments<ProgramArg> * pargs
	= new Arguments<ProgramArg>(

	//Program arguments
	{
	arguments::arg("instance_file","Instance file path."),
	arguments::arg("test_directory","Directory where to find the test definition files.")
	},
	2,

	//Program options
	{
	arguments::sop("E","encoding",ENCODING,"time",
	{"time","task"},
	"Encoding used to solve the problem. Default: time."),

	arguments::sop("C","chart",CHART_PATH,"",
	"File where to print the charts (PDF). If not specified, no charts will be printed. If not specified, no chart is produced."),


	arguments::bop("A","active",ACTIVITY,true,
	"If set to 1, an inprocessing step is done to find active solutions. Default: 1."),
	},
	"Solve the clinic analitics problem."
	);

	SolvingArguments * sargs = SolvingArguments::readArguments(argc,argv,pargs);

	Clinics * instance = parser::parseClinics(pargs->getArgument(0),pargs->getArgument(1));


	ClinicsEncoding * enc;
	if(pargs->getStringOption(ENCODING) == "time")
		enc = new ClinicsTimeEncoding(instance);
	else if(pargs->getStringOption(ENCODING) == "task")
		enc = new ClinicsTaskEncoding(instance);

	string chartpath = pargs->getStringOption(CHART_PATH);

	if(chartpath!="")
		enc->setPrintCharts(chartpath);

	int UB = instance->trivialUB();

	if(sargs->getIntOption(UPPER_BOUND)==INT_MIN){
		vector<int> starts;
		instance->computeGreedySchedule(starts);

		//Mostrar
		cout << "v ";
		instance->printSolution(cout,starts);
		cout << endl;

		if(chartpath!="")
			instance->makeChart(chartpath,starts);

		UB = starts[instance->getNActivities()+1]-1;
	}

	//EventHandler * handler =  new ClinicsEventHandler(instance,enc,sargs,pargs->getBoolOption(ACTIVITY));
	//BasicController c(sargs,enc,true,instance->trivialLB(),UB, handler);
	BasicController c(sargs,enc,true,instance->trivialLB(),UB);

	c.run();

	return 0;
}

