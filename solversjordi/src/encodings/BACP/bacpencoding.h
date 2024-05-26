#ifndef BACPENCODING_DEFINITION
#define BACPENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "bacp.h"
#include "auction.h"

using namespace std;
using namespace smtapi;

class BACPEncoding : public Encoding {
    
private:
    
    BACP * instance;
    AMOPBEncoding amopbenc;
    
    vector<int> period;
    
public:
    
    BACPEncoding(BACP * instance, AMOPBEncoding amopbenc);
    ~BACPEncoding();
    
    SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);
    
    void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
    
    bool printSolution(ostream &os) const;
    
};

#endif

