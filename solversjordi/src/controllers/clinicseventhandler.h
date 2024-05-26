#ifndef CLINICSEVENTHANDLER_DEFINITION
#define CLINICSEVENTHANDLER_DEFINITION

#include "basiceventhandler.h"
#include "encoder.h"
#include "solvingarguments.h"
#include "clinics.h"
#include "clinicsencoding.h"

class ClinicsEventHandler : public BasicEventHandler{

private:

	Clinics * ins;
	ClinicsEncoding * encoding;
	bool active;

public:

	ClinicsEventHandler(Clinics * instance, ClinicsEncoding * encoding, SolvingArguments * sargs, bool active);

	~ClinicsEventHandler();

	virtual void onSATSolutionFound(int & lb, int & ub, int & obj_val);

};

#endif
