#include "parser.h"
#include "errors.h"
#include "util.h"


using namespace std;

namespace parser
{


MRCPSP * parseMRCPSP(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if (extension=="mm" || extension=="MM" || extension=="sm" || extension=="SM")
	  return parser::parseMRCPSPfromMM(filename);
  else if(extension=="mm2" || extension=="MM2")
	  return parser::parseMRCPSPfromMM2(filename);
  else if(extension=="rcp" || extension=="RCP"){
     return parser::parseMRCPSPfromRCP(filename);
  }
  else if(extension=="prb" || extension=="PRB"){
		return parser::parseMRCPSPfromPRB(filename);
  }
  else if(extension=="data" || extension=="DATA"){
		return parser::parseMRCPSPfromDATA(filename);
  }
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}


/* This method parses an RCPSP instance from an rcp format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromRCP(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nactivities >> nresources;
	nactivities-=2;

	instance = new MRCPSP(nactivities,nresources,0);

	for (int j=0;j<nresources;j++) {
		f >> aux;
		instance->setCapacity(j,aux);
	}

	for(int i=0;i<=nactivities+1;i++){
		instance->setNModes(i,1);
		f >> aux;
		instance->setDuration(i,0,aux);

		for (int j=0;j<nresources;j++) {
			f >> aux;
			instance->setDemand(i,j,0,aux);
		}
		int nsuccessors;
		f >> nsuccessors;
		for (int k=0;k<nsuccessors;k++) {
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
	}

	f.close();

	return instance;
}

/* This method parses an RCPSP instance from an rcp format file,
 * and generalizes it to PRCPSP in the obvious way (activities with 1 step of duration)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPasPRCPSPfromRCP(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nmrcpspactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nmrcpspactivities >> nresources;
	nmrcpspactivities-=2;

	int defnactivities = 0;

	vector<int> * succs = new vector<int> [nmrcpspactivities+2];
	vector<int> first_vec;
	vector<int> last_vec;
	vector<int> * demands = new vector<int> [nresources];
	vector<int> capacities;

	for (int j=0;j<nresources;j++) {
		f >> aux;
		capacities.push_back(aux);
	}

	for(int i=0;i<=nmrcpspactivities+1;i++){
		f >> aux;
		
		//std::cout << i << std::endl;
		first_vec.push_back(defnactivities);
		last_vec.push_back(defnactivities + aux);

		if(aux==0)
			defnactivities++;
		else 
			defnactivities += aux;

		// Resource demand
		int demand;
		for (int j=0;j<nresources;j++) {
			f >> demand;
			demands[j].insert(demands[j].begin()+i,demand);
		}

		// Push back successors
		int nsuccessors;
		f >> nsuccessors;
		for (int k=0;k<nsuccessors;k++) {
			f >> aux;
			succs[i].push_back(aux-1);
		}
	}

	defnactivities -= 2;
	instance = new MRCPSP(defnactivities,nresources,0);

	for (int r=0;r<nresources;r++)
		instance->setCapacity(r,capacities[r]);

	for(int s: succs[0]) {
		instance->addSuccessor(0,first_vec[s]);
	}

	for(int i=0;i<=nmrcpspactivities+1;i++){
		for(int j=first_vec[i]; j<last_vec[i]; j++){

			instance->setNModes(j,1);
			instance->setDuration(j,0,1);
			if(j>first_vec[i])
				instance->addSuccessor(j-1,j);

			// Resource demands
			for (int r=0;r<nresources;r++) {
				instance->setDemand(j,r,0,demands[r][i]);
			}
		}
		
		//Successors
		for(int s: succs[i]){
			if(first_vec[i]!=last_vec[i] && s<defnactivities+1)
				instance->addSuccessor(last_vec[i]-1,first_vec[s]);
			else if (first_vec[i]==last_vec[i])
				instance->addSuccessor(last_vec[i],first_vec[s]);
		}
	}

	/* std::cout << instance->getNActivities() << std::endl;
	for (int i=0;i<defnactivities+2;i++) {
		std::cout << "ACTIVITY " << i << ": " << std::endl << std::endl;
		std::cout << "Duration:" << instance->getDuration(i,0) << std::endl;
		for (int r=0;r<nresources;r++) {
			std::cout << "Demand for " << r << ": " << instance->getDemand(i,r,0) << std::endl;
		}
		std::cout << std::endl;
		std::cout << "Successors " << i << ": ";
		for(int s: instance->getSuccessors(i) ){
			std::cout << s << " ";
		}
		std::cout << std::endl << std::endl;
	} */

