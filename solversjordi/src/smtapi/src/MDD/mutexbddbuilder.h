#ifndef MUTEXBDDBUILDER_DEF
#define MUTEXBDDBUILDER_DEF

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <list>
#include <yices.h>
#include <climits>
#include <algorithm>
#include "encoders/encoder.h"
#include "MDDBuilder.h"
#include "MDD.h"


class MutexBDDBuilder : public MDDBuilder {

private:

	vector<boolvar> X;
	map<boolvar,list<boolvar> > adj_list;

	map<pair<int,int>,MDD *> * H;

	MDD * BDDConstruction(int l, map<boolvar,bool> & forbidden);

protected:
	virtual MDD * buildMDD();

public:

	//Constructor
	MutexBDDBuilder(const vector<boolvar> &X, const map<boolvar,list<boolvar> > & adj_list);
	~MutexBDDBuilder();


	virtual void createGraphviz(ostream & os, vector<vector<int> > * labels = NULL) const;
};



#endif
