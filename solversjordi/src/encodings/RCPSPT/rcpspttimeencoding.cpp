#include "rcpspttimeencoding.h"
#include <limits.h>
#include "util.h"


RCPSPTTimeEncoding::RCPSPTTimeEncoding(RCPSPT * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) 
	: RCPSPTEncoding(instance, sargs, pargs) {
}

RCPSPTTimeEncoding::~RCPSPTTimeEncoding() {

}

SMTFormula * RCPSPTTimeEncoding::encode(int lb, int ub){
	int N = ins->getNActivities();
	int R = ins->getNResources();

	if(ub > ins->getTimeHorizon()){
		cerr << "The upper bound cannot be higher than the time horizon" << endl;
		exit(BADCODIFICATION_ERROR);
	}
	SMTFormula * f = new SMTFormula();

	vector<intvar> S(N+2);


	//Variables and bounds on start time
	for (int i=0;i<=N+1;i++) {
		S[i] = f->newIntVar("S",i);
		f->addClause(S[i] >= ins->ES(i));
		f->addClause(S[i] <= ins->LS(i,ub));
		for (int t=ins->ES(i);t<=ins->LS(i,ub);t++) {
			literal xt= f->newBoolVar("y",i,t);
			f->addClause(!xt | S[i] == t);
			f->addClause(xt | S[i] != t);
		}
	}

	f->addClause(S[0]==0);
	f->addClause(S[N+1]>=lb);
	f->addClause(S[N+1]<=ub);

	for(int i = 0; i<= N+1; i++)
		for(int j =0; j <= N+1; j++)
			if(ins->isPred(i,j))
				f->addClause(S[j]-S[i] >= ins->getExtPrec(i,j));


	for (int k=0;k<R;k++) {
		for (int t=0;t<ub;t++) {
			vector<vector<literal> > X;
			vector<vector<int> > Q;

			vector<int> vtasques;
			vector<set<int> > groups;

			for (int i=1;i<=N;i++) {
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (ESi<= t && t<LCi)
					vtasques.push_back(i);
			}

			if(!vtasques.empty())
				ins->computeMinPathCover(vtasques,groups);


			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					for (int d=0;d<ins->getDuration(i);d++) {
						int t2 = t - d;
						if(ins->ES(i) <= t2 && t2 <= ins->LS(i,ub)){
							int q=ins->getDemand(i,k,d);
							if (q!=0) {
								vars_part.push_back(f->bvar("y",i,t2));
								coefs_part.push_back(q);
							}
						}
					}
				}

				if(!coefs_part.empty()){
					X.push_back(vars_part);
					Q.push_back(coefs_part);
				}
			}

			if (!X.empty())
				f->addAMOPB(Q,X,ins->getCapacity(k,t),sargs->getAMOPBEncoding());
		}
	}

	return f;
}

void RCPSPTTimeEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();
	this->starts=vector<int>(N+2);
	for (int i=0;i<=N+1;i++)
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
}

bool RCPSPTTimeEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){

	int N = ins->getNActivities();

	if(ub < lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		for(int i = 0; i <= N+1; i++)
			for(int t = max(ins->ES(i),ins->LS(i,ub)+1); t <= ins->LS(i,lastUB); t++)
				ef.f->addClause(!ef.f->bvar("y",i,t));

		return true;
	}
	else return false;
}

void RCPSPTTimeEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}