	f.close();

	delete[] succs;
	delete[] demands;

	return instance;
}


/* This method parses an RCPSP instance from a data format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromDATA(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nresources;
	vector<int> capacities(nresources);
	for(int i = 0; i < nresources; i++)
		f >> capacities[i];

	f >> nactivities;

	instance = new MRCPSP(nactivities,nresources,0);
	instance->setNModes(0,1);
	instance->setNModes(nactivities+1,1);
	instance->setDuration(0,0,0);
	instance->setDuration(nactivities+1,0,0);

	for (int i=0;i<nresources;i++){
		instance->setCapacity(i,capacities[i]);
		instance->setDemand(0,i,0,0);
		instance->setDemand(nactivities+1,i,0,0);
	}


	for(int i=1;i<=nactivities;i++){
		instance->setNModes(i,1);
		f >> aux;
		instance->setDuration(i,0,aux);
		instance->addSuccessor(0,i);
		instance->addSuccessor(i,nactivities+1);
	}
	for (int j=0;j<nresources;j++) {
		for(int i=1;i<=nactivities;i++){
			f >> aux;
			instance->setDemand(i,j,0,aux);
		}
	}

	instance->addSuccessor(0,nactivities+1);

	string line_s;
	getline(f,line_s);

	for(int i=1;i<=nactivities;i++){
		getline(f,line_s);
		stringstream str(line_s);
		int suc;
		while(str>>suc)
			instance->addSuccessor(i,suc);
	}



	f.close();

	return instance;
}


/* This method parses an RCPSP instance from a prb format file,
 * and generalizes it to MRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
MRCPSP * parseMRCPSPfromPRB(const string &  filename)
{

	MRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nactivities >> nresources;

	instance = new MRCPSP(nactivities,nresources,0);

	for (int i=1;i<=nactivities;i++) {
		int npreds, nmodes;
		f >> npreds;
		if(npreds == 0)
			instance->addSuccessor(0,i);
		else{
			for(int j = 0; j < npreds; j++){
				f >> aux;
				instance->addSuccessor(aux,i);
			}
		}
		f >> nmodes;
		instance->setNModes(i,nmodes);
		for(int m = 0; m < nmodes; m++){
			f >> aux;
			instance->setDuration(i,m,aux);
			for(int r = 0; r < nresources; r++){
				f >> aux;
				instance->setDemand(i,r,m,aux);
			}
		}
	}
	for (int j=0;j<nresources;j++) {
		f >> aux;
		instance->setCapacity(j,aux);
	}


	f.close();


	instance->setNModes(0,1);
	instance->setNModes(nactivities+1,1);
	instance->setDuration(0,0,0);
	instance->setDuration(nactivities+1,0,0);
	for (int j=0;j<nresources;j++){
		instance->setDemand(0,j,0,0);
		instance->setDemand(nactivities+1,j,0,0);
	}
	for(int i = 0; i <= nactivities; i++)
		if(instance->getSuccessors(i).empty())
			instance->addSuccessor(i,nactivities+1);


	return instance;
}

MRCPSP * parseMRCPSPfromMM2(const string &  filename)
{
	ifstream f;
	int nresources = 0;
	int nactivities=0;
	int nresourcesnorew=0;

	MRCPSP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string line_s;
	vector<int> nmodes;
	char skip='a';
	int aux;

	do f >> skip; while(skip!=':');

	f >> nactivities;
	nactivities -=2;
	nmodes.resize(nactivities+2);
	do f >> skip; while(skip!=':');
	f >> nresources;
	do f >> skip; while(skip!=':');
	f >> nresourcesnorew;

	instance = new MRCPSP(nactivities,nresources,nresourcesnorew);

	for (int i=1;i<=5;i++)
	getline(f,line_s);

	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		f >> aux; nmodes[i]=aux;
		instance->setNModes(i,aux);
		f >> aux; int nsucessors=aux;

		for(int j =0; j < nsucessors; j++){
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
		getline(f,line_s);
	}

	//Second half of the file

	for (int i=1;i<=4;i++)
		getline(f,line_s);

	int tascaant=0;
	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		for(int j = 0; j < nmodes[i]; j++){
			f >> aux;
			f >> aux;
			instance->setDuration(i,j,aux);
			for(int r = 0; r < nresources + nresourcesnorew; r++){
				f >> aux;
				instance->setDemand(i,r,j,aux);
			}
		}
	}
	for (int i=1;i<=5;i++)
		getline(f,line_s);

	for (int i=0;i<nresources+nresourcesnorew;i++){
		f >> aux;
		instance->setCapacity(i,aux);
	}

	f.close();

	return instance;
}

MRCPSP * parseMRCPSPfromMM(const string &  filename){
	ifstream f;
	int nresources = 0;
	int nactivities=0;
	int nresourcesnorew=0;

	MRCPSP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string line_s;
	vector<int> nmodes;
	char skip='a';
	int aux;

	for (int i=1;i<=5;i++)
		getline(f,line_s);

	do f >> skip; while(skip!=':');

	f >> nactivities;
	nactivities -=2;
	nmodes.resize(nactivities+2);
	do f >> skip; while(skip!=':');
	do f >> skip; while(skip!=':');
	f >> nresources;
	do f >> skip; while(skip!=':');
	f >> nresourcesnorew;

	instance = new MRCPSP(nactivities,nresources,nresourcesnorew);

	for (int i=1;i<=9;i++)
		getline(f,line_s);

	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		f >> aux; nmodes[i]=aux;
		instance->setNModes(i,aux);
		f >> aux; int nsucessors=aux;

		for(int j =0; j < nsucessors; j++){
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
		getline(f,line_s);
	}

	//Second half of the file

	for (int i=1;i<=4;i++)
		getline(f,line_s);

	int tascaant=0;
	for(int i=0;i<nactivities+2;i++){
		f >> aux;
		for(int j = 0; j < nmodes[i]; j++){
			f >> aux;
			f >> aux;
			instance->setDuration(i,j,aux);
			for(int r = 0; r < nresources + nresourcesnorew; r++){
				f >> aux;
				instance->setDemand(i,r,j,aux);
			}
		}
	}
	for (int i=1;i<=4;i++)
		getline(f,line_s);

	for (int i=0;i<nresources+nresourcesnorew;i++){
		f >> aux;
		instance->setCapacity(i,aux);
	}

	f.close();
	return instance;
}

/* This method parses an RCPSP instance from an rcp format file,
 * and generalizes it to PRCPSP in the obvious way (single modes, no nonrenewable resources)
 * filename: path to the instance file
 */
