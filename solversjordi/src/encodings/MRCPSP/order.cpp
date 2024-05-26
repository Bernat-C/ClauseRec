#include "order.h"
#include "util.h"
#include <limits.h>


Order::Order(MRCPSP * instance, AMOPBEncoding amopbenc, int predeforder, bool maxsat) : MRCPSPEncoding(instance)  {
	this->maxsat = maxsat;
    this->predeforder = predeforder;
	this->amopbenc = amopbenc;
	this->usepb = false;
	this->printpb = false;
	this->printamo = false;
}

void Order::setPrintPB(const string & filename){
	this->printpb = true;
	this->pbfilename = filename;
}

void Order::setPrintAMO(const string & filename){
	this->printamo = true;
	this->amofilename = filename;
}

void Order::setUsePB(PBEncoding pbenc){
	this->usepb = true;
	this->pbenc = pbenc;
} 

SMTFormula * Order::encode(int lb, int ub){
   int N = ins->getNActivities();
   int N_RR = ins->getNRenewable();
   int N_NR = ins->getNNonRenewable();
   int N_R = ins->getNResources();

   SMTFormula * f = new SMTFormula();
    
   vector<literal> order;
   std::vector<float> vfl;

   //Execution modes of the activities
	for (int i=0;i<=N+1;i++) {
		vector<literal> vmodes;
        for (int p=0;p<ins->getNModes(i);p++){
            boolvar v =f->newBoolVar("sm",i,p,vfl);
            order.push_back(v);
			vmodes.push_back(v);
        }
		f->addEO(vmodes); //Each activity has exactly one execution mode
	}

	//Creation of x_i,t,o
	for (int i=0;i<=N+1;i++)
		for (int g=0;g<ins->getNModes(i);g++)
            for (int t=ins->ES(i);t<ins->LC(i,ub);t++)
				order.push_back(f->newBoolVar("x",i,t,g,vfl));
    


	for(int i=0;i<N+2; i++){
      //Order encoding of the start times
	  int ESi=ins->ES(i);
	  int LSi=ins->LS(i,ub);
      int ECi=ins->EC(i);
      int LCi=ins->LC(i,ub);
      if(ESi > LSi){
         f->addEmptyClause();
         return f;
      }
		for(int t=ESi; t <= LSi; t++){
			boolvar o = f->newBoolVar("o",i,t,vfl); //Create variable o_{i,t}
            order.push_back(o);
			if(t>ESi)
				f->addClause(!f->bvar("o",i,t-1) | o); // o_{i,t-i} -> o_{i,t}
		}
      f->addClause(f->bvar("o",i,LSi)); //It has started for sure at the latest start time
	}

   std::cout << "c PROBLEM VARS " << f->getNBoolVars() << std::endl;
   f->addClause(f->bvar("o",0,0)); //S_0 = 0
   if(lb>ins->ES(N+1))
      f->addClause(!f->bvar("o",N+1,lb-1));

	//Definition of x_{i,t,o}
	for(int i=0; i < N+2; i++){
		for(int m = 0; m < ins->getNModes(i); m++){
			int ESi=ins->ES(i);
			int LSi=ins->LS(i,ub);
			int ECi=ins->EC(i);
			int LCi=ins->LC(i,ub);
			int dur = ins->getDuration(i,m);
			if(LSi >= ESi){
				for(int t=ESi; t < LCi; t++){
					boolvar x = f->bvar("x",i,t,m);
					boolvar o = t < ESi ? f->falseVar(&vfl) : t > LSi ? f->trueVar(&vfl) : f->bvar("o",i,t);
					boolvar o2 = t-dur < ESi ? f->falseVar(&vfl) : t-dur > LSi ? f->trueVar(&vfl) : f->bvar("o",i,t-dur);
					boolvar sm = f->bvar("sm",i,m);
					f->addClause(!sm | !o | o2 | x );
					f->addClause(sm | !x );
					f->addClause(o | !x );
					f->addClause(!o2| !x );
				}
			}
		}
	}

	//Precedences
	for(int i=0; i< N+2; i++){
		int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);
		for(int j : ins->getSuccessors(i)){
			int ESj=ins->ES(j);
			int LSj=ins->LS(j,ub);


			for(int m = 0; m < ins->getNModes(i); m++){
				int dur = ins->getDuration(i,m);
				int mint = min(ESi,ESj-dur);
				int maxt = max(LSj-dur,LSi);
				for(int t=mint; t <= maxt; t++){
					boolvar oi;
					boolvar oj;
					boolvar sm = f->bvar("sm",i,m); // i runs in mode m
					if(t < ESi)
						oi = f->falseVar(&vfl);
					else if(t > LSi)
						oi = f->trueVar(&vfl);
					else
						oi=f->bvar("o",i,t);   // i starts at t or earlier
					if(t+dur<ESj)
						oj=f->falseVar(&vfl);
					else if(t +dur > LSj)
						oj=f->trueVar(&vfl);
					else
						oj=f->bvar("o",j,t+dur);  //  j starts at t plus dur or earlier
					f->addClause(!sm | oi | !oj);
				}
			}
		}
	}

	ofstream os;
	stringstream ss;

	ofstream osamo;

	if(printamo){
		osamo.open(amofilename.c_str());
		if(!osamo.is_open()){
			cerr << "Error: could not open file: " << amofilename << endl;
			exit(BADFILE_ERROR);
		}
	}

	int nconstraints = 0;
	boolvar bmax;
  	//Renewable resource constraints
	for (int r=0;r<N_RR;r++) {
		for (int t=0;t<ub;t++) {
            char aux[50];
            sprintf(aux,"R_%d_%d",r,t);
            f->setAuxBoolvarPref(aux);
            
			//Encode resource constraints using PB constraints
			if(usepb){
				vector<literal> X;
				vector<int> Q;

				for (int i=1;i<=N;i++) {
					int ESi=ins->ES(i);
					int LCi=ins->LC(i,ub);
					if (t>=ESi && t<LCi){
						for (int g=0;g<ins->getNModes(i);g++) {
							int q=ins->getDemand(i,r,g);
							if (q!=0) {
								boolvar x = f->bvar("x",i,t,g);

								X.push_back(x);
								Q.push_back(q);
								if(printpb)
									ss << "+" << q << " x" << x.id << " ";
								if(x.id > bmax.id)
									bmax = x;
							}
						}
					}
				}

				if (!X.empty()){
					util::sortCoefsDecreasing(Q,X);
					f->addPB(Q,X,ins->getCapacity(r),pbenc,&vfl);

					if(printpb){
						ss << "<= " << ins->getCapacity(r) << endl;
					}
				}
			}

			//Encode resource constraints using AMO-PB constraints
			else{

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
						for (int g=0;g<ins->getNModes(i);g++) {
							int auxir=ins->getDemand(i,r,g);
							if (auxir!=0) {
                                //std::cout << " x_" << i << "_" << t << "_" << g;
								boolvar x = f->bvar("x",i,t,g);
								vars_part.push_back(x);
								coefs_part.push_back(auxir);

								if(printamo){
									if(!printed){
										osamo << nconstraints;
										printed=true;
									}
									osamo << " " << x.id;
								}

							}
						}
					}
					if(printed)
						osamo << endl;

					if(!coefs_part.empty()){
						X.push_back(vars_part);
						Q.push_back(coefs_part);
					}
                   // std::cout << std::endl;
				}


				if (!X.empty()){
					util::sortCoefsDecreasing(Q,X);
					f->addAMOPB(Q,X,ins->getCapacity(r),amopbenc,&vfl);
				}
			}
			nconstraints++;
		}
	}


	//Non-renewable resource constraints
	for (int r=N_RR;r<N_R;r++) {
        char aux[50];
        sprintf(aux,"NR_%d",r);
        f->setAuxBoolvarPref(aux);
		//Encode resource constraints using PB constraints
		if(usepb){
			vector<literal> X;
			vector<int> Q;
			for (int j=1;j<=N;j++) {
				for (int g=0;g<ins->getNModes(j);g++) {
					int q=ins->getDemand(j,r,g);
					if (q!=0) {
						boolvar x = f->bvar("sm",j,g);
						X.push_back(x);
						Q.push_back(q);
						if(printpb)
							ss << "+" << q << " x" << x.id << " ";
						if(x.id > bmax.id)
									bmax = x;
					}
				}
			}

			if (!X.empty()){
				util::sortCoefsDecreasing(Q,X);
				f->addPB(Q,X,ins->getCapacity(r),pbenc,&vfl);
				if(printpb){
					ss << "<= " << ins->getCapacity(r)<< endl;
				}
			}
		}

		//Encode resource constraints using AMO-PB constraints
		else{
			vector<vector<literal> > X;
			vector<vector<int> > Q;
			for (int j=1;j<=N;j++) {
				vector<literal> vars_part;
				vector<int> coefs_part;
				bool printed = false;
				for (int g=0;g<ins->getNModes(j);g++) {
					int d=ins->getDemand(j,r,g);
					if (d!=0) {
						boolvar x = f->bvar("sm",j,g);
						vars_part.push_back(x);
						coefs_part.push_back(d);

						if(printamo){
							if(!printed){
								osamo << nconstraints;
								printed=true;
							}
							osamo << " " << x.id;
						}
					}
				}
				if(!coefs_part.empty()){
					X.push_back(vars_part);
					Q.push_back(coefs_part);
				}
				if(printed)
					osamo << endl;
			}

			if (!X.empty()){
				util::sortCoefsDecreasing(Q,X);
				f->addAMOPB(Q,X,ins->getCapacity(r),amopbenc,&vfl);
			}
		}
		nconstraints++;
	}
    f->setDefaultAuxBoolvarPref();

	if(maxsat)
		for(int t=ins->ES(N+1); t <= ins->LS(N+1,ub); t++)
			f->addSoftClause(f->bvar("o",N+1,t));
    
    if(predeforder>0)
        f->setUsePredefDecs(order,predeforder>1);
    
	int nvars=bmax.id;

	if(printpb){
		os.open(pbfilename.c_str());
		if(!os.is_open()){
			cerr << "Error: could not open file: " << pbfilename << endl;
			exit(BADFILE_ERROR);
		}
		os << "* #variable= " << nvars << " #constraint= " << nconstraints << endl;
		os << ss.rdbuf();
		os.close();
	}

	if(printamo){
		osamo.close();
	}

   return f;
}


