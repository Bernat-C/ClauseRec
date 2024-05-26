#ifndef MSPSP_H
#define MSPSP_H
#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include<set>
#include "nbdd.h"


using namespace std;

class MSPSP
{

private:

	int nactivities; //Number of non-dummy activites
	int nresources; //Number of resources
	int nskills; //Number of skills

	bool ** resHasSkill; //Resource has skill
	vector<vector<int> >  actSkillDemand; //Demand of each activity for each skill

	vector<int> * succs; //List of successors of each activity
	int ** extPrecs; //Extended precedences
	int * duration; //Duration of each activity

	int * nResourcesMastering; //Number of resources mastering each skill

	vector<NBDD*> nbdds;
	vector<vector<int> > nbddOrderedResources;

	//Data used in implied constraints
	vector<vector<int> > res_types; //Resources grouped by type (set of skills that master)

	vector<bool> dominatedSets; //Indicates whether a subset of skills is dominated

	list<pair<pair<int,int>,int> > energy_precs; //Precedences increased due to enerergy reasoning

	//Preprocess
	void computeExtPrecs();
	void computeNResourcesMastering();
	void computeResTypes();
	void computeNBDDOrder();
	void computeNBDDs();
	void computeDominatedSets();


	int nMastered(int k) const;

	int next_activity(const vector<vector<int> > & ordered_res, const vector<vector<int> > & predecessors,
		set<int> & remaining,const set<int> & C, vector<bool> & occupation,
		vector<pair<int,int> > & used_res, std::set<std::vector<int> > & U);

    bool getAssignment(vector<int> & dem, int demsum, vector<bool> & fr, int k, int rescount, std::set<std::vector<int> > & U, vector<pair<int,int> > & used_res);


public:
	MSPSP(int nactivities, int nresources, int nskills);

	int getNActivities() const;
	int getNResources() const;
	int getNSkills() const;
	int getNResourcesMastering(int l) const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setResourceSkill(int k, int l);
	bool hasSkill(int k, int l) const;


	void setDemand(int i, int l, int d);
	int getDemand(int i, int l) const;
	const vector<int> & getDemands(int i) const;
	bool demandsSkill(int i, int l) const;
	int getTotalDemand(int i) const;
	bool usefulToActivity(int k, int i) const;
	const vector<int> & getNBDDOrderedResources(int i) const;

	
	int getNResourceTypes() const;
	const vector<int> & getResourcesOfType(int t) const;

	void getCapacities(vector<int> & capacities) const;
	
	NBDD * getNBDD(int i) const;

	void addSuccessor(int i, int j);
	const vector<int> & getSuccessors(int i) const;

	void preProcess();
	void preProcessNBDD();

	void computeEnergyPrecedences();
	void recomputeExtPrecs();

	int computePSS(vector<int> & starts, vector<vector<pair<int,int> > > & assignment);

	int getExtPrec(int i, int j) const;
	bool isPred(int i, int j) const;
	bool inPath(int i, int j) const;

	int trivialUB() const;
	int trivialLB() const;
	int ES(int i) const;
	int LS(int i, int ub) const;
	int EC(int i) const;
	int LC(int i, int ub) const;

	void computeMinPathCover(const vector<int> & vasks, vector<set<int> > & groups) const;

	void makeDurationsUnitary();

	void makeResourceMasteryGerarquic();

	bool isDominated(int subset); //Indicates whether a subset of skills is cominated

	~MSPSP();

	friend ostream &operator <<(ostream &, MSPSP &);

	void printJSON(int UB); 

	void printSolution(ostream & os, const vector<int> & starts, const vector<vector<pair<int,int> > > & assignment);
	void printCTLPInstance(ostream & f, const int UB, bool useint);
	void printDTLPInstance(ostream & f, const int UB, bool dt);
	void printJCLPInstance(ostream & f, const int UB, bool implicates, bool useint, bool reif);

	void printDZN(ostream &, int maxt_heur) const;


};

#endif