PRCPSP * parsePRCPSPfromRCP(const string &  filename)
{

	PRCPSP * instance;
	ifstream f;

	int nresources=0;
	int nactivities=0;
	int aux;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nactivities >> nresources;
	nactivities-=2;

	instance = new PRCPSP(nactivities,nresources,0);

	for (int j=0;j<nresources;j++) {
		f >> aux;
		instance->setCapacity(j,aux);
	}

	for(int i=0;i<=nactivities+1;i++){
		f >> aux;
		instance->setDuration(i,aux);

		for (int j=0;j<nresources;j++) {
			f >> aux;
			instance->setDemand(i,j,aux);
		}
		int nsuccessors;
		f >> nsuccessors;
		for (int k=0;k<nsuccessors;k++) {
			f >> aux;
			instance->addSuccessor(i,aux-1);
		}
	}

	f.close();

	return instance;
}

void parsePRCPSPSolution(istream & str,PRCPSP * instance,vector<int> &starts, vector<int> &modes, vector<int> &ends, vector<vector<int>> &is_exec){
	starts.resize(instance->getNActivities()+2);

	try {

		findStart(str);

		if(str.eof()) {
			std::cerr << "The file does not have the expected format." << std::endl;
			exit(-1);
		}
		
		for(int i=0;i<instance->getNActivities()+2;i++) {
			starts[i] = readAssingnment(str);
		}

		/*ends.resize(instance->getNActivities()+2);
		for(int i=0;i<instance->getNActivities()+2;i++) {
			ends[i] = readAssingnment(str);
		}*/

		is_exec.resize(instance->getNActivities()+2);
		for(int i=0;i<instance->getNActivities()+2;i++) {
			is_exec[i].resize(starts.back());
			readExecution(str,is_exec[i]);
		}

		ends.resize(instance->getNActivities()+2);
		for(int i=0;i<instance->getNActivities()+1;i++) {
			ends[i] = 0;
			for(int t=0; t<is_exec[i].size(); t++)
				if(is_exec[i][t])
					ends[i] = t;
		}
		ends[instance->getNActivities()+1] = starts[instance->getNActivities()+1];
	}
	catch (std::exception const& e){
     	cout << "There was an error reading the file: " << e.what() << endl;
	}
}

