#include "mspsptimegerarquicencoding.h"
#include <set>
#include "util.h"

using namespace smtapi;

MSPSPTimeGerarquicEncoding::MSPSPTimeGerarquicEncoding(MSPSP * instance, SolvingArguments * sargs, Arguments<ProgramArg> * pargs) : MSPSPEncoding(instance) {
	this->sargs = sargs;
	this->pargs = pargs;
}

MSPSPTimeGerarquicEncoding::~MSPSPTimeGerarquicEncoding() {

}


SMTFormula * MSPSPTimeGerarquicEncoding::encode(int lb, int ub){
	int N = ins->getNActivities();
	int R = ins->getNResources();
	int L = ins->getNSkills();

    
    SMTFormula * f = new SMTFormula();
    


	//Start time integer variables
	vector<intvar> S(N+2);
	for (int i=0;i<=N+1;i++){
		S[i]=f->newIntVar("S",i);
		if(1 <= i && i <= N){
			f->addClause(S[i] >= ins->ES(i));
			f->addClause(S[i] <= ins->LS(i,ub));
		}
	}

	//Activity 0 starts at time 0
	f->addClause(S[0] == 0);

	//Set bounds on makespan
	f->addClause(S[N+1] >= lb);
	f->addClause(S[N+1] <= ub);


	//Precedences
	for(int i = 0; i<= N+1; i++)
		for(int j =0; j <= N+1; j++)
			if(ins->isPred(i,j))
				f->addClause(S[j]-S[i] >= ins->getExtPrec(i,j));

	

	//Activity running at time t
	for(int t = 0; t < ub; t++){
		for(int i = 1; i <= N; i++){
			if(ins->ES(i) <= t && t < ins->LC(i,ub)){
				boolvar x = f->newBoolVar("act_time",i,t);
				literal started = S[i] <= t;
				literal noended = t - ins->getDuration(i) < S[i];
                f->addClause(!x | started);
                f->addClause(!x | noended);
                f->addClause(x | !started | !noended);
			}
		}
	}


	for(int t = 0; t < ub; t++){

		vector<vector<vector<literal> > > X(L);
		vector<vector<vector<int> > > Q(L);

		for(int l = 0; l < ins->getNSkills(); l++){

			vector<int> vtasks;
			vector<set<int> > groups;

			//Get only activites that can be running at 't' and demand skill 'l'
			for (int i=1;i<=N;i++){
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (t>=ESi && t<LCi && ins->demandsSkill(i,l)){
					vtasks.push_back(i);
				}
			}

			if(!vtasks.empty())
				ins->computeMinPathCover(vtasks,groups);
		
			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					int demand = ins->getDemand(i,l);

					if(demand != 0){
						vars_part.push_back(f->bvar("act_time",i,t));
						coefs_part.push_back(demand);
					}
				}

				if(!coefs_part.empty()){
					X[l].push_back(vars_part);
					Q[l].push_back(coefs_part);
				}
			}
		}
		vector<int> capacities;
		f->addGerarquicAMOAMK(Q,X,capacities);
	}

	return f;
}

void MSPSPTimeGerarquicEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();
	int R = ins->getNResources();
	int L = ins->getNSkills();

	this->starts=vector<int>(N+2);
	this->assignment.clear();
	this->assignment.resize(N+2);

	for (int i=0;i<=N+1;i++){
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
		for(int l = 0; l < L; l++){
			int demand = ins->getDemand(i,l);
			for(int k = 0; k < R; k++){
				if(ins->hasSkill(k,l) && ins->demandsSkill(i,l)){
					if(SMTFormula::getBValue(ef.f->bvar("act_res_skill",i,k,l),bmodel)){
						if(demand > 0){
							assignment[i].push_back(pair<int,int>(k,l));
							demand--;
						}
					}
				}
			}
		}
	}
}

bool MSPSPTimeGerarquicEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();
	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}

void MSPSPTimeGerarquicEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}
