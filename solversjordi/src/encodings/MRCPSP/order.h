#ifndef ORDER_DEFINITION
#define ORDER_DEFINITION


#include <vector>
#include <set>
#include <string>
#include <map>
#include <sstream>
#include <stdio.h>
#include <iostream>
#include "mrcpspencoding.h"
#include <stdlib.h>

using namespace std;

class Order : public MRCPSPEncoding {

private:
	bool maxsat;
    int predeforder;
	AMOPBEncoding amopbenc;
	bool usepb;
	PBEncoding pbenc;

	bool printpb;
	string pbfilename;

	bool printamo;
	string amofilename;

public:

	Order(MRCPSP * instance,  AMOPBEncoding amopbenc, int predeforder, bool maxsat = false);
	~Order();

	SMTFormula * encode(int LB = INT_MIN, int UB = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);

	void setPrintPB(const string & filename);
	void setPrintAMO(const string & filename);
	void setUsePB(PBEncoding pbenc);
};

#endif

