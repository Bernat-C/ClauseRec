#ifndef CLINICSENCODING_DEFINITION
#define CLINICSENCODING_DEFINITION


#include <vector>

#include <sstream>
#include <stdio.h>
#include <iostream>
#include "encoding.h"
#include "clinics.h"

using namespace std;

class ClinicsEncoding : public Encoding {

protected:

	Clinics * ins;
	vector<int> starts;
	string filename;

public:
	ClinicsEncoding(Clinics * instance);
	void setModel(const EncodedFormula & ef, int lb, int ub, const vector<bool> & bmodel, const vector<int> & imodel);
	int getObjective() const;
	bool printSolution(ostream &os) const;
	void setPrintCharts(const string & filename);
	void makeChart(const string & path) const;
	vector<int> & getStarts();

	~ClinicsEncoding();
};

#endif

