#include "Pricing.h"

Pricing::Pricing(MCFDR* _mcfdr) 
{
   mcfdr = _mcfdr;
   ptrNode = NULL;   
}

void Pricing::Set(Node &node) {
    ptrNode = &node;
}

/**
 * @brief Solve the subproblems of commodity k 
 * @param : dual values, conflict information
 * @return column index 
 */
void Pricing::Solve(vector<double> &dual) 
{
   for(int k = 0; k < mcfdr->count->ncommodities; k++)
   {  
      Path path;
      ShortestPath(k, path);
      paths.push_back(path);
   }
}