#ifndef CLINICSTIMEENCODING_DEFINITION
#define CLINICSTIMEENCODING_DEFINITION

#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "clinicsencoding.h"
#include "mmkp.h"

using namespace std;

class ClinicsTimeEncoding : public ClinicsEncoding {

private:

public:
	ClinicsTimeEncoding(Clinics * instance);
	SMTFormula *  encode(int lb = INT_MIN, int ub = INT_MAX);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);
	~ClinicsTimeEncoding();

};

#endif

