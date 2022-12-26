#ifndef ORA_SOLUTION_H
#define ORA_SOLUTION_H

#include "Path.h"

class Solution : public vector<Path> 
{
   public:
      int cost;
   public:
      Solution() : cost(0) {

    }

      void calculate() 
      {
         cost = 0;
         // for (Path &path: *this)
            // cost += path.cost;
      }

};

#endif //ORA_SOLUTION_H
