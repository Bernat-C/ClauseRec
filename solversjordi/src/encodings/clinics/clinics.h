#ifndef CLINICS_H
#define CLINICS_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>

using namespace std;


class Clinics
{

private:

	//INSTANCE DATA
	int ntests; //Number of tests
	int nresources; //Number of renewable resources
	int ntypes; //Number of types of activities

	int nactivities; //Number of non-dummy activites

	int * test_sample; //Sample to which the test belongs
	string * test_type; //Type of the test


	int * activity_test; //Test to which the activity belongs
	int * activity_id; //Local id of the activity in the test
	int * activity_type; //Type of the activity
	string * clean_factor; //Label which, if different in another activity of the same type, enables the cleaning
	vector<int> * type_activity; //List of activities of each type

	//vector<string> type_name; //Names of the types

	vector<pair<int,int> > * demands; //(first) and (second) are the delimiter of an occupation of the 'i-th' resource
	vector<pair<int,int> > * ons; //For each 'on' activity, its associated off and resource
	vector<pair<int,int> > * offs; //For each 'off' activity, its associated on and resource

	vector<int> * succs; //Successors of each activity
	vector<int> * preds; //Predecessors of each activity
	int ** ext_precs; //Extended time lags

	int * duration; //Duration of the activity without clean
	int * clean_duration; //Duration of the activity with clean

	bool ** need_clean; //True iff it is needed a clean action between the pair of activities

	int * capacity; //Capacities of the resources

	bool needClean(const vector<pair<int,int> > & ordered, int i, int id, int type) const;
	bool needCleanAtTime(int * usage, int makespan, int t, int id) const;
	void buildMachineState(const vector<int> & starts, const vector<pair<int,int> > & ordered, int ** usage, int ** capacities, int makespan) const;
	bool i_enforceActivity(vector<int> & starts, vector<pair<int,int> > & ordered,int ** usage, int ** capacities) const;

public:

	Clinics(int ntests, int ntypes, int nactivities, int nresources);
   ~Clinics();

	int getNActivities() const;
	int getNResources() const;
	int getNTests() const;

	int getActivitySample(int i) const;
	int getTestSample(int test) const;
	void setTestSample(int test, int sample);

	int getId(int i) const;
	void setId(int i, int id);

	string getActivityTestType(int i) const;
	string getTestType(int id) const;
	void setTestType(int id, string s);

	int getTest(int i) const;
	void setTest(int i, int test);

	int getType(int i) const;
	void setType(int i, int type);

	void setCapacity(int r, int c);
	int getCapacity(int r) const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setCleanDuration(int i, int p);
	int getCleanDuration(int i) const;

	void setCleanFactor(int i, const string & s);
	string getCleanFactor(int i) const;

	void setNeedClean(int i , int j, bool b);
	bool needClean(int i, int j) const;

	bool needMutex(int i, int j) const;

	void addDemand(int i, int j, int r);
	const vector<pair<int,int> > & getDemands(int r) const;

	const vector<pair<int,int> > & getOns(int i) const;
	const vector<pair<int,int> > & getOffs(int i) const;

	void addSuccessor(int i, int j, int lag);
	const vector<int> & getSuccessors(int i) const;
	const vector<int> & getPredecessors(int i) const;

	int getExtPrec(int i, int j) const;
	bool areOrdered(int i, int j) const;

	int trivialUB() const;
	int trivialLB() const;
	int ES(int i) const;
	int LS(int i, int ub) const;
	int EC(int i) const;
	int LC(int i, int ub) const;

	void computeExtPrecs();

	bool enforceActivity(vector<int> & starts) const;

	void computeGreedySchedule(vector<int> & starts) const;

	void printSolution(ostream & os, const vector<int> & starts) const;
	void makeChart(const string & filename, const vector<int> & starts) const;

	friend ostream &operator <<(ostream &, Clinics &);

};

#endif

