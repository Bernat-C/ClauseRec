#include "rcpspmaxencoding.h"
#include "util.h"


RCPSPMAXEncoding::RCPSPMAXEncoding(RCPSPMAX * instance, AMOPBEncoding amopbenc) : Encoding() {
	this->ins = instance;
	this->amopbenc = amopbenc;
}

RCPSPMAXEncoding::~RCPSPMAXEncoding() {

}



SMTFormula * RCPSPMAXEncoding::encode(int lb, int ub){

	int N = ins->getNActivities();
	int N_R = ins->getNResources();

	SMTFormula * f = new SMTFormula();

	//Start time integer variables
	vector<intvar> S(N+1);
	for (int i=0;i<=N+1;i++)
		S[i]=f->newIntVar("S",i);

	//Activity 0 starts at time 0
	f->addClause(S[0] == 0);

	//Set bounds on makespan
	f->addClause(S[N+1] >= lb);
	f->addClause(S[N+1] <= ub);


	//Precedence constraints
	for (int i=0;i<=N+1;i++) {
		//Precedences
		for (int j : ins->getSuccessors(i))
			f->addClause(S[j] - S[i] >= ins->getTimeLag(i,j));

		//Extended precedences (implied)
		for (int j=0;j<=N+1;j++) {
			if(ins->startsBefore(i,j))
				f->addClause(S[j] - S[i] >= ins->getExtPrec(i,j));
		}
	}


	//Definition of x_i,t
	for (int i=1;i<=N;i++) {
		for (int t=ins->ES(i);t<ins->LC(i,ub);t++) {
			boolvar x = f->newBoolVar("x",i,t);
			literal geSi = S[i] <= t;
			literal ltCi = (t-ins->getDuration(i)) < S[i];
			f->addClause(!x | geSi);
			f->addClause(!x | ltCi);
			f->addClause(x | !geSi | !ltCi);
		}
	}

	//Renewable resource constraints
	for (int r=0;r<N_R;r++) {
		vector<int> lastvtasks;
		vector<set<int> > groups;

		for (int t=0;t<ub;t++) {
			vector<vector<literal> > vars_group;
			vector<vector<int> > coefs_group;
			vector<int> vtasks;

			for (int i=1;i<=N;i++)
				if (t>=ins->ES(i) && t<ins->LC(i,ub))
					vtasks.push_back(i);

			if(lastvtasks != vtasks && !vtasks.empty()){
				groups.clear();
				ins->computeMinPathCover(vtasks,groups);
			}


			for (const set<int> & group : groups) {
				vector<literal> vars_part;
				vector<int> coefs_part;

				for(int i : group){
					int auxir=ins->getDemand(i,r);
					if (auxir!=0) {
						vars_part.push_back(f->bvar("x",i,t));
						coefs_part.push_back(auxir);
					}
				}

				if(!coefs_part.empty()){
					vars_group.push_back(vars_part);
					coefs_group.push_back(coefs_part);
				}
			}

			util::sortCoefsDecreasing(coefs_group,vars_group);

			if (!vars_group.empty())
				f->addAMOPB(coefs_group,vars_group,ins->getCapacity(r),amopbenc);
		}
	}

	return f;
}

void RCPSPMAXEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();

	this->starts=vector<int>(N+2);
	this->modes=vector<int>(N+2);
	for (int i=0;i<=N+1;i++)
		this->starts[i]=ef.f->getIValue(ef.f->ivar("S",i),imodel);
}

bool RCPSPMAXEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				ef.f->addClause(!ef.f->bvar("x",i,t));

		return true;
	}
	else return false;
}

void RCPSPMAXEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}


int RCPSPMAXEncoding::getObjective() const{
	return starts.back();
}

bool RCPSPMAXEncoding::printSolution(ostream & os) const{
	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";

	os << endl;
	return true;
}
