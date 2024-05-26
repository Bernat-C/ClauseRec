#include "mrcpspsatencoding.h"
#include "util.h"
#include <limits.h>

MRCPSPSATEncoding::MRCPSPSATEncoding(MRCPSP * instance, AMOPBEncoding amopbenc) : MRCPSPEncoding(instance)  {
	this->amopbenc = amopbenc;
}

SMTFormula * MRCPSPSATEncoding::encode(int lb, int ub){
	int N = ins->getNActivities();
    int N_RR = ins->getNRenewable();
    int N_NR = ins->getNNonRenewable();
	int N_R = ins->getNResources();

	SMTFormula * f = new SMTFormula();

	//Execution modes of the activities
	for (int i=0;i<=N+1;i++) {
		vector<literal> vmodes;
		for (int p=0;p<ins->getNModes(i);p++)
			vmodes.push_back(f->newBoolVar("sm",i,p));
		f->addEO(vmodes); //Each activity has exactly one execution mode
	}

	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {
		vector<vector<literal> > vars_group;
		vector<vector<int> > coefs_group;
		for (int j=1;j<=N;j++) {
			vector<literal> vars_part;
			vector<int> coefs_part;
			for (int g=0;g<ins->getNModes(j);g++) {
				int d=ins->getDemand(j,r,g);
				if (d!=0) {
					vars_part.push_back(f->bvar("sm",j,g));
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

void MRCPSPSATEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	int N = ins->getNActivities();
	int N_RR = ins->getNRenewable();
	int N_NR = ins->getNNonRenewable();
	int N_R = ins->getNResources();


	this->modes=vector<int>(N+2);
	this->modes[0] = 0;
	this->modes[N+1] = 0;

	for (int i=1;i<=N;i++)
		for (int p=0;p<ins->getNModes(i);p++)
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel))
				this->modes[i]=p;
}


MRCPSPSATEncoding::~MRCPSPSATEncoding() {
}
