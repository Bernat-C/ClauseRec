#include "auctionencoding.h"
#include "util.h"
#include "errors.h"
#include <limits.h>

BACPEncoding::BACPEncoding(BACP * instance, AMOPBEncoding amopbenc) : Encoding()  {
    this->instance = instance;
    this->amopbenc = amopbenc;
}

BACPEncoding::~BACPEncoding() {
    
}

SMTFormula * BACPEncoding::encode(int lb, int ub){
    
    SMTFormula * f = new SMTFormula();
    
    for(int c = 0; c < instance->getNCourses(); c++){
        for(int p=0; p<instance->getNPeriods(); p++){
            f->newBoolVar("o",c,p);
            f->newBoolVar("x",c,p);
        }
        
        //Order encoding
        for(int p = 1; p < instance->getNPeriods(); p++){
            f->addClause(!f->bvar("o",c,p-1) | f->bvar("o",c,p));
        }
        
        //Last period
        f->addClause(f->bvar("o",c,instance->getNPeriods()-1));
        
        //Chanelling
        f->addClause(!f->bvar("o",c,0) | f->bvar("x",c,0));
        f->addClause(f->bvar("o",c,0) | !f->bvar("x",c,0));
        for(int p = 1; p < instance->getNPeriods(); p++){
            f->addClause(!f->bvar("o",c,p) | f->bvar("o",c,p-1) | f->bvar("x",c,0));
            f->addClause(f->bvar("o",c,p) | !f->bvar("x",c,p));
            f->addClause(!f->bvar("o",c,p-1) | !f->bvar("x",c,p));
        }
    }
    
    //Precedences
    for(const pair<int,int> & pre : instance->getPrerequirements()){
        f->addClause(!f->bvar("o",pre.second,0));
        for(int p = 1; p < instance->getNPeriods; p++)
            f->addClause(!f->bvar("o",pre.second,p) | f->bvar("o",pre.first,p-1));
    }
    
    
    //Maximum load
    
    
    //Minimum load
    

    //Maximum courses
    
    
    //Minimum courses
    
    
    
    
    
    
    
    vector<literal> x(instance->getNBids());
    
    // boolvar x = f->newBoolVar("x",1,3,4);  variable x_1,3,4
    //f->bvar("x",1,3,4);
    
    //f->addClause(x);
    //f->addClause(!x);
    //f->addClause(l);
    //f->addClause(x | !l);
    
    //vector<literal> v;
    //f->addAMO(v);
    
    //literal l = x;
    //l = !x;
    //l = !l;
    
    for(int i = 0; i < instance->getNBids(); i++)
        x[i]=f->newBoolVar("x",i);
    
    for(int i = 0; i < instance->getNBids()-1; i++)
        for(int item : instance->getBid(i))
            for(int j = i+1; j < instance->getNBids(); j++)
                if(instance->demandsItem(j,item))
                    f->addClause(!x[i] | !x[j]);
    
    vector<vector<int> > cover;
    instance->computeBidCover(cover);
    
    vector<vector<literal> > X(cover.size());
    vector<vector<int> > Q(cover.size());
    
    for(int i = 0; i < cover.size(); i++){
        for(int j : cover[i]){
            Q[i].push_back(instance->getBidValue(j));
            X[i].push_back(x[j]);
        }
    }
    
    //(3x1+ 4y) + 2x2 +7x3 <= LB
    //Q = [[3,4], [2], [7]]
    //X= [[x1,y], [x2], [x3]]
    
    //f->addAMOPB(Q,X,lb,amopbenc);
    
    f->addAMOPBGEQ(Q,X,lb,amopbenc);
    
    return f;
}


void BACPEncoding::setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel){
    this->period.resize(instance->getNCourses());
    for(int c=0; c<instance->getNCourses(); c++){
        for(int p=0; p<instance->getNPeriods(); p++){
            if(SMTFormula::getBValue(ef.f->bvar("o",c,p),bmodel){
                period[c]=p;
                break;
            }
        }
    }
}


bool BACPEncoding::printSolution(ostream & os) const {
    for(int c=0; c<instance->getNCourses(); c++)
        os << " :" << period[c];
    os << endl;
    return true;
}

