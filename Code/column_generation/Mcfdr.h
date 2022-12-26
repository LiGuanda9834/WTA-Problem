#ifndef _MCFDR_H__
#define _MCFDR_H__

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iterator>
#include <vector>
#include <algorithm>
#include <time.h>
#include <cassert>
#include <functional>
#include <unordered_map>
using std::vector;
using std::string;
using std::unordered_map;

/* Problem data */
#define EPS     1e-6
#define BUFSIZE 1000
#define alpha   140
#define beta    100
#define gamma   120
#define viodist alpha+gamma


#ifdef __cplusplus
extern "C" {
#endif

bool MCFDRReadData(
   MCFDR*              mcfdr,
   string              demandfile,
   string              edgefile
);

/**
 * @brief Detailed Routed Vertex
 * @details vertex on a 3D-grid graph
 */
class Vertex  
{
   public:
      int index;           /** index of the node*/
      int x;               /** x-coordinate of a vertex */
      int y;               /** y-coordinate of a vertex */
      int z;               /** z-coordinate of a vertex */

      Edge* relateEdge;    /** incidence edges of a vertex */

      /* reload operator of "==" and "<" */
      bool operator==(const Vertex ver)
      {
         if(this->x == ver.x && this->y == ver.y && this->z == ver.z )
            return true;
         return false;
      }
      bool operator<(const Vertex ver)
      {
         if(this->x < ver.x)
            return true;
         else if(this->x > ver.x)
            return false;
         else
         {
            if(this->y < ver.y)
               return true;
            else if(this->y > ver.y)
               return false;
            else
               return this->z < ver.z;
         }
      }
};

class Edge
{
   public:
      int index;                /** index of edge */
      int commodity;            /** the commodity on edge*/
      Vertex tail;              /** the tail of edge*/
      Vertex head;              /** the head of edge*/
      double cost;              /** the cost of edge*/

      Vertex* adjVertex;        /** adjecent nodes of edge */

      bool operator==(const Edge & e)
      {
         return (this->commodity == e.commodity && this->tail == e.tail && this->head == e.head);
      }
};

class Com
{
   public:
      int commodity;
      vector<Vertex> S;
      vector<Vertex> T;
};

class Layer
{
   public:
      int num;
      int length;
      int width;
};

class Count
{
   public:
      int nvertexs;
      int nedges;
      int ncommodities;
      int nlayers;
};

class MCFDR
{
   public:
      MCFDR(Vertex* vertex_Set, Edge* edge_Set, Com* coms_Set, Layer* layer_Set, Count* count_Set);
      ~MCFDR() = default;

   public:
      Vertex*          vertex;
      Edge*            edge;
      Com*             com;
      Layer*           layer;
      Count*           count;
      vector<Vertex>   ST;                     /**> Source sets and target sets */
      
      unordered_map<string, int>           varId;
      unordered_map<string, int>::iterator varIdIter;
      unordered_map<int,string>            varName;
      unordered_map<int,string>::iterator  varNameIter;
      
      double Distance(Vertex v1, Vertex v2);
      bool FindConflict(vector<Vertex> *conflicts);
};

/* create a mcfdr 3D-graph */
bool CreateGraph()
{

};

/* free a mcfdr 3D-graph */
bool FreeGraph()
{

};

#ifdef __cplusplus
}
#endif

#endif
