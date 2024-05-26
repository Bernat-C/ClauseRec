#ifndef MRCPSPMAX_H
#define MRCPSPMAX_H
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include "rcpspmax.h"


using namespace std;

class MRCPSPMAX
{

private:

	//INSTANCE DATA
	int nactivities; //Number of non-dummy activites
	int nrenewable; //Number of renewable resources
	int nnonrenewable; //Number of non-renewable resources
	int nresources; //Number of resources (nresouces = nrenewable + nnonrenewable)

	vector<vector<int> > * demand; //Demands of each activity
	vector<int> * succs; //List of successors of each activity
	vector<int> * duration; // Duration of each activity
	int *nmodes; //Number of modes of the activities
	int *capacity; //Capacities of the resources
	map<int,int** > * timelags; //Time lags

	//PREPROCESSED DATA
	int ** extPrecs; //Extended time lags


public:
	MRCPSPMAX(int nactivities, int nrenewable, int nnonrenewable);
	~MRCPSPMAX();

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

	void setTimeLag(int i, int j, int mi, int mj, int l);
	int getTimeLag(int i, int j, int mi, int mj) const;


	int getExtPrec(int i, int j) const;
	int trivialUB() const;
	int trivialLB() const;
	int ES(int i) const;
	int LS(int i, int ub) const;
	int EC(int i) const;
	int LC(int i, int ub) const;
	bool inPath(int i, int j) const;
	bool isPred(int i, int j) const;
	bool startsBefore(int i, int j) const;

	int getMinDuration(int i) const; //Minimum duration of activity i among modes
	int getMaxDuration(int i) const; //Maximum duration of activity i among modes

	int getMinTimeLag(int i, int j) const; //Minimum time lag between i and j among modes of i and j
	int getMaxTimeLag(int i, int j) const; //Maximum time lag between i and j among modes of i and j

	int getMinTimeLag(int i, int j, int mi) const; //Minimum time lag between i, in mode mi, and j among modes of j
	int getMaxTimeLag(int i, int j, int mj) const; //Maximum time lag between i, in mode mi, and j among modes of j

	int getMinDemand(int i, int r) const; //Minimum demmand of activity i over resource r ammong modes
	int getMaxDemand(int i, int r) const; //Maximum demmand of activity i over resource r ammong modes
	int getMostRepDemand(int i, int r) const; //Most repeated demand of activity i over resource r

	RCPSPMAX * getRCPSPMAX(const vector<int> & modes) const; //Get the corresponding RCPSPMAX instances for the given modes

	//Preprocesses
	void computeExtPrecs();
	void recomputeExtPrecs();

	void computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups);

	void reduceNRDemandMin();
	void reduceNRDemandMostFrequent();


	friend ostream &operator <<(ostream &, MRCPSPMAX &);

};

#endif

