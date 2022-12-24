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
   const AlgoParameter &parameter
)
{
   mcfdr = _mcfdr;
}

void BranchAndCut::run() {
   /* todo: create the root node*/
   Node root;
   root.status = NodeStatus::Unsolved; /* the initial status of root is unsolved */
   root.lpsol = NULL;
   root.lb = 0;
   
   SolveRootNode(root);

   if( root.status == NodeStatus::Fractional )
   {
      // Branch();
   }

}

/**
 * @brief Solve root node 
 * @param node
 * @return node status
 */
void BranchAndCut::SolveRootNode(Node &node)
{

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
   pricing.Solve(/*no dual variables*/);
   master.AddCol(pricing.paths);
}


bool BranchAndCut::ColumnGeneration(
   Node          &node,
   Conflict*      conflict
) 
{
   if( !master.Solve() )
      return false;
   
   vector<double> dual;

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
      
      // pricing.Solve(mcfdr, conflict, dual);
   
   }

   

   return true;
}