void parseMRCPSPSolution(istream & str,MRCPSP * instance,vector<int> &starts, vector<int> &modes){
	starts.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		starts[i] = readAssingnment(str);

	modes.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		modes[i] = readAssingnment(str)-1;
}


RCPSPMAX * parseRCPSPMAX(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if(extension=="sch" || extension=="SCH")
	  return parser::parseRCPSPMAXfromSCH(filename);
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}

RCPSPMAX * parseRCPSPMAXfromSCH(const string & filename) {
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	int nactivities,nresources;
	int aux;
	f >> nactivities >> nresources >> aux >> aux;

	RCPSPMAX * instance = new RCPSPMAX(nactivities,nresources);

	for(int i = 0; i < nactivities+2; i++){
		char c; //Auxilliary to remove the parenthesis
		int nsucs,lag,suc;
		f >> aux >> aux >> nsucs; //Number of modes always 1, stored into aux
		for(int j = 0; j < nsucs; j++){
			f >> suc;
			instance->addSuccessor(i,suc);
		}
		for(int j : instance->getSuccessors(i)){
			f>>c; //Skip '['
			f >> lag;
			instance->setTimeLag(i,j,lag);
			f>>c; //Skip ']'
		}
	}

	for(int i = 0; i < nactivities+2; i++){
		int dur, dem;
		f >> aux >> aux >> dur;
		instance->setDuration(i,dur);
		for(int k = 0; k < nresources; k++){
			f >> dem;
			instance->setDemand(i,k,dem);
		}
	}

	for(int k = 0; k < nresources; k++){
		int cap;
		f >> cap;
		instance->setCapacity(k,cap);
	}
	return instance;
}

void parseRCPSPMAXSolution(istream & str, RCPSPMAX * instance,vector<int> &starts){
	starts.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		starts[i] = readAssingnment(str);
}




MRCPSPMAX * parseMRCPSPMAX(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if(extension=="sch" || extension=="SCH")
	  return parser::parseMRCPSPMAXfromSCH(filename);
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}

MRCPSPMAX * parseMRCPSPMAXfromSCH(const string & filename) {
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	char buf[4096];

	int nactivities,nrenewable,nnonrenewable;
	int aux;
	f >> nactivities >> nrenewable >> nnonrenewable >> aux;

	f.getline(buf,4096);

	MRCPSPMAX * m = new MRCPSPMAX(nactivities,nrenewable,nnonrenewable);

	int nresources = nrenewable + nnonrenewable;
	stringstream * streams = new stringstream[nactivities+2];

	int * sucs = new int[nactivities+2];

	for(int i = 0; i < nactivities+2; i++){
		int modes;
		f.getline(buf,4096);
		streams[i]<<string(buf);
		int act, lag;
		streams[i] >> act >> modes >> sucs[i];
		m->setNModes(i,modes);
	}
	for(int i = 0; i < nactivities+2; i++){
		char c; //Auxilliary to remove the parenthesis
		int lag,suc;
		for(int j = 0; j < sucs[i]; j++){
			streams[i] >> suc;
			m->addSuccessor(i,suc);
		}
		for(int j : m->getSuccessors(i)){
			streams[i]>>c; //Skip '['
			for(int o1 = 0; o1 < m->getNModes(i); o1++){
				for(int o2 = 0; o2 < m->getNModes(j); o2++){
					streams[i] >> lag;
					m->setTimeLag(i,j,o1,o2,lag);
				}
			}
			streams[i]>>c; //Skip ']'
		}
	}

	for(int i = 0; i < nactivities+2; i++){
		f >> aux;
		for(int o = 0; o < m->getNModes(i); o++){
			int dur, dem;
			f >> aux >> dur;
			m->setDuration(i,o,dur);
			for(int k = 0; k < nresources; k++){
				f >> dem;
				m->setDemand(i,k,o,dem);
			}
		}
	}

	for(int k = 0; k < nresources; k++){
		int cap;
		f >> cap;
		m->setCapacity(k,cap);
	}

	delete[] sucs;
	delete[] streams;

	return m;
}

void parseMRCPSPMAXSolution(istream & str, MRCPSPMAX * instance,vector<int> &starts, vector<int> &modes){
	starts.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		starts[i] = readAssingnment(str);

	modes.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		modes[i] = readAssingnment(str)-1;
}



RCPSPT * parseRCPSPT(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if(extension=="smt" || extension=="SMR")
	  return parser::parseRCPSPTfromSMT(filename);
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}


