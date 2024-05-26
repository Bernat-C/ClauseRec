#ifndef MDDOPERATIONBUILDER_DEF
#define MDDOPERATIONBUILDER_DEF

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

#define AND 0
#define OR 1


struct R_M;

class MDDOperationBuilder : public MDDBuilder {

private:
	map<pair<int,int>,MDD *> G;
	map<vector<MDD *>,MDD *> * H;

	MDD * m1;
	MDD * m2;

	int op;

	int nodeCount;
	MDD * apply(MDD * m1, MDD * m2);
	MDD * applyToLeaf(MDD * m1, MDD * m2);
	MDD * mk(const vector<boolvar> & selectors, const vector<MDD *> & children, int depth);

	void createGraphviz(MDD * mdd, ostream & os, bool * visited, vector<vector<int> > * labels) const;

protected:
	virtual MDD * buildMDD();

public:

	//Constructor
	MDDOperationBuilder(MDD * m1, MDD * m2, int op);
	~MDDOperationBuilder();


	virtual void createGraphviz(ostream & os, vector<vector<int> > * labels = NULL) const;
};


#endif
