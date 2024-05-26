#ifndef TESTENCODING_DEFINITION
#define TESTENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "encoding.h"

using namespace std;

class TestEncoding : public Encoding {

private:

	vector<vector<boolvar> > x;
	vector<vector<bool> > v;

public:
	TestEncoding(string filename);
	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	bool printSolution(ostream & os) const;


	~TestEncoding();

};

#endif