RCPSPT * parseRCPSPTfromSMT(const string & filename){
  ifstream f;

  f.open(filename.c_str());
  if (!f.is_open()) {
	cerr << "Could not open file " << filename << endl;
	exit(BADFILE_ERROR);
}

  int nresources = 0;
  int nactivities=0;
  int horizon = 0;
  int aux=0;

  string auxs;
  char skip='a';

  do f >> skip; while(skip!=':');
  do f >> skip; while(skip!=':');
  do f >> skip; while(skip!=':');
  do f >> skip; while(skip!=':');
  f >> nactivities;
  nactivities -=2;

  do f >> skip; while(skip!=':');
  f >> horizon;



  do f >> skip; while(skip!=':');
  f >> nresources;
  do f >> skip; while(skip!=':');
  f >> aux;

  RCPSPT * instance = new RCPSPT(nactivities,nresources,horizon);

  for (int i=0;i<9;i++) getline(f,auxs);

  for(int i=0;i<nactivities+2;i++){
	int nsuccessors;
	f >> aux >> aux >> nsuccessors;

	for(int j =0; j < nsuccessors; j++){
	  f >> aux;
	  instance->addSuccessor(i,aux-1);
	}
	getline(f,auxs);
  }


 for (int i=0;i<4;i++) getline(f,auxs);

 for(int i=0;i<nactivities+2;i++){
    f >> aux >> aux >> aux;
	instance->setDuration(i,aux);
	if(i>0 && i < nactivities+1){
		for(int r = 0; r < nresources; r++){
			for(int j = 0; j < instance->getDuration(i); j++){
				f >> aux;
				instance->setDemand(i,r,j,aux);
			}
		}
	}
 }
 for (int i=0;i<5;i++) getline(f,auxs);

 for (int i=0;i<nresources;i++){
   for(int j = 0; j < horizon; j++){
		f >> aux;
		instance->setCapacity(i,j,aux);
	}
 }

 return instance;
}

void parseRCPSPTSolution(istream & str, RCPSPT * instance,vector<int> &starts){
	starts.resize(instance->getNActivities()+2);
	for(int i=0;i<instance->getNActivities()+2;i++)
		starts[i] = readAssingnment(str);
}


MSPSP * parseMSPSP(const string & filename) {
  string extension=filename.substr(filename.rfind(".")+1);
  if(extension=="txt" || extension=="TXT")
	  return parser::parseMSPSPfromTXT(filename);
  if(extension=="dat" || extension=="DAT")
	  return parser::parseMSPSPfromDAT(filename);
  else {
	  cerr << "bad input file extension" << endl;
	  exit(BADFILEEXTENSION_ERROR);
	}
}

MSPSP * parseMSPSPfromTXT(const string & filename) {
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	int nactivities,nresources,nskills, npreds;

	f >> nactivities;
	int * duration = new int[nactivities];
	for(int i = 0; i < nactivities; i++)
		f>> duration[i];

	f >> nskills;
	int ** demands = new int*[nactivities];
	for(int i = 0; i < nactivities; i++){
		demands[i]=new int[nskills];
		for(int l = 0; l < nskills; l++)
			f >> demands[i][l];
	}

	f >> nresources;

	MSPSP * m = new MSPSP(nactivities-2,nresources,nskills);

	for(int k = 0; k < nresources; k++){
		for(int l = 0; l < nskills; l++){
			int hasSkill;
			f >> hasSkill;
			if(hasSkill==1)
				m->setResourceSkill(k,l);
		}
	}


	for(int i = 0; i < nactivities; i++){
		m->setDuration(i,duration[i]);
		for(int l = 0; l < nskills; l++){
			m->setDemand(i,l,demands[i][l]);
		}
	}


	f >> npreds;

	int * preds = new int[npreds];
	int * sucs = new int[npreds];

	for(int p = 0; p < npreds; p++)
		f >> preds[p];

	for(int p = 0; p < npreds; p++)
		f >> sucs[p];

	for(int p = 0; p < npreds; p++)
		m->addSuccessor(preds[p]-1,sucs[p]-1);

	delete [] duration;
	for(int i = 0; i < nactivities; i++)
		delete [] demands[i];
	delete [] demands;

	delete [] preds;
	delete [] sucs;


	return m;
}

