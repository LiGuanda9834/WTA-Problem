// Created by Qian Hu on 2022/1/6 下午8:54.
// Copyright (c) 2022 NJU ORA Team. All rights reserved.

#ifndef ORA_BRANCHANDCUT_H
#define ORA_BRANCHANDCUT_H

#include "Node.h"
#include "Master.h"
#include "Mcfdr.h"
#include "AlgoParameter.h"
#include "Pricing.h"
#include "Solution.h"


class COL
{
   public:
      void PrintCol(){ printf("%.2f<%s>[%d]\n", this->obj, this->name, this->index); };

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
      void PrintRow(){ printf("<%s>[%d]\n", this->name, this->index); };

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

class BranchAndCut {
public:
   BranchAndCut(MCFDR* mcfdr, const AlgoParameter &parameter);

   ~BranchAndCut() = default;

   void run();

private:
   void SolveRootNode(Node &node);
   void solve(Node &node);
   bool InitialColumns(Node &node);
   bool ColumnGeneration(Node &node, Conflict* conflict);
   bool bigM(Node &node);
   void branch(Node &parent);
   void incumbentHeuristic(Node &node);

   inline bool timeout() const
   {
      // return timer.elapsed() > parameter.time_limit;
   }

   inline bool optimal() const
   {
      return globalLb + parameter.optimalityGap > globalUb;
   }

   inline double gap() const
   {
      return 100.0 * (globalUb - globalLb) / globalLb;
   }

   void print(Node &node, bool diving = false) const;

   void outputPerformanceMetrics();

public:
   MCFDR* mcfdr;
   const AlgoParameter &parameter;
   Master master;
   Pricing pricing;
   
   /* index, edges, order of edges */
   COL* col; //path
   ROW* row;
   Conflict* conflict;

   Solution incumbent;
   double globalUb;
   double globalLb;

   // statistics
   long nodeCnt;
   int nCuts;
   double timeOnHeuristic;

private:

   double rootUb;
   double rootLb;
};


#endif //ORA_BRANCHANDCUT_H
