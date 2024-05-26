#ifndef MRCPSPMAXRELAXEDENCODING_DEFINITION
#define MRCPSPMAXRELAXEDENCODING_DEFINITION

#include <vector>
#include "mrcpspmax.h"
#include "mrcpspmaxencoding.h"
#include "smtformula.h"
#include "encoding.h"


using namespace std;
using namespace smtapi;

class MRCPSPMAXRelaxedEncoding : public MRCPSPMAXEncoding {
private:

protected:

public:

	MRCPSPMAXRelaxedEncoding(MRCPSPMAX * instance, AMOPBEncoding amopbenc);
	~MRCPSPMAXRelaxedEncoding();

	SMTFormula * encode(int lb = INT_MIN, int ub = INT_MAX);
	bool narrowBounds(const EncodedFormula & ef, int lastLB, int lastUB, int lb, int ub);


};

#endif

