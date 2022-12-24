//
// LP relaxation of subproblem at each node in the branch-and-bound tree
//
// Created by Qian Hu on 2021/11/22.
//

#ifndef ORA_MASTER_H
#define ORA_MASTER_H

// #include "Pdptw.h"
// #include "Route.h"
#include <ilcplex/ilocplex.h>
#include "AlgoParameter.h"
#include "Node.h"
#include "Mcfdr.h"


class Master {
public:
    Master(MCFDR* mcfdr, const AlgoParameter &parameter);

    ~Master() {
        env.end();
    }

    void set(const Node &node);
    bool Solve();
    void GetDualValues(std::vector<double> &dual);
    void AddCol(std::vector<Path> &routes);
    void addSlackToCardinality();
    void GetSol(Node &node);

    void delSlack() {
        model.remove(slack);
    }

    double getSlackValue() {
        return cplex.getValue(slack);
    }

    void writeModel(const std::string &name = "master.lp") {
        cplex.exportModel(name.c_str());
    }

   //  int numCol() const {
      //   return routePool.size();
   //  }

public:
    long numLp;
    double time;
private:
   MCFDR* mcfdr;
   const AlgoParameter &param;
   // RoutePool routePool;
   // huq::Timer timer;

   IloEnv env;
   IloModel model;
   IloCplex cplex;
   IloObjective obj;
   IloRangeArray rng;
   IloRange *ptrNumVehicle;
   IloNumVarArray y;
   IloNumVar slack;
};


#endif //ORA_MASTER_H
