#ifndef ORA_BRANCHANDCUT_H
#define ORA_BRANCHANDCUT_H

#include "Master.h"
#include "Pricing.h"
#include "Solution.h"
#include "Tree.h"


class COL
{
   public:
      void PrintCol(){ printf("%.2f<%s>[%d]\n", this->obj, this->name.c_str(), this->index); };

   public:
      int index;   /* index of a column */
      string name;
      double obj;
      double redcost;
      vector<int> edge;
};

class ROW
{
   public:
      void PrintRow(){ printf("<%s>[%d]\n", this->name.c_str(), this->index); };

   public:
      int index;
      string name;
      double rhs;
      double dualsol;
      // vector<int> 
};

class Conflict
{ 
   public:
      int index;
      double cost;
      vector<int> invertex;        /** vectices in conflict */
};

class BranchAndCut 
{
   public:
      BranchAndCut(MCFDR *mcfdr, const AlgoParameter &parameter);

      ~BranchAndCut() = default;

      void Run();

   private:
      void SolveRootNode(Node &node);
      void Solve(Node &node);
      bool InitialColumns(Node &node);
      bool ColumnGeneration(Node &node, Conflict *conflict);
      bool BigM(Node &node);
      void Branch(Node &parent);
      void IncumbentHeuristic(Node &node);

      inline bool TimeLimit() const
      {
         return timer.elapsed() > parameter.time_limit;
      }

      inline bool Optimal() const
      {
         return globalLb + parameter.optimalityGap > globalUb;
      }

      inline double Gap() const
      {
         return 100.0 * (globalUb - globalLb) / globalLb;
      }

      void Print(Node &node, bool diving = false) const;

      void OutputPerformanceMetrics();

   public:
      MCFDR *mcfdr;
      const AlgoParameter &parameter;
      Master master;
      Pricing pricing;
      Tree tree;
      vector<double> dual;

      /* index, edges, order of edges */
      COL *col; // path
      ROW *row;
      Conflict *conflict;

      Solution incumbent;
      double globalUb;
      double globalLb;

      // statistics
      long nodeCnt;
      int cutsCnt;
      double timeOnHeur;

   private:
      compute::Timer timer;
      double rootUb;
      double rootLb;
};


#endif //ORA_BRANCHANDCUT_H