MSPSP * parseMSPSPfromDAT(const string & filename) {
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	char buf[256];

	int nactivities,nresources,nskills;
	f >> nactivities >> nskills >> nresources;
	f.getline(buf,256);

	MSPSP * m = new MSPSP(nactivities-2,nresources,nskills);

	for(int i = 0; i < nresources; i++){
		f.getline(buf,256);
		stringstream str(buf);
		int skill,res;
		str >> res;
		while(str >> skill)
			m->setResourceSkill(res,skill);
	}

	for(int i = 0; i < nactivities; i++){
		f.getline(buf,256);
		stringstream str(buf);
		int act, dur, skill, dem;
		str >> act;
		str >> dur;
		m->setDuration(act,dur);
		while(str>>skill){
			str >> dem;
			m->setDemand(act,skill,dem);
		}
	}

	for(int i = 0; i < nactivities; i++){
		f.getline(buf,256);
		stringstream str(buf);
		int act, act2;
		str >> act;
		while(str>>act2)
			m->addSuccessor(act,act2);
	}
	return m;
}

void parseMSPSPSolution(istream & str, MSPSP * instance, vector<int> &starts, vector<vector<pair<int,int> > > & assignments){
	int N = instance->getNActivities();
	int R = instance->getNResources();
	int L = instance->getNSkills();

	starts.resize(N+2);
	for (int i=0;i<=N+1;i++){
		starts[i] = readAssingnment(str);
	}

	assignments.resize(N+2);
	for (int i=0;i<=N+1;i++){
		for(int l = 0; l < L; l++){
			for(int k = 0; k < instance->getDemand(i,l); k++){
				int act, res, skill;
				act = readAssingnment(str);
				if(str.eof())
					return;
				res = readAssingnment(str);
				if(str.eof())
					return;
				skill = readAssingnment(str);
				if(str.eof())
					return;
				assignments[act].push_back(pair<int,int>(res,skill));

			}
		}
	}
}



MMKP * parseMMKP(const string &  filename) {
	string extension=filename.substr(filename.rfind(".")+1);
	if (extension=="mmkp")
		return parser::parseMMKPfromMMKP(filename);
	else if(extension=="data")
		return parser::parseMMKPfromDATA(filename);
	else if(extension=="txt")
		return parser::parseMMKPfromTXT(filename);
	else {
		cerr << "bad input file extension" << endl;
		exit(BADFILEEXTENSION_ERROR);
	}
}


MMKP * parseMMKPfromMMKP(const string &  filename){
	ifstream f;
	int n;
	int m;
	int l;

	MMKP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> n >> l >> m;

	instance = new MMKP(n,m);
	instance->R.resize(m);
	instance->v.resize(n);
	instance->r.resize(n);
	instance->l.resize(n);

	for(int i = 0; i < m; i++)
		f >> instance->R[i];

	int set;
	for(int i = 0; i < n; i++){
		f >> set;

		instance->l[i]=l;
		instance->v[i].resize(l);
		instance->r[i].resize(l);
		for(int j = 0; j < l; j++){
			float p;
			f >> p;
			instance->v[i][j] = (int) p;
			instance->r[i][j].resize(m);
			for(int k = 0; k < m; k++)
				f >> instance->r[i][j][k];
		}
	}

	f.close();
	return instance;
}



MMKP * parseMMKPfromDATA(const string &  filename){
	ifstream f;
	int n;
	int m;
	int l;

	MMKP * instance;

	f.open(filename.c_str());


	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}


	char skip='a';
	int aux;
	string line_s;

	for (int i=0;i<4;i++)
		getline(f,line_s);

	f >> line_s;
	f >> n;

	f >> line_s;
	f >> m;
	instance = new MMKP(n,m);

	instance->R.resize(m);
	instance->v.resize(n);
	instance->r.resize(n);
	instance->l.resize(n);

	for(int i = 0; i < n; i++){
		getline(f,line_s);
		getline(f,line_s);

		f >> line_s;
		f >> instance->l[i];
		instance->v[i].resize(instance->l[i]);
		instance->r[i].resize(instance->l[i]);
		for(int j = 0; j < instance->l[i]; j++){
			f >> instance->v[i][j];
			instance->r[i][j].resize(m);
			for(int k = 0; k < m; k++)
				f >> instance->r[i][j][k];
		}
	}

	f >> line_s;
	for(int i = 0; i < m; i++)
		f >> instance->R[i];

	f.close();
	return instance;
}


