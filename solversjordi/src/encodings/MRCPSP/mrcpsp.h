#ifndef MRCPSP_H
#define MRCPSP_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>


using namespace std;

class MRCPSP
{

private:

	//INSTANCE DATA
	int nactivities; //Number of non-dummy activites
	int nrenewable; //Number of renewable resources
	int nnonrenewable; //Number of non-renewable resources
	int nresources; //Number of resources (nresouces = nrenewable + nnonrenewable)

	vector<int> ** demand; //Demands of each activity
	vector<int> * succs; //List of successors of each activity
	vector<int> * duration; // Duration of each activity
	int *nmodes; //Number of modes of the activities
	int *capacity; //Capacities of the resources


	//PREPROCESSED DATA
	int ** extPrecs; //Extended time lags
	int ** nSteps; //Minimal number of edges joining two activities
	bool ** resource_incompatibles;
	bool ** tw_incompatibles;
	bool ** resource_disjoints;


	//Statistics
	int ntwincompatibilities;
	int nresincomps;
	int nenergyprecs;
	int ndisjoints;
	int nreducednrdemands;





	int getMostRepDemand(int i, int r) const; //Most repeated demand of activity i over resource r
	int next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik,int t, const vector<int> & smodes);

public:

	MRCPSP(int nactivities, int nrenewable, int nnonrenewable);
   ~MRCPSP();

	int getNActivities() const;
	int getNResources() const;
	int getNRenewable() const;
	int getNNonRenewable() const;

	void setCapacity(int r, int c);
	int getCapacity(int r) const;

	void setDuration(int i, int mode, int p);
	int getDuration(int i, int mode) const;

	void setDemand(int i, int r, int mode, int d);
	int getDemand(int i, int r, int mode) const;

	void addSuccessor(int i, int j);
	const vector<int> & getSuccessors(int i) const;

	int getNModes(int i) const;
	void setNModes(int i, int n);

	void ignoreNR(); //Make this instance have 0 non-renewable resources

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

	int getMinDuration(int i) const; //Minimum duration of activity i among modes
	int getMaxDuration(int i) const; //Maximum duration of activity i among modes

	int getMinDemand(int i, int r) const; //Minimum demmand of activity i over resource r ammong modes
	int getMaxDemand(int i, int r) const; //Maximum demmand of activity i over resource r ammong modes

	vector<int> getModesOrdByDur(int act);

	//Preprocesses
	void computeExtPrecs();
	void recomputeExtPrecs();
	void computeSteps();
	void computeTWIncompatibilities(int UB);
	void computeResourceDisjoints();
	void computeResourceIncompatibilities();
	void computeEnergyPrecedences();
	void reduceNRDemandMin();
	void reduceNRDemandMostFrequent();
	int computePSS(vector<int> & starts, const vector<int> & modes);
	void computeMinPathCover(const vector<int> & vasks, vector<set<int> > & groups);
	void getPossibleParents(int i, int ub, vector<int> & parents);

	//Statistics
	int getNTWIncompatibilities() const;
	int getNResourceIncompatibilities() const;
	int getNPrecedenceIncompatibilities() const;
	int getNEnergyPrecedences() const;
	int getNResourceDisjoints() const;
	int getNReducedNRDemands() const;

	void printSolution(ostream & os, const vector<int> & starts, const vector<int> & modes) const;
	void printPRCPSPSolution(ostream & os, const vector<int> & starts, const vector<int> & modes) const;
	friend ostream &operator <<(ostream &, MRCPSP &);


	void printPractica(ostream & output);
	void printRCP(ostream & output);

};

#endif

