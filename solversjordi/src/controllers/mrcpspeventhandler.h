#ifndef MRCPSPEVENTHANDLER_DEFINITION
#define MRCPSPEVENTHANDLER_DEFINITION

#include "basiceventhandler.h"
#include "encoder.h"
#include "mrcpspencoding.h"
#include "solvingarguments.h"
#include "mrcpsp.h"

class MRCPSPEventHandler : public BasicEventHandler{

private:


	MRCPSP * instance;


public:

	MRCPSPEventHandler(MRCPSPEncoding * encoding, SolvingArguments * sargs, MRCPSP * instance);

	~MRCPSPEventHandler();

	void afterSatisfiabilityCall(int lb, int ub, Encoder * encoder);

};

#endif
