#include "rcpspttaskencoding.h"
#include <limits.h>
#include "util.h"


RCPSPTTaskEncoding::RCPSPTTaskEncoding(RCPSPT * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) 
	: RCPSPTEncoding(instance, sargs, pargs) {
}

RCPSPTTaskEncoding::~RCPSPTTaskEncoding() {

}

SMTFormula * RCPSPTTaskEncoding::encode(int lb, int ub){
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
			literal yt= f->newBoolVar("y",i,t);
			f->addClause(!yt | S[i] == t);
			f->addClause(yt | S[i] != t);
		}
	}

	for(int j=1; j <= N; j++){
		for(int i=1; i <= N; i++) if(i!=j && !ins->inPath(i,j)){
			for(int e = 1 - ins->getDuration(j); e < ins->getDuration(i); e++){
				boolvar z = f->newBoolVar("z",i,j,e);
				f->addClause(!z | S[j]-S[i]==e);
				f->addClause(z | S[j]-S[i]!=e);
			}
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
		for(int j = 1; j <= N; j++){
			int ESj=ins->ES(j);
			int LSj=ins->LS(j,ub);
			int prevDemand=0;
			for(int e = 0; e < ins->getDuration(j); e++){
			//	if(ins->getDemand(j,k,e)>prevDemand){ //Only encode the constraint in demand increases

					vector<vector<literal> > X;
					vector<vector<int> > Q;

					vector<int> vtasques;
					vector<set<int> > groups;

					for (int i=1;i<=N;i++) if(i!=j && !ins->inPath(i,j)){//If activity i and j are not connected by a path
						int ESi=ins->ES(i);
						int LCi=ins->LC(i,ub);
						
						if (!(ESj+e >= LCi || ESi > LSj+e))//If activity i can be running while j starts
							vtasques.push_back(i);
					}

					if(!vtasques.empty())
						ins->computeMinPathCover(vtasques,groups);


					//Get the terms for possibly overlapping activities
					for (const set<int> & group : groups) {
						vector<literal> vars_part;
						vector<int> coefs_part;

						for(int i : group){
							for(int ep = 0; ep < ins->getDuration(i); ep++){
								vars_part.push_back(f->bvar("z",i,j,ep-e));
								coefs_part.push_back(ins->getDemand(i,k,ep));
							}
						}
						X.push_back(vars_part);
						Q.push_back(coefs_part);
					}

					//Get the terms for activity 'j'
					vector<literal> vars_part;
					vector<int> coefs_part;
					int minCoef = INT_MAX;
					for(int t = ESj; t <= LSj; t++){
						int q = -(ins->getCapacity(k,t+e) - ins->getDemand(j,k,e));
						if(q < minCoef)
							minCoef=q;
						coefs_part.push_back(q);
						vars_part.push_back(f->bvar("y",j,t));
					}
					for(int & coef : coefs_part)
						coef-=minCoef;

					X.push_back(vars_part);
					Q.push_back(coefs_part);

					if (!X.empty())
						f->addAMOPB(Q,X,-minCoef,sargs->getAMOPBEncoding());
			//	}
				prevDemand = ins->getDemand(j,k,e);
			}
		}
	}
	return f;
}

void RCPSPTTaskEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();
	this->starts=vector<int>(N+2);
	for (int i=0;i<=N+1;i++)
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
}

bool RCPSPTTaskEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){

	int N = ins->getNActivities();

	if(ub < lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}

void RCPSPTTaskEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}

