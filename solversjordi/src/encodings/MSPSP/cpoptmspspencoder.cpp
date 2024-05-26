//
// Created by jordi on 30/9/20.
//

#include "cpoptmspspencoder.h"
#include <vector>
#include <ilcp/cp.h>

using namespace std;
using namespace smtapi;

CPOPTMSPSPEncoder::CPOPTMSPSPEncoder(Arguments<ProgramArg> * pargs){
    this->pargs = pargs;
}

void CPOPTMSPSPEncoder::solve(MSPSP *instance, int UB) const {

    IloEnv env;

    try {
        IloModel model(env);
        IloInt N = instance->getNActivities();
        IloInt R = instance->getNResources();
        IloInt L = instance->getNSkills();

        //Activity variables
        IloIntervalVarArray activities(env, N+2);

        //Assign variables
        vector<IloIntVarArray> assign(N+2);

        for (int i = 0; i < N+2; i++) {
            activities[i] = IloIntervalVar(env);
            assign[i] = IloIntVarArray(env,R);
            for(int r = 0; r < R; r++) {
                assign[i][r] = IloIntVar(env);
                assign[i][r].setMin(0);
                if (i > 0 && i <= N)
                    assign[i][r].setMax(L);
                else
                    assign[i][r].setMax(0);
            }
        }

        //Correct resource usage
        for(int r = 0; r < R; r++) {
            for(int l = 0; l < L; l++){
                for(int i = 1; i <= N; i++){
                    if(!instance->hasSkill(r,l) || !instance->demandsSkill(i,l)){
                        model.add(assign[i][r]!=l);
                    }
                }
            }
        }

        model.add(IloStartOf(activities[0]) == 0);

        //Set durations, time windows
        for (int i = 0; i < N+2; i++) {
            //Duration
            activities[i].setSizeMin(instance->getDuration(i));
            activities[i].setSizeMax(instance->getDuration(i));

            //Time windows
            if(1 <= i && i <= N) {
                activities[i].setStartMin(instance->ES(i));
                activities[i].setStartMax(instance->LS(i, UB));
            }
            cout << i << " ES " << instance->ES(i) << " LS " << instance->LS(i,UB) << endl;


            //Extended precedences lags
            for (int j = 0; j < N + 2; j++) {
                if (i != j && instance->isPred(i, j)) {
                    IloInt lag = instance->getExtPrec(i, j);
                    model.add(IloStartBeforeStart(env, activities[i], activities[j], lag));
                }
            }
        }

        //Set resource consumptions
        for(int i = 1; i <= N; i++){
            IloIntVarArray demands(env,L+1);
            IloIntArray values(env,L+1);
            int demL = R;
            for(int l = 0; l < L; l++){
                demands[l] = IloIntVar(env);
                demands[l].setMin(instance->getDemand(i,l));
                demands[l].setMax(instance->getDemand(i,l));
                demL-= instance->getDemand(i,l);
                values[l]=l;
            }
            values[L] = L;
            demands[L] = IloIntVar(env,demL,demL);

            //Global cardinality constraint
            model.add(IloDistribute(env,demands,values,assign[i]));
        }



        //Resource constraints
        for(int i = 1; i <= N; i++) for(int j = 1; j <= N; j++) if(i!=j && !instance->inPath(i,j)){
            for(int r = 0; r < R; r++) {
                IloAnd a = assign[i][r] != L && assign[j][r]!=L;
                IloOr o = IloStartOf(activities[i]) >= IloEndOf(activities[j]) || IloStartOf(activities[j]) >= IloEndOf(activities[i]) ;
                model.add(IloIfThen(env,a,o));
            }
        }

        //Implied 1



        //Implied 2



        //Implied 3


        //Objective function
        model.add(IloMinimize(env, IloEndOf(activities[N+1])));

        IloCP cp(model);
        cp.setParameter(IloCP::Workers, 1);
        //cp.setParameter(IloCP::TimeLimit, timeLimit);
        if (cp.solve()) {
            cp.out() << "Makespan \t: " << cp.getObjValue() << std::endl;
            /*for (IloInt ta = 0; ta < nbTasks; ta++) {
                cp.out() << "Tasca : " << ta << " inici : " << cp.getStart(tasks[ta]) << " ";
            }
            cp.out() << endl;*/
        }
        else {
            cp.out() << "No solution found." << std::endl;
        }
    }
    catch (IloException& e) {
        env.out() << " ERROR: " << e << std::endl;
    }
    env.end();


}
