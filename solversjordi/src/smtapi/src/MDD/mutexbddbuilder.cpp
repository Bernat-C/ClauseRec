#include "MutexBDDBuilder.h"
#include "util.h"



MutexBDDBuilder::MutexBDDBuilder(const vector<boolvar> &X, const map<boolvar,list<boolvar> > & adj_list)
	: MDDBuilder(){
	this->X = X;
	this->adj_list = adj_list;
	this->H = new map<pair<int,int>,MDD *>[X.size()+1];
	this->longedges=true;
	this->depth=X.size();
}


MutexBDDBuilder::~MutexBDDBuilder(){
	delete [] H;
}

MDD *  MutexBDDBuilder::buildMDD(){
	map<boolvar,bool> forbidden;
	for(boolvar v : X)
		forbidden[v] = false;

	return BDDConstruction(0,forbidden);
}

MDD * MutexBDDBuilder::BDDConstruction(int l, map<boolvar,bool> & forbidden){
	if(l == X.size()) //Last layer reached without violating mutex
		return MDD::MDDTrue();
	else{
		MDD * t;

		if(forbidden[X[l]])
			t = MDD::MDDFalse();
		else{
			set<boolvar> forbidden_now;
			for(boolvar v : adj_list[X[l]]){
				if(!forbidden[v]){
					forbidden[v]=true;
					forbidden_now.insert(v);
				}
			}
			t = BDDConstruction(l+1,forbidden);
			for(boolvar v : forbidden_now)
				forbidden[v]=false;
		}

		MDD * f = BDDConstruction(l+1,forbidden);

		if(t==f) return t;
		else{
			map<pair<int,int>,MDD *>::iterator it = H[l].find(pair<int,int>(t->getId(),f->getId()));
			if(it!=H[l].end()) //ja existeix
				return it->second; //obtenir existent
			else{
				MDD * m = new MDD(nodeCount++,X.size()-l);
				m->setElseChild(f);
				m->addChild(X[l],t);
				H[l][pair<int,int>(t->getId(),f->getId())]=m; //inserir m
				return m;
			}
		}
	}
}

void MutexBDDBuilder::createGraphviz(ostream & os, vector<vector<int> > * labels) const{

}





