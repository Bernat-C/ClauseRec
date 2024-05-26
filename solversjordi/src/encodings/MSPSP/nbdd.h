#ifndef NBDDDEF
#define NBDDDEF

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <climits>
#include <algorithm>
#include <set>

#include "smtapi.h"
class MSPSP;

using namespace std;
using namespace smtapi;


/*
 * Non-deterministic BDD representation of the resource assignment to skills requests of activities of the MSPSP
 * It can be either
 * 	- a trivial/leaf true/false MDD
 * 	- a NBDD with a selector variable. There's one child associated with 'selector=0', 
 * 	  and many childs associated with 'selector=1', each one corresponding to a different skill.
 * Each layer is associated with a different resource
 */
class NBDD {

private:

	//Invariant: id > id of any child (not only direct child)
	int id;

	literal selector;
	vector<NBDD *> truechildren;
	NBDD * falsechild;

	bool istrivialtrue;
	bool istrivialfalse;

	bool recursivedelete;

	vector<vector<int> > occupancies; //Occupancies represented by the node 

   //Number of variables in the Boolean function that this MDD represents, plus 1
	//It is equivalent to the depth of an OMDD without any long edge (i.e. all variables appear in the MDD)
	int vardepth;

	//Number of layers with some node in the MDD. Notice that realdepth<=vardepth
	int realdepth;

	NBDD(bool b); //Trivial NBDD constructor

	//Auxilliary function for the ostream operator
	void print(ostream & s, bool *printed) const;

	//Compute MDD size taking into acount the already visited nodes
	void getSize(bool * visited, int & size) const;

	void unmarkRecursiveDelete(vector<NBDD *> & children);

	static NBDD * constructNBDD(MSPSP * instance,  const vector<int> & resources, int & idCount, vector<map<vector<int>,NBDD*> > & L, int layer,
		map<pair<set<NBDD*>,NBDD*>,NBDD *> & H, vector<int> & occupancy, const vector<int> & goal, int sumgoal,
		NBDD * nbtrue, NBDD * nbfalse);

	void createNodeGraphviz(const vector<int> & goal, int resource, ostream & os);

public:

	//Constructor
	NBDD(int id,int nvars);

	//Destructor
	~NBDD();

	//Get all the selectors
	literal  getSelector() const;

	const vector<NBDD*> & getTrueChildren() const;

	NBDD * getFalseChild() const;

	//Get id
	int getId() const;

	//Get vardepth
	int getVarDepth() const;

	//Get realdepth
	int getRealDepth() const;

	//Get size, linear cost
	int getSize() const;

	//Get Id-based size, constant cost, soundness not guaranteed for nodes
	//different than the root
	int getIdBasedSize() const;

	//Get maximum width of a layer
	//int getLayerWidth();

	//Check if the NBDD is leaf (either true or false)
	bool isLeafNBDD() const;

	//Check if the NBDD is the true leaf
	bool isTrueNBDD() const;

	//Check if the NBDD is the false leaf
	bool isFalseNBDD() const;

	//Add a child to the MDD
	void addTrueChild(NBDD * child);

	//Set the else child of the MDD
	void setFalseChild(NBDD * child);

	bool getAssignment(MSPSP * instance, const vector<int> & resources, const vector<int> & partial_assingment, vector<pair<int,int> > & assignment);

	void getNodesByLayer(vector<bool> & visited, vector<vector<NBDD*> > & layers, int layer);

	void createGraphviz(MSPSP * instance, const vector<int> & resources, const vector<int> & goal, ostream & os);

	static NBDD * constructNBDD(MSPSP * instance, const vector<int> & resources, const vector<int> & goal);


};

#endif
