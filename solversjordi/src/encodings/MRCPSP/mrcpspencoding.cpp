#include "mrcpspencoding.h"
#include <limits.h>


MRCPSPEncoding::MRCPSPEncoding(MRCPSP * instance) : Encoding() {
	this->ins = instance;
}

MRCPSPEncoding::~MRCPSPEncoding() {
}

int MRCPSPEncoding::getObjective() const{
	return starts.back();
}

void MRCPSPEncoding::getModes(vector<int> &modes){
	modes.resize(this->modes.size());
	for(int i = 0; i < modes.size(); i++)
		modes[i]=this->modes[i];
}


void MRCPSPEncoding::getStartsAndModes(vector<int> &starts, vector<int> &modes){
	modes.resize(this->modes.size());
	for(int i = 0; i < modes.size(); i++)
		modes[i]=this->modes[i];

	starts.resize(this->starts.size());
	for(int i = 0; i < starts.size(); i++)
		starts[i]=this->starts[i];
}

bool MRCPSPEncoding::printSolution(ostream & os) const{
	ins->printSolution(os,starts,modes);
	return true;
}
