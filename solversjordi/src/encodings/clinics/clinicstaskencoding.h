#ifndef CLINICSTASKENCODING_DEFINITION
#define CLINICSTASKENCODING_DEFINITION

#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "clinicsencoding.h"
#include "mmkp.h"

using namespace std;

class ClinicsTaskEncoding : public ClinicsEncoding {

private:

public:
	ClinicsTaskEncoding(Clinics * instance);
	SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);
	bool narrowBounds(const EncodedFormula &f, int lastLB, int lastUB, int lb, int ub);
	~ClinicsTaskEncoding();

	//const vector<int> & getAssignment();
};

#endif