MMKP * parseMMKPfromTXT(const string &  filename){
	ifstream f;
	string line;
	string num;
	stringstream ss;

	int nclasses;
	int ndimensions;
	MMKP * instance;

	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	f >> nclasses >> ndimensions;

	instance = new MMKP(nclasses,ndimensions);
	instance->R.resize(ndimensions);
	instance->v.resize(nclasses);
	instance->r.resize(nclasses);
	instance->l.resize(nclasses);

	getline(f,line);
	getline(f,line);
	ss << line;

	for(int i = 0; i < ndimensions; i++){
		getline(ss,num,'-');
		instance->R[i] = stoi(num);
	}

	for(int i = 0; i < nclasses; i++){
		f >> instance->l[i];
		getline(f,line);
		instance->v[i].resize(instance->l[i]);
		instance->r[i].resize(instance->l[i]);
		for(int j = 0; j < instance->l[i]; j++){
			instance->r[i][j].resize(ndimensions);
			stringstream ss2;
			getline(f,line);
			ss2 << line;
			getline(ss2,num,'-');
			instance->v[i][j] = stoi(num);
			for(int k = 0; k < ndimensions; k++){
				getline(ss2,num,'-');
				instance->r[i][j][k] = stoi(num);
			}
		}
	}

	f.close();
	return instance;
}

void parseMMKPSolution(istream & str,MMKP * instance,vector<int> &solution){
	solution.resize(instance->n);
	for(int i=0;i<instance->n;i++)
		solution[i] = readAssingnment(str)-1;
}


Clinics * parseClinics(const string & filename, const string & dir){
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string rs;
	int ri;
	int nsets, nmutex, nresources;
	vector<int> capacities;
	map<string,ClinicsTest *> tests;
	map<string,int> ntests_type;
	//vector<pair<string,vector<int> > > sets;
	vector<pair<string,int> > tests_v;

	int ntests = 0;
	int nactivities = 0;

	f >> rs;
	do f >> rs; while(rs!="##");

	f >> nsets >> nmutex >> nresources;

	for(int i = 0; i < nresources; i++){
		f >> ri >> ri;
		capacities.push_back(ri);
	}

	map<int,bool> singleTest; //singleTest[id] true iff sample 'id' has only one test

	//sets.resize(nsets);
	for(int i = 0; i < nsets; i++){
		f >> rs;
		//sets[i].first = rs;
		ClinicsTest * t;
		if(tests.find(rs)==tests.end()){
			t = parseClinicsTest(dir+"/"+rs+".test",nresources);
			tests[rs]=t;
			ntests_type[rs]=0;
		}
		else
			t = tests[rs];
		int ntestsSet;
		f >> ntestsSet;
		ntests_type[rs]+=ntestsSet;
		ntests+=ntestsSet;
		for(int j = 0; j < ntestsSet; j++){
			f >> ri;
			//sets[i].second.push_back(ri);
			tests_v.push_back(pair<string,int>(rs,ri));
			nactivities+=t->getNActivities();

			if(singleTest.find(ri)==singleTest.end())
				singleTest[ri]=true;
			else
				singleTest[ri]=false;

		}
	}

	//Create the instance
	Clinics * ins = new Clinics(ntests,nmutex, nactivities, nresources);

	//Set capacities
	for(int i = 0; i < nresources; i++)
		ins->setCapacity(i,capacities[i]);


	//Fill it with activities of each test
	int testFirstId = 1;

	int * firstIds = new int[ntests];
	int lastSingleTest = -1;
	string testType = "";

	for(int i = 0; i < tests_v.size(); i++){
		ClinicsTest * t = tests[tests_v[i].first];
		ins->setTestSample(i,tests_v[i].second);
		ins->setTestType(i,tests_v[i].first);
		firstIds[i] = testFirstId;

		for(int j = 0; j < t->getNActivities(); j++){
			int activityId = testFirstId + j;
			ins->setTest(activityId,i);
			ins->setId(activityId,j);
			ins->setType(activityId,t->getType(j));
			ins->setDuration(activityId,t->getDuration(j));
			ins->setCleanDuration(activityId,t->getCleanDuration(j));
			ins->setCleanFactor(activityId,t->getCleanFactor(j));
			for(int k : t->getSuccessors(j))
				ins->addSuccessor(activityId,testFirstId+k,t->getTimeLag(j,k));
		}
		for(int k : t->getOpeningActivities())
			ins->addSuccessor(0,testFirstId+k,0);
		for(int k : t->getClosingActivities())
			ins->addSuccessor(testFirstId+k,nactivities+1,t->getCleanDuration(k));

		//Add demands
		for(int r = 0; r < nresources; r++)
			for(const pair<int,int> & p2 : t->getDemands(r))
				ins->addDemand(testFirstId+p2.first,testFirstId+p2.second,r);

		//If single test in sample
		if(singleTest[tests_v[i].second]){
			if(testType==tests_v[i].first){ //If another single test in this set
				for(int j = 0; j < t->getNActivities(); j++){
					int dur = t->getCleanFactor(j)=="Sample" ? t->getCleanDuration(j) : t->getDuration(j);
					ins->addSuccessor(lastSingleTest+j,testFirstId+j,dur);
				}
			}
			lastSingleTest=testFirstId;
			testType=tests_v[i].first;
		}

		testFirstId+=t->getNActivities();

	}

	//Config mutexes and add demands
	for(int i = 0; i < tests_v.size()-1; i++){
		ClinicsTest * ti = tests[tests_v[i].first];
		int sampleIdi = tests_v[i].second;
		for(int j = 0; j < tests_v.size(); j++){
			ClinicsTest * tj = tests[tests_v[j].first];
			int sampleIdj = tests_v[j].second;
			for(int ii = 0; ii < ti->getNActivities(); ii++){
				for(int jj = 0; jj < tj->getNActivities(); jj++){
					int idi = firstIds[i]+ii;
					int idj = firstIds[j]+jj;
					if(idi==idj)
						ins->setNeedClean(idi,idj,false);
					else{
						if(ti->getType(ii) == tj->getType(jj)){
							if(ti->getCleanFactor(ii)==tj->getCleanFactor(jj)){
							  if(ti->getCleanFactor(ii)=="Sample")
									 ins->setNeedClean(idi,idj,sampleIdi != sampleIdj);
							}
							else
								ins->setNeedClean(idi,idj,true);
						}
					}
				}
			}
		}
	}

	ins->computeExtPrecs();

	delete [] firstIds;
	for(const pair<string,ClinicsTest *> p : tests)
		delete p.second;

	return ins;
}

