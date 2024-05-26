#ifndef MRCPSPENCODING_DEFINITION
#define MRCPSPENCODING_DEFINITION

#include <vector>
#include "mrcpsp.h"
#include "smtformula.h"
#include "encoding.h"


using namespace std;

class MRCPSPEncoding : public Encoding {
private:

protected:

  vector<int> starts;
  vector<int> modes;
  MRCPSP * ins;

public:

	MRCPSPEncoding(MRCPSP * instance);
	int getObjective() const;
	void getModes(vector<int> &modes);
	void getStartsAndModes(vector<int> &starts, vector<int> &modes);
	bool printSolution(ostream & os) const;
	virtual ~MRCPSPEncoding();
};

#endif

