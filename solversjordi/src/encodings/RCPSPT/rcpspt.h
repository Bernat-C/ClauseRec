#ifndef RCPSPT_H
#define RCPSPT_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>


using namespace std;

class RCPSPT
{

private:

	//INSTANCE DATA
	int nactivities; //Number of non-dummy activites
	int nresources; //Number of renewable resources
	int timeHorizon; //Time horizon of the schedule

	vector<int> ** demand; //Demands of each activity
	vector<int> * succs; //List of successors of each activity
	int * duration; // Duration of each activity
	int **capacity; //Capacities of the resources at each time


	//PREPROCESSED DATA
	int ** extPrecs; //Extended time lags
	int ** nSteps; //Minimal number of edges joining two activities

	int next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik,  int t) const;

public:

	RCPSPT(int nactivities, int nresources, int timeHorizon);
   ~RCPSPT();

	int getNActivities() const;
	int getNResources() const;
	int getTimeHorizon() const;

	void setCapacity(int r, int t, int c);
	int getCapacity(int r, int t) const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setDemand(int i, int r, int t, int d);
	int getDemand(int i, int r, int t) const;

	void addSuccessor(int i, int j);
	const vector<int> & getSuccessors(int i) const;

	int getExtPrec(int i, int j) const;
	int getNSteps(int i, int j) const;
	int trivialUB() const;
	int trivialLB() const;
	int ES(int i) const;
	int LS(int i, int ub) const;
	int EC(int i) const;
	int LC(int i, int ub) const;
	bool inPath(int i, int j) const;
	bool isPred(int i, int j) const;

	//Preprocesses
	void computeExtPrecs();
	void recomputeExtPrecs();
	void computeEnergyPrecedences();
	void computeSteps();
	int computePSS(vector<int> & starts) const;
	void computeMinPathCover(const vector<int> & vasks, vector<set<int> > & groups) const;

	void printSolution(ostream & os, const vector<int> & starts);
	friend ostream &operator <<(ostream &, RCPSPT &);
	void generateParam();


};

#endif