ClinicsTest * parseClinicsTest(const string & filename, int nresources){

	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	string rs;
	int ri;
	int nactivities;

	f >> rs;
	do f >> rs; while(rs!="##");

	f >> nactivities;

	ClinicsTest * test = new ClinicsTest(nactivities,nresources);

	//Read precedence relations
	for(int i = 0; i < nactivities; i++){
		int nsucs, suc, lag;
		f >> ri >> nsucs;
		for(int j = 0; j < nsucs; j++){
			f >> suc >> lag;
			test->addSuccessor(i,suc-1,lag);
		}
	}


	//Durations
    int type, dur, cleandur;
    string factor;
    for(int i = 0; i < nactivities; i++){
		f >> ri >> type >> dur;
		test->setType(i,type-1);
		test->setDuration(i,dur);
		if(type!=3) //If it is not shuttle
		{
			f >> cleandur >> factor;
			test->setCleanDuration(i,cleandur);
			test->setCleanFactor(i,factor);
		}
		else{
			test->setCleanDuration(i,dur);
			test->setCleanFactor(i,"");
		}
    }

    //Resource consumptions
    f >> ri;
    int resource, act1, act2;
    for(int i = 0; i < ri; i++){
		f >> resource >> act1 >> act2;
		test->addDemand(act1-1,act2-1,resource-4);
    }

	return test;
}


Auction * parseAuction(const string & filename){
	ifstream f;
	f.open(filename.c_str());
	if (!f.is_open()) {
		cerr << "Could not open file " << filename << endl;
		exit(BADFILE_ERROR);
	}

	int nitems, nbids, v;
	f >> nitems >> nbids;
	Auction * a = new Auction(nitems, nbids);
	for(int i = 0; i < nbids; i++){
		f >> v;
		while(v != 0){
			a->addItem(i,v-1);
			f >> v;
		}
	}
	for(int i = 0; i < nbids; i++){
		f >> v;
		a->setBidValue(i,v);
	}

	return a;
}

void findChar(istream & str, char c){
	char aux;
	do{
		str >> aux;
	}while(aux!=c && !str.eof());
}

void findStart(istream & str) {
	char cur, prev;
	cur=' ';
	do{
		prev = cur;
		str >> cur;
	}while((cur!='_' || prev!='S') && !str.eof());
}

int readAssingnment(istream & str){
	findChar(str,':');
	int i;
	str >> i;
	return i;
}

vector<int> readExecution(istream & str, vector<int> &is_exec){
	findChar(str,':');
	string st;
	str >> st;
	for(int i=0; i<is_exec.size(); i++){
		is_exec[i] = st[i] - '0';
	}
	return is_exec;
}
}
