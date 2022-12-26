// LP relaxation of subproblem at each node in the branch-and-bound tree

#ifndef _MASTER_H_
#define _MASTER_H_

#include <ilcplex/ilocplex.h>
#include "Mcfdr.h"
#include "AlgoParameter.h"
#include "Node.h"


class Master {
   public:
      Master();
      Master(MCFDR* mcfdr, const AlgoParameter &param);

      ~Master() 
      {
        env.end();
      }

      void Set(const Node &node);
      bool Solve();
      void GetDualValues(std::vector<double> &dual);
      void AddCol(std::vector<Path> &routes);
      void AddSlackToCardinality();
      void GetSol(Node &node);

      void DelSlack()
      {
         model.remove(slack);
      }

      double GetSlackValue()
      {
         return cplex.getValue(slack);
      }

      void writeModel(const std::string &name = "master.lp")
      {
         cplex.exportModel(name.c_str());
      }

       int numCol() const {
        return pathpool.size();
       }

   public:
      MCFDR *mcfdr;
      const AlgoParameter &param;
      PathPool pathpool;
      long numLp;
      double time;

   // cplex 类
   private:
      IloEnv env;
      IloModel model;
      IloCplex cplex;
      IloObjective obj;
      IloRangeArray rng;
      IloRange *ptrNumVehicle;
      IloNumVarArray y;
      IloNumVar slack;
};

#endif //_MASTER_H_
