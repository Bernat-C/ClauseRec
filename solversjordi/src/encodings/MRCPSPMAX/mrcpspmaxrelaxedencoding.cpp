#include "mrcpspmaxrelaxedencoding.h"
#include "util.h"


MRCPSPMAXRelaxedEncoding::MRCPSPMAXRelaxedEncoding(MRCPSPMAX * instance, AMOPBEncoding amopbenc) : MRCPSPMAXEncoding(instance,amopbenc) {
}

MRCPSPMAXRelaxedEncoding::~MRCPSPMAXRelaxedEncoding() {
}


SMTFormula * MRCPSPMAXRelaxedEncoding::encode(int lb, int ub){

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
		for (int j : ins->getSuccessors(i)){
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


bool MRCPSPMAXRelaxedEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->ivar("S",N+1) <= ub);
		ef.f->addClause(ef.f->ivar("S",N+1) >= lb);
		return true;
	}
	else return false;
}
