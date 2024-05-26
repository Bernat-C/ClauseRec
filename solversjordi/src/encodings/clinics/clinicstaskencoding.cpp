#include "clinicstaskencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

ClinicsTaskEncoding::ClinicsTaskEncoding(Clinics * instance) : ClinicsEncoding(instance)  {
}

ClinicsTaskEncoding::~ClinicsTaskEncoding() {

}

SMTFormula * ClinicsTaskEncoding::encode(int lb, int ub){

	SMTFormula * f = new SMTFormula();
	int n = ins->getNActivities();
	vector<intvar> S(n+2);

	//Create start time variables and set time windows
	for(int i = 0; i < n+2; i++){
		S[i] = f->newIntVar("S",i);
		f->addClause(S[i] >= ins->ES(i));
		f->addClause(S[i] <= ins->LS(i,ub));
	}

	//Start at time 0
	f->addClause(S[0]==0);

	//Bound objective function
	f->addClause(S[n+1]>=lb);
	f->addClause(S[n+1]<=ub);

	//Add precedences
	for(int i = 0; i < n+2; i++)
		for(int j = 0; j < n+2; j++)
			if(i!=j && ins->getExtPrec(i,j)!=INT_MIN)
				f->addClause(S[j]-S[i]>=ins->getExtPrec(i,j));

	//Add mutual exclucions
	for(int i = 1; i <= n-1; i++){
		for(int j = i+1; j <= n; j++){
			if(ins->needMutex(i,j)){
				int duri, durj;
				if(ins->needClean(i,j)){
					duri=ins->getCleanDuration(i);
					durj=ins->getCleanDuration(j);
				}
				else{
					duri=ins->getDuration(i);
					durj=ins->getDuration(j);
				}
				f->addClause(S[i] - S[j] >= durj | S[j] - S[i] >= duri);
			}
		}
	}

	//Add resource constraints
	for(int r = 0; r < ins->getNResources(); r++){
		int d1 = 0;
		for(const pair<int,int> & p1 : ins->getDemands(r)){
			int s1 = p1.first; //Start
			int e1 = p1.second; //End
			int d2 = 0;
			vector<literal> lits;
			for(const pair<int,int> & p2 : ins->getDemands(r)){
				if(d1 != d2){
					int s2 = p2.first; //Start
					int e2 = p2.second; //End
					//z <-> Ss2 <= Ss1 & Ss1 < Se2 + dur(e2)
					boolvar z = f->newBoolVar("z",d1,d2,r);
					literal l1 = S[s2] <= S[s1];
					literal l2 = S[s1]-S[e2] < ins->getDuration(e2);
					f->addClause(!z | l1);
					f->addClause(!z | l2);
					f->addClause(z | !l1 | !l2);
					lits.push_back(z);
				}
				d2++;
			}
			if(!lits.empty())
				f->addAMK(lits,ins->getCapacity(r)-1);
			d1++;
		}
	}

	return f;
}

bool ClinicsTaskEncoding::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
    if(lastLB <= lb && lastUB >= ub){
		int n = ins->getNActivities();
		ef.f->addClause(ef.f->ivar("S",n+1)>=lb);
		ef.f->addClause(ef.f->ivar("S",n+1)<=ub);
		return true;
    }
    else return false;
}
