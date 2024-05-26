#include "mrcpspmaxencoding.h"
#include "util.h"


MRCPSPMAXEncoding::MRCPSPMAXEncoding(MRCPSPMAX * instance, AMOPBEncoding amopbenc) : Encoding() {
	this->ins = instance;
	this->amopbenc = amopbenc;
}

MRCPSPMAXEncoding::~MRCPSPMAXEncoding() {

}



SMTFormula * MRCPSPMAXEncoding::encode(int lb, int ub){

	int N = ins->getNActivities();
	int N_RR = ins->getNRenewable();
	int N_NR = ins->getNNonRenewable();
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


	//Execution modes of the activities
	for (int i=0;i<=N+1;i++) {
		vector<literal> vmodes;
		for (int p=0;p<ins->getNModes(i);p++)
			vmodes.push_back(f->newBoolVar("sm",i,p));
		f->addEO(vmodes); //Each activity has exactly one execution mode
	}


	//Precedence constraints
	for (int i=0;i<=N+1;i++) {
		//Precedences
		for (int j : ins->getSuccessors(i)) {
			for (int mi=0;mi<ins->getNModes(i);mi++)
				for (int mj=0;mj<ins->getNModes(j);mj++)
					f->addClause(!f->bvar("sm",i,mi) | !f->bvar("sm",j,mj) | S[j] - S[i] >= ins->getTimeLag(i,j,mi,mj));
		}

		//Extended precedences (implied)
		for (int j=0;j<=N+1;j++) {
			if(ins->startsBefore(i,j))
				f->addClause(S[j] - S[i] >= ins->getExtPrec(i,j));
		}
	}


	//Definition of x_i,t,o
	for (int i=1;i<=N;i++) {
		for (int g=0;g<ins->getNModes(i);g++) {
			for (int t=ins->ES(i);t<ins->LC(i,ub);t++) {
				boolvar x = f->newBoolVar("x",i,t,g);
				boolvar sm = f->bvar("sm",i,g);
				literal geSi = S[i] <= t;
				literal ltCi = (t-ins->getDuration(i,g)) < S[i];

				f->addClause(!x | geSi);
				f->addClause(!x | ltCi);
				f->addClause(!x | sm);
				f->addClause(x | !geSi | !ltCi | !sm);
			}
		}
	}

	//Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
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
					for (int g=0;g<ins->getNModes(i);g++) {
						int auxir=ins->getDemand(i,r,g);
						if (auxir!=0) {
							vars_part.push_back(f->bvar("x",i,t,g));
							coefs_part.push_back(auxir);
						}
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


	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {
		vector<vector<literal> > vars_group;
		vector<vector<int> > coefs_group;
		for (int i=1;i<=N;i++) {
			vector<literal> vars_part;
			vector<int> coefs_part;
			for (int g=0;g<ins->getNModes(i);g++) {
				int d=ins->getDemand(i,r,g);
				if (d!=0) {
					vars_part.push_back(f->bvar("sm",i,g));
					coefs_part.push_back(d);
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

	return f;
}

void MRCPSPMAXEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();


	this->starts=vector<int>(N+2);
	this->modes=vector<int>(N+2);
	for (int i=0;i<=N+1;i++){
		this->starts[i]=ef.f->getIValue(ef.f->ivar("S",i),imodel);
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
				break;
			}
		}
	}
}

bool MRCPSPMAXEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				for(int g = 0; g < ins->getNModes(i); g++)
					ef.f->addClause(!ef.f->bvar("x",i,t,g));

		return true;
	}
	else return false;
}

void MRCPSPMAXEncoding::assumeBounds(const EncodedFormula & ef, int lb, int ub, vector<literal> & assumptions){
	int N = ins->getNActivities();
	assumptions.push_back(ef.f->ivar("S",N+1) <= ub);
	assumptions.push_back(ef.f->ivar("S",N+1) >= lb);
}


int MRCPSPMAXEncoding::getObjective() const{
	return starts.back();
}

void MRCPSPMAXEncoding::getModes(vector<int> &modes){
	modes.resize(this->modes.size());
	for(int i = 0; i < modes.size(); i++)
		modes[i]=this->modes[i];
}


void MRCPSPMAXEncoding::getStartsAndModes(vector<int> &starts, vector<int> &modes){
	modes.resize(this->modes.size());
	for(int i = 0; i < modes.size(); i++)
		modes[i]=this->modes[i];

	starts.resize(this->starts.size());
	for(int i = 0; i < starts.size(); i++)
		starts[i]=this->starts[i];
}

bool MRCPSPMAXEncoding::printSolution(ostream & os) const{
	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";

	os << endl;
	os << "v ";
	for(int i = 0; i < modes.size(); i++)
		os << "M_" << i << ":" << modes[i]+1 << "; ";
	os << endl;

	return true;
}
