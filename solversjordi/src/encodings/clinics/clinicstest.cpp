#include "clinicstest.h"
#include "limits.h"
#include <list>
#include <algorithm>
#include "util.h"
#include <math.h>

using namespace std;

ClinicsTest::ClinicsTest(int nactivities, int nresources){

	//Prepare instance data

	this->nactivities = nactivities;
	this->nresources = nresources;

	this->activity_type = new int[nactivities];
	this->clean_factor = new string[nactivities];

	this->succs = new vector<int>[nactivities];
	this->time_lags = new int*[nactivities];
    for(int i = 0; i < nactivities; i++)
		this->time_lags[i] = new int[nactivities];
		
	this->duration = new int[nactivities];
	this->clean_duration = new int[nactivities];
	
	this->demands = new vector<pair<int,int> >[nresources];

	for(int i = 0; i < nactivities; i++)
		this->time_lags[i] = new int[nactivities];

	

	for(int i = 0; i < nactivities; i++)
		for(int j = 0; j < nactivities; j++)
			this->time_lags[i][j] = INT_MIN;

}

ClinicsTest::~ClinicsTest(){
	delete [] activity_type;
	delete [] clean_factor;
	delete [] succs;
	delete [] duration;
	delete [] clean_duration;
	delete [] demands;

	for(int i = 0; i < nactivities; i++)
		delete [] time_lags[i];

	delete [] time_lags;
}

int ClinicsTest::getNActivities() const{
	return nactivities;
}

int ClinicsTest::getType(int i) const{
	return activity_type[i];
}

void ClinicsTest::setType(int i, int type){
	activity_type[i] = type;
}

void ClinicsTest::setDuration(int i, int p){
	duration[i] = p;
}

int ClinicsTest::getDuration(int i) const{
	return duration[i];
}

void ClinicsTest::setCleanDuration(int i, int p){
	clean_duration[i] = p;
}

int ClinicsTest::getCleanDuration(int i) const{
	return clean_duration[i];
}

void ClinicsTest::setCleanFactor(int i, const string & s){
	clean_factor[i]=s;
}

string ClinicsTest::getCleanFactor(int i) const{
	return clean_factor[i];
}


void ClinicsTest::addDemand(int i, int j, int r){
	demands[r].push_back(pair<int,int>(i,j));
}

const vector<pair<int,int> > & ClinicsTest::getDemands(int r) const{
	return demands[r];
}


void ClinicsTest::addSuccessor(int i, int j, int lag){
	succs[i].push_back(j);
	time_lags[i][j]=lag;
}

const vector<int> & ClinicsTest::getSuccessors(int i) const {
	return succs[i];
}

vector<int> ClinicsTest::getOpeningActivities() const{
	vector<int> result;
	for(int i = 0; i < nactivities; i++){
		bool opening = true;
		for(int j = 0; j < nactivities; j++)
			if(time_lags[j][i]!=INT_MIN)
				opening = false;
		if(opening)
			result.push_back(i);
	}
	return result;
}

vector<int> ClinicsTest::getClosingActivities() const{
	vector<int> result;
	for(int i = 0; i < nactivities; i++){
		bool closing = true;
		for(int j = 0; j < nactivities; j++)
			if(time_lags[i][j]!=INT_MIN)
				closing = false;
		if(closing)
			result.push_back(i);
	}
	return result;
}

int ClinicsTest::getTimeLag(int i, int j) const{
	return time_lags[i][j];
}

ostream &operator << (ostream &output, ClinicsTest &m)
{

	output << "N activities: " << m.nactivities << endl;
	output << "N resources: " << m.nresources << endl;
	for(int i = 0; i < m.nactivities; i++){
		output << "=== Activity " << (i+1) << " ===" << endl;
		output << "Duration: " << m.duration[i] << endl;
		output << "Clean duration: " << m.clean_duration[i] << endl;
		output << "Activity type: " << m.activity_type[i] << endl;
		output << "Clean factor: " << m.clean_factor[i] << endl;
		output << "Successors:" ;
		for(int j : m.succs[i])
			output << " " << (j+1) << " " << m.time_lags[i][j];
		output << endl;
		output << "=================" << endl;
	}
	output << "Starts :";
	for(int i : m.getOpeningActivities())
		output << " " << (i+1);
	output << endl;
	output << "Ends :";
	for(int i : m.getClosingActivities())
		output << " " << (i+1);
	output << endl;
	for(int r = 0; r < m.nresources; r++){
		output << "Demands " << r << ":";
		for(const pair<int,int> & p : m.demands[r])
			output << " (" << (p.first+1) << "," << (p.second+1) << ")";
		output << endl;
	}
	
	return output;
}

