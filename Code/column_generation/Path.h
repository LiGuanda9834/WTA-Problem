#ifndef ORA_PATH_H
#define ORA_PATH_H

#include <vector>
#include "Mcfdr.h"

class Path : public vector<int> {
   public:
      int cost; // reserved keyword
      int index;   /* index of a column */
      string name;
      double obj;
      double redcost;
      vector<int> vertexIdx;
      vector<int> edge;

   public:
      Path() = default;
      ~Path() = default;

      inline bool emptyPath() const
      {
         return size() < 1;
      }

      void PrintCol(){ printf("%.2f<%s>[%d]\n", this->obj, this->name, this->index); };
    

};

class PathPool : public vector<Path> {

};

#endif //ORA_ROUTE_H
