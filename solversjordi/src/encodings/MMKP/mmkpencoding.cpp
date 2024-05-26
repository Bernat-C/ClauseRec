#include "mmkpencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

MMKPEncoding::MMKPEncoding(MMKP * instance,  Arguments<ProgramArg>  * pargs, SolvingArguments * sargs) : Encoding()  {
	this->instance = instance;
	this->pargs=pargs;
	this->sargs = sargs;
	sharedmdd=NULL;
}

MMKPEncoding::~MMKPEncoding() {

}

SMTFormula * MMKPEncoding::encode(int lb, int ub){

	SMTFormula * f = new SMTFormula();


	for (int i=0; i<instance->n; i++)
		for (int j=0; j < instance->l[i]; j++)
			f->newBoolVar("x",i,j);

	if(pargs->getBoolOption(OBJECTIVE)){
		int K = lb;
		vector<vector<int> > Q;
		vector<vector<literal> > X;
		for (int i=0; i<instance->n; i++){
			vector<int> q;
			vector<literal> x;
			for (int j=0; j < instance->l[i]; j++){
				int profit = instance->v[i][j];
				q.push_back(profit);
				x.push_back(f->bvar("x",i,j));
			}
			if(!q.empty()){
				Q.push_back(q);
				X.push_back(x);
			}
		}
		if(pargs->getBoolOption(REDUCE_EO))
			util::reduceByEO(Q,X,K);
		f->addAMOPBGEQ(Q,X,K,sargs->getAMOPBEncoding());
	}


	for(int k = 0; k < instance->m; k++){
			vector<vector<int> > Q;
			vector<vector<literal> > X;

			for(int i = 0; i < instance->n; i++){
				vector<int> q;
				vector<literal> x;
				for(int j = 0; j < instance->l[i]; j++){
					q.push_back(instance->r[i][j][k]);
					x.push_back(f->bvar("x",i,j));
				}
				if(!q.empty()){
					Q.push_back(q);
					X.push_back(x);
				}
			}

			int K = instance->R[k];

			if(pargs->getBoolOption(REDUCE_EO))
				util::reduceByEO(Q,X,K);

			int nVars = f->getNBoolVars();
			int nClauses = f->getNClauses();

			f->addAMOPB(Q,X,K,sargs->getAMOPBEncoding());

			nVars = f->getNBoolVars() - nVars;
			nClauses = f->getNClauses() - nClauses;

			cerr << "c amopb;" << nVars << ";" << nClauses << endl;
	}


	for(int i = 0; i < instance->n; i++){
		vector<literal> v;
		for(int j = 0; j < instance->l[i]; j++)
			v.push_back(f->bvar("x",i,j));
		f->addAMO(v,sargs->getAMOEncoding());
	}

	for(int i = 0; i < instance->n; i++){
		vector<literal> v;
		for(int j = 0; j < instance->l[i]; j++)
			v.push_back(f->bvar("x",i,j));
		f->addALO(v);
	}
	exit(0);
	return f;
}

bool MMKPEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	/*if(objenc=="smdd" && lb >= lastLB){
		f->addLB(lb,sharedmdd);
		return true;
	}
	else*/ return false;
}

void MMKPEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	this->assignment.resize(instance->n);

	for (int i=0; i<instance->n; i++){
		this->assignment[i]=-1;
		for (int j=0; j < instance->l[i]; j++){
			if(SMTFormula::getBValue(ef.f->bvar("x",i,j),bmodel)){
				this->assignment[i]=j;
				break;
			}
		}
	}
}

int MMKPEncoding::getObjective() const{
	int sum = 0;
	for (int i=0; i<instance->n; i++)
		sum+=instance->v[i][assignment[i]];

	return sum;
}


bool MMKPEncoding::printSolution(ostream & os) const {
	for(int i = 0; i < assignment.size(); i++)
		os << "I_" << i+1 << ":" << (assignment[i]+1) << "; ";

	os << endl;
	return true;
}
