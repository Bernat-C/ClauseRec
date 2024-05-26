#ifndef CLINICSTEST_H
#define CLINICSTEST_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>

using namespace std;


class ClinicsTest
{

private:

	//INSTANCE DATA

	int nactivities; //Number of non-dummy activites
	int nresources; //Number of resources
	
	vector<int> * succs;
	int * duration; //Duration of the activity without clean
	int * clean_duration; //Duration of the activity with clean
	int * activity_type; //Type of the activity
	string * clean_factor; //Label which, if different in another activity of the same type, enables the cleaning
	int ** time_lags; //Extended time lags; //Extended time lags

	vector<pair<int,int> > * demands; //(first) and (second) are the delimiter of an occupation of resource (third)

public:

	ClinicsTest(int nactivities, int nresources);
   ~ClinicsTest();

	int getNActivities() const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setCleanDuration(int i, int p);
	int getCleanDuration(int i) const;

	void setCleanFactor(int i, const string & s);
	string getCleanFactor(int i) const;

	int getType(int i) const;
	void setType(int i, int type);

	void addDemand(int i, int j, int r);
	const vector<pair<int,int> > & getDemands(int r) const;

	void addSuccessor(int i, int j, int lag);
	const vector<int> & getSuccessors(int i) const;

	vector<int> getOpeningActivities() const;
	vector<int> getClosingActivities() const;

	int getTimeLag(int i, int j) const;

	friend ostream &operator <<(ostream &, ClinicsTest &);

};

#endif

