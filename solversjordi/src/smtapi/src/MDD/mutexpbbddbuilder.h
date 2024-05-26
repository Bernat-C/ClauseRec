#ifndef MUTEXPBMDDBUILDER_DEF
#define MUTEXPBMDDBUILDER_DEF

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


struct R_M;

class MutexPBMDDBuilder : public MDDBuilder {

private:
	vector<vector<MDD *> > L_MDDs;
	vector<vector<R_M> > L;

	int K;
	vector<int> Q;
	vector<boolvar> X;

	void initL();
	void mostrarL(int i_l=0);
	void insertMDD(R_M rb_in,int i_l);
	int inf_sum(int possible_inf, int x);

	R_M searchMDD(int i_k,int i_l);

	R_M MDDConstruction(int i_l, int i_k);
	int getmax(const vector<int> & v);

protected:
	virtual MDD * buildMDD();

public:

	//Constructor
	PBMDDBuilder(const vector<vector<int> > &Q,const vector<vector<boolvar> > &X, int K, bool longedges=true);
	PBMDDBuilder(const vector<int> &Q,const vector<boolvar> &X, int K, bool longedges=true);
	~PBMDDBuilder();


	virtual void createGraphviz(ostream & os, vector<vector<int> > * labels = NULL) const;
};


//Encapsulation of an MDD with its interval [B,Y]
struct R_M {
MDD *mdd;
int B;
int Y;
R_M() {
    mdd=NULL;
    B=0;
    Y=0;
}
R_M(MDD * m, int b, int y) {
    mdd=m;
    B=b;
    Y=y;
}
};


#endif
