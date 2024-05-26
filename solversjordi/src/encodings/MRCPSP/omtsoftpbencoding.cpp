#include "omtsoftpbencoding.h"
#include "util.h"
#include "smtapi.h"

using namespace smtapi;

OMTSoftPBEncoding::OMTSoftPBEncoding(MRCPSP * instance) : MRCPSPEncoding(instance)  {
}

SMTFormula * OMTSoftPBEncoding::encode(int lb, int ub){

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
		for (int j=0;j<=N+1;j++) {

			/*if (i==j && preds[i][j]>0) //Self precedence
				f->addEmptyClause();

			if (i!=j && preds[i][j]!=INT_MIN && preds[j][i]>0)   //Cycle
				f->addEmptyClause();*/

			//Extended precedences. Implied constraint. WARNING: if removed, direct precedence constraints below must be uncommented
			if(ins->isPred(i,j))
				f->addClause(S[j] - S[i] >= ins->getExtPrec(i,j));
		}

		for (int j : ins->getSuccessors(i)) {
			int min=ins->getMinDuration(i);
			for (int k=0;k<ins->getNModes(i);k++) {
				if (min<ins->getDuration(i,k))
					f->addClause(!f->bvar("sm",i,k) | S[j] - S[i] >= ins->getDuration(i,k));
				/*else //Already done by extended precedences
					f->addClause(S[j] - S[i] >= ins->getDuration(k));*/
			}
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
		for (int t=0;t<ub;t++) {
			intvar sum = f->newIntVar("BIGSUM_Ren",r, t,false);
			for (int i=1;i<=N;i++){
				if (t>=ins->ES(i) && t<ins->LC(i,ub)){
					for(int g=0;g<ins->getNModes(i);g++) {
						int weight=ins->getDemand(i,r,g);
						if (weight!=0)
							f->addSoftClauseWithVar(!f->bvar("x",i,t,g),weight,sum);
					}
				}
			}
			f->addClause(sum <= ins->getCapacity(r));
		}
	}

	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {
		intvar sum = f->newIntVar("BIGSUM_NonRen",r,false);
		for (int j=1;j<=N;j++) {
			for (int g=0;g<ins->getNModes(j);g++) {
				int weight=ins->getDemand(j,r,g);
				if (weight!=0)
					f->addSoftClauseWithVar(!f->bvar("sm",j,g),weight,sum);
			}
		}
		f->addClause(sum <= ins->getCapacity(r));
	}

	f->minimize(S[N+1]);
	f->setLowerBound(lb);
	f->setUpperBound(ub); //The bound will be make exclusive in formula output time

	return f;
}

void OMTSoftPBEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();
	this->starts=vector<int>(N+2);
	this->modes=vector<int>(N+2);
	for (int i=0;i<=N+1;i++){
		this->starts[i]=SMTFormula::getIValue(ef.f->ivar("S",i),imodel);
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
				break;
			}
		}
	}
}


OMTSoftPBEncoding::~OMTSoftPBEncoding() {
}
