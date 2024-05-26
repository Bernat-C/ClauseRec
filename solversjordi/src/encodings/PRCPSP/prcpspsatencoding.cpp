#include "prcpspsatencoding.h"
#include "util.h"
#include "assert.h"
#include <limits.h>

PRCPSPSATEncoding::PRCPSPSATEncoding(PRCPSP * instance, AMOPBEncoding amopbenc) : PRCPSPEncoding(instance)  {
	this->amopbenc = amopbenc;
}

SMTFormula * PRCPSPSATEncoding::encode(int lb, int ub){
	int N = ins->getNActivities();
    int N_RR = ins->getNRenewable();

	SMTFormula * f = new SMTFormula();

    // Defining variables
	// Activity i starts at time j.
	vector<vector<literal>> x(N+2);
	vector<vector<literal>> s(N+2);

	std::vector<float> dummyFeatures;
	dummyFeatures.push_back(0); // La variable és d'inicis
	dummyFeatures.push_back(0); // La variable no és de execucions
	dummyFeatures.push_back(-1); // Temps que fa referencia respecte ub
	dummyFeatures.push_back(-1); // Durada de l'activitat respecte la màxima
	dummyFeatures.push_back(-1); // Nombre de successors respecte el màxim
	dummyFeatures.push_back(-1); // Mínim consum de l'activitat respecte la capacitat d'aquell recurs
	dummyFeatures.push_back(-1); // Màxim consum de l'activitat respecte la capacitat d'aquell recurs

	int maxdur = ins->getMaxDuration();
	int maxnsuc = ins->getMaxSuccessors();
	// Initialize Start times
    for(int i = 0; i < N+2; i++) {

		float maxdur_i = ins->getDuration(i) / static_cast< float >(maxdur);
		float nsuc_i = ins->getSuccessors(i).size() / static_cast< float >(maxnsuc);
		float mincs = ins->getMinResourceConsumption(i);
		float maxcs = ins->getMaxResourceConsumption(i);

        x[i].resize(ub);
        for (int t = ins->ES(i); t < ins->LC(i, ub); t++) {
			std::vector<float> vfl;
			vfl.push_back(0); // La variable no és d'inicis
			vfl.push_back(1); // La variable és de execucions
			float tub = static_cast< float >(t) / ub;
			vfl.push_back(tub); // Temps que fa referencia respecte ub
			vfl.push_back(maxdur_i); // Durada de l'activitat respecte la màxima
			vfl.push_back(nsuc_i); // Nombre de successors respecte el màxim
			vfl.push_back(mincs); // Mínim consum de l'activitat respecte la capacitat d'aquell recurs
			vfl.push_back(maxcs); // Màxim consum de l'activitat respecte la capacitat d'aquell recurs
            x[i][t]= f->newBoolVar("x",i,t,vfl);
        }

        s[i].resize(ub+1);
        for (int t = ins->ES(i); t <= ins->LS(i, ub); t++) {
			std::vector<float> vfl;
			vfl.push_back(1); // La variable és d'inicis
			vfl.push_back(0); // La variable no és de execucions
			float tub = static_cast< float >(t) / ub;
			vfl.push_back(tub); // Temps que fa referencia respecte ub
			vfl.push_back(maxdur_i); // Durada de l'activitat respecte la màxima
			vfl.push_back(nsuc_i); // Nombre de successors respecte el màxim
			vfl.push_back(mincs); // Mínim consum de l'activitat respecte la capacitat d'aquell recurs
			vfl.push_back(maxcs); // Màxim consum de l'activitat respecte la capacitat d'aquell recurs
            s[i][t]= f->newBoolVar("s",i,t,vfl);
        }
	}
	// Relation x - s
	for (int i=1;i < N+1;i++) {
		// If it is in execution, it has started
        for (int t = ins->ES(i); t < ins->LS(i, ub); t++)
			f->addClause(!f->bvar("x",i,t) |  f->bvar("s",i,t));

		// Each activity has exactly duration times where it is running.
		vector<literal> exec_times;
        for (int t = ins->ES(i); t < ins->LC(i, ub); t++)
			exec_times.push_back(f->bvar("x",i,t));
		
        f->addEK(exec_times,ins->getDuration(i),&dummyFeatures); 
		
		exec_times.clear();

		int ESi=ins->ES(i);
        int LSi=ins->LS(i,ub);
        int ECi=ins->EC(i);
        int LCi=ins->LC(i,ub);
        
		if(ESi>LSi) {
            f->addClause(f->falseVar(&dummyFeatures));
			return f;
		}

        f->addClause(f->bvar("x",i,ESi) | !f->bvar("s",i,ESi));
		int m = LSi;
		if (LSi==LCi)
			LSi--; // To avoid accessing a variable that does not exist
        for(int t=ESi+1; t <= LSi; t++){
            f->addClause(f->bvar("x",i,t) | !f->bvar("s",i,t) | f->bvar("s",i,t-1));
		}
	}
    // If precedence exists i -> j, i can't be in execution after s(j)
	for (int i=0;i<N+2;i++) {
		if(ins->ES(i)<=ins->LS(i,ub))
			f->addClause(f->bvar("s",i,ins->LS(i, ub)));
		else {			
            f->addClause(f->falseVar(&dummyFeatures));
			return f;
		}

		for(int j : ins->getSuccessors(i)) {
			for (int t = ins->ES(i); t < ins->LC(i, ub); t++) {
				if(ins->LS(j,ub)<t)
					f->addClause(!f->bvar("x",i,t));
				else if(ins->ES(j)<=t){
					f->addClause(!f->bvar("s",j,t) | !f->bvar("x",i,t));
				}
				// Altrament t < ESj
			}
		}
	}
	// Horizontal implications
	for (int i=0;i < N+2;i++) {
        for (int t = ins->ES(i); t < ins->LS(i, ub); t++) {
			f->addClause(!f->bvar("s",i,t) | f->bvar("s",i,t+1));
		}
	}

	for (int r=0;r<N_RR;r++) {
		for (int t=0;t<ub;t++) {
			vector<vector<literal> > X;
			vector<vector<int> > Q;

			vector<int> vtasques;
			vector<set<int> > groups;

			for (int i=1;i<=N;i++) {
				int ESi=ins->ES(i);
				int LCi=ins->LC(i,ub);
				if (t>=ESi && t<LCi)
					vtasques.push_back(i);
			}

			if(!vtasques.empty())
				ins->computeMinPathCover(vtasques,groups);

			for (const set<int> & group : groups) {
				//std::cout << "c group_"  << r << "_" << t << ":";
				vector<literal> vars_part;
				vector<int> coefs_part;
				bool printed = false;
				for(int i : group){
					int auxir=ins->getDemand(i,r);
					if (auxir!=0) {
						//std::cout << " x_" << i << "_" << t << "_" << g;
						boolvar x = f->bvar("x",i,t);
						vars_part.push_back(x);
						coefs_part.push_back(auxir);
					}
				}
				if(!coefs_part.empty()){
					X.push_back(vars_part);
					Q.push_back(coefs_part);
				}
				// std::cout << std::endl;
			}


			if (!X.empty()){
				util::sortCoefsDecreasing(Q,X);
				f->addAMOPB(Q,X,ins->getCapacity(r),amopbenc,&dummyFeatures);
			}
		}
	}

	// Extended Precedences
	for (int i=0;i < N+2;i++) {
		for (int j=0;j < N+2;j++) { 
			int extprec = ins->getExtPrec(i,j);
			int maxt = std::min(ins->LS(j,ub), ins->LS(i, ub));
			if(extprec>0) {
				for (int t = ins->ES(j); t < maxt; t++) {
					if (t-extprec >= ins->ES(i))
						f->addClause(!f->bvar("s",j,t) | f->bvar("s",i,t-extprec));
					else
						f->addClause(!f->bvar("s",j,t));
				}
			}
		}
	}

	return f;
}

void PRCPSPSATEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){

	int N = ins->getNActivities();
	int N_RR = ins->getNRenewable();
	int N_R = ins->getNResources();

	this->en_execucio=vector<vector<bool>>(N+2);
	this->starts=vector<int>(N+2);

	for (int i=0;i<N+2;i++) {
		
		int ESi=ins->ES(i);
	    int LSi=ins->LS(i,ub);

        for(int t = ESi; t <= LSi; t++){
			if(SMTFormula::getBValue(ef.f->bvar("s",i,t),bmodel)) {
				this->starts[i]=t;
				break;
			}
		}
	}

	for (int i=0;i<N+2;i++) {
		
		int ESi=ins->ES(i);
	    int LSi=ins->LS(i,ub);

        for(int t = ESi; t <= LSi; t++){
			if(SMTFormula::getBValue(ef.f->bvar("s",i,t),bmodel)) {
				this->starts[i]=t;
				break;
			}
		}
	}
	
    for (int i=0;i< N+2;i++) {

		int ESi=ins->ES(i);
	    int LCi=ins->LC(i,ub);
		
		this->en_execucio[i] = vector<bool>(ub,false);

        for(int t = ESi; t < LCi; t++){
			if(SMTFormula::getBValue(ef.f->bvar("x",i,t),bmodel)) {
				this->en_execucio[i][t]=true;
			}
			else
				this->en_execucio[i][t]=false;
        }
    }
}

PRCPSPSATEncoding::~PRCPSPSATEncoding() {
}
