#ifndef RCPSPMAX_H
#define RCPSPMAX_H
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>

using namespace std;

class RCPSPMAX
{

private:

	//INSTANCE DATA
	int nactivities; //Number of non-dummy activites
	int nresources; //Number of resources

	int ** demand; //Demands of each activity
	vector<int> * succs; //List of successors of each activity
	int * duration; // Duration of each activity
	int * capacity; //Capacities of the resources

	map<int,int> * timelags; //Time lags

	//PREPROCESSED DATA
	int ** extPrecs; //Extended time lags


public:
	RCPSPMAX(int nactivities, int nresources);
	~RCPSPMAX();

	int getNActivities() const;
	int getNResources() const;


	void setCapacity(int r, int c);
	int getCapacity(int r) const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setDemand(int i, int r, int d);
	int getDemand(int i, int r) const;


	void addSuccessor(int i, int j);
	void setTimeLag(int i, int j, int l);
	void addTimeLag(int i, int j, int l);
	int getTimeLag(int i, int j) const;
	const vector<int> & getSuccessors(int i) const;

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

	//Preprocesses
	void computeExtPrecs();

	void computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups);

	friend ostream &operator <<(ostream &, RCPSPMAX &);

};

#endif

