//
// Created by jordi on 30/9/20.
//

#ifndef CPOPTMSPSPENCODER_H
#define CPOPTMSPSPENCODER_H

#include "mspsp.h"
#include "mspspencoding.h"
#include "solvingarguments.h"
#include "arguments.h"


using namespace std;
using namespace smtapi;

class CPOPTMSPSPEncoder {

private:
    Arguments<ProgramArg> * pargs;

public:

    CPOPTMSPSPEncoder(Arguments<ProgramArg> * pargs);

    void solve(MSPSP * instance, int UB) const;


};


#endif //SOLVERSJORDI_COPOPTENCODER_H