void Order::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
	int N = ins->getNActivities();

	this->modes=vector<int>(N+2);
   this->starts=vector<int>(N+2);
	this->modes[0] = 0;
	this->modes[N+1] = 0;

	for (int i=0;i<=N+1;i++) {
		for (int p=0;p<ins->getNModes(i);p++){
			if(SMTFormula::getBValue(ef.f->bvar("sm",i,p),bmodel)){
				this->modes[i]=p;
            break;
         }
		}

      int ESi=ins->ES(i);
		int LSi=ins->LS(i,ub);

      for(int t = ESi; t <= LSi; t++){
         if(SMTFormula::getBValue(ef.f->bvar("o",i,t),bmodel)){
            this->starts[i]=t;
            break;
         }
      }
	}


// //Print boolean model
//	for(int i=0; i < N+2; i++){
//		int ESi=ins->ES(i);
//		int LSi=ins->LS(i,ub);
//		cout << i << "\t";
//		for(int t = 0; t <= ub; t++){
//			if(t < ESi || t > LSi)
//				cout << "x";
//			else cout << f->getBValue(f->bvar("o",i,t),bmodel) ? 1 : 0;
//
//		}
//		cout << endl;
//		for(int g = 0; g < ins->getNModes(i); g++){
//			cout << "  " << g << "\t";
//			for(int t = 0; t <= ub; t++){
//				if(t < ESi || t >= ins->LC(i,ub))
//					cout << "x";
//				else cout << f->getBValue(f->bvar("x",i,t,g),bmodel) ? 1 : 0;
//			}
//			cout << endl;
//		}
//		cout << endl;
//	}


}

bool Order::narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub){
	int N = ins->getNActivities();

	if(ub <= lastUB){
		ef.f->addClause(ef.f->bvar("o",N+1,ub));
		for(int i = 1; i <= N; i++)
			for(int t = ins->LC(i,ub); t < ins->LC(i,lastUB); t++)
				for(int g = 0; g < ins->getNModes(i); g++)
					ef.f->addClause(!ef.f->bvar("x",i,t,g));

		return true;
	}
	else return false;
}

Order::~Order() {
}
