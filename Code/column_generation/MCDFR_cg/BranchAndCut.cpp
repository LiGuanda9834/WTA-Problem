/**
 * @file BranchAndCut.cpp
 * @author Zhou Xiao 
 * @brief 
 * @date 2022-11-07
 * @copyright Copyright (c) 2022
 */

#include <cassert>
#include "BranchAndCut.h"
#include "print.h"


BranchAndCut::BranchAndCut(
   MCFDR* _mcfdr,
   const AlgoParameter &_parameter
) : mcfdr(_mcfdr), parameter(_parameter), master(mcfdr, parameter), pricing(mcfdr), tree(parameter.depthFirst)
{
   globalLb = -parameter.maxValue;
   globalUb = parameter.maxValue;
   nodeCnt = 0;
   cutsCnt = 0;
   timeOnHeur = 0.0;
}

// 执行函数
void BranchAndCut::Run() {
   /* todo: create the root node*/
   Node root;
   root.status = NodeStatus::Unsolved; /* the initial status of root is unsolved */
   root.lpsol = NULL;
   root.lb = 0;
   
   SolveRootNode(root);

   /* solve the leaves of branch and bound tree */
   if( !parameter.rootOnly )
   {
      while( !tree.empty() && !TimeLimit() && !Optimal() )
      {
         Node node = tree.next();
         tree.pop();
         Solve(node);
         if( node.status == NodeStatus::Integral || node.status == NodeStatus::Fractional )
         {
            globalLb = tree.updateGlobalLb();
         }
         if( node.status == NodeStatus::Fractional )
         {
            Branch(node);
         }
      }
      if( tree.empty() )
      {
         globalLb = globalUb;
      }
   }
   // 输出信息函数
   OutputPerformanceMetrics();
}

/**
 * @brief Solve root node 
 * @param node
 * @return node status
 */
void BranchAndCut::SolveRootNode(Node &node)
{
   info::print(debuginfo, "enter SolveRootNode");
   bool feasible = InitialColumns(node); 
   //infeasible: 
   if( feasible )
   {
      /* if root is feasible, then get the optimal solution of LP relaxtion of root node */
      ColumnGeneration(node, conflict);
      master.GetSol(node);
      node.lb = node.lpsol->obj;
   }
   else //infeasible
   {
      /* otherwise, if the root node is infeasible, then ... */
   }

   if( node.status == NodeStatus::Fractional )
   {
      // Branch();
   }

}

void BranchAndCut::Solve(Node &node)
{

}

/**
 * @brief master lp initial columns
 * @param : root node 
 * @return index of columns
 * @todo add cols, add rows((5b) and part of conflict cons[lazy constraint])
 */
bool BranchAndCut::InitialColumns(
   Node          &node
)
{  
   // pricing.Solve(/*no dual variables*/);
   master.AddCol(pricing.paths);
   return true;
}


bool BranchAndCut::ColumnGeneration(
   Node          &node,
   Conflict*      conflict
) 
{
   if( !master.Solve() )
      return false;

   master.GetDualValues(dual);
   // pricing.Solve(mcfdr, conflict, dual);

   /* add column(s) gradually to master problem */
   while (!pricing.paths.empty())  /* while routes empty, stop*/
   {
      /* Add path(s) from the candidate pool */
      master.AddCol(pricing.paths);

      master.Solve();

      /* Get the value of dual variables of master problem */
      master.GetDualValues(dual);
      
      pricing.Solve(dual);
   
   }

   return true;
}

bool BranchAndCut::BigM(Node &node)
{
   master.AddSlackToCardinality();
   bool feasible = master.Solve();
   if( !feasible )
   {}
   else
   {
      double slackValue = master.GetSlackValue();
      master.GetDualValues(dual);
      pricing.Solve(dual);
      while( !pricing.paths.empty() && slackValue > parameter.eps )
      {
         master.AddCol(pricing.paths);
         master.Solve();
         slackValue = master.GetSlackValue();
         master.GetDualValues(dual);
         pricing.Solve(dual);
      }
   }
   master.DelSlack();
   return feasible;
}

void BranchAndCut::Branch(Node &parent)
{
   if( parameter.enableBranchOnSum )
   {
      double total = parent.lpsol->accumulate();
      if( !compute::integral(total, parameter.eps) )
      {
         int floor = int(std::floor(total));
         // tree.emplace(NULL, ++nodeCnt, true, )
      }
      
   }
}

void BranchAndCut::Print(Node &node, bool diving) const 
{
   string tag = diving ? "dive" : "Node";
   info::print_tab(tag, tree.size(), nodeCnt, node.statusStr(), node.lb, globalLb, globalUb, Gap(),
                     timer.elapsed(), master.time, master.numLp, master.numCol());
}

void BranchAndCut::OutputPerformanceMetrics() {
   // cout << "Porblem Name         : " << mcfdr->probname << endl;
   cout << "Parameter Infomation : " << endl;
   cout << "   Algorithm Name    : " << parameter.algo_name << endl;
   cout << "   Time Limit        : " << parameter.time_limit << endl;
   cout << "   Is Root Only      : " << parameter.rootOnly << endl;
   cout << "   Is Enable Cuts    : " << parameter.enableCuts << endl;
   cout << "Root Gap(%)          : " << 100 * (rootUb - rootLb) / rootLb << endl;
   // cout << "Gap Closed(%)        : " << 100 * (rootLb - FirstLb) / (objValue - FirstLb) << endl;
   cout << "Global Gap(%)        : " << Gap() << endl;
   cout << "Time                 : " << timer.elapsed() << endl;
   cout << "Time On Master       : " << master.time << endl;
   cout << "Node Count           : " << nodeCnt - tree.size() << endl;
   cout << "Number of Columns In Master : " << master.numCol() << endl;
   cout << "Current Date Time    : " << timer.getCurrentDateTime() << endl;
}