#include "Mcfdr.h"
#include "print.h"

MCFDR::MCFDR(Vertex* vertex_Set, Edge* edge_Set, Com* coms_Set, Layer* layer_Set, Count* count_Set)
{
   vertex = vertex_Set;
   edge = edge_Set;
   com = coms_Set;
   count = count_Set;
}

void UniqueElement(vector<Vertex> &Set)
{
   vector<Vertex>::iterator vertexIt;
   sort(Set.begin(), Set.end());
   vertexIt = unique(Set.begin(), Set.end());
   if(vertexIt != Set.end())
      Set.erase(vertexIt, Set.end());
}

double MCFDR::Distance(Vertex v1, Vertex v2)
{
   assert(v1.z == v2.z);
   double disx = abs( v1.x - v2.x );
   double disy = abs( v1.y - v2.y );
   return (disx - disy > EPS) ? (disx * layer[v1.z].length) : (disy * layer[v1.z].width);
}

/**
 * @brief store all the data about nets(commodities)
 * @details a net includes its source sets and target sets.
 * 
 * @return int 
 */
static int ReadNet(
   string              file, 
   Layer**             layer,
   Com**               coms,
   Count**             count
)
{
   FILE*        infile     = NULL;
   Vertex       point;
   char         line[BUFSIZE];
   char         ch;
   int          nrows      = 0;
   int          demand;
   int          commodity;
   int          nvertexs;
   int          nedges;
   int          nlayers;
   int          ncommodities;
   int          l          = 1;
   int          m          = 0;
   int          n;

   unordered_map<int, int> hash_com;
   unordered_map<int, int>::iterator iter;

   infile = fopen(file.c_str(), "r");
   if(infile == NULL)
      return -1;

   while(!feof(infile))
   {
      ch = fgetc(infile);
      if(ch == '\n')
         nrows++;
   }
 
   *layer = new Layer[nrows];  /* need to delete */
   *coms = new Com[nrows]; 
   *count = new Count;

   /* Read data of nets from net file */
   rewind(infile);
   for( n = 0; n < nrows; n++ )
   {
      fgets(line, sizeof(line), infile); 
      if( n < 1 )
      {
         /* the first line represents the total amount of different data */
         sscanf(line, "%d %d %d %d", &nlayers, &ncommodities, &nvertexs, &nedges);
      }
      else if( n >= 1 && n <= nlayers )
      {
         /* get the information of layers, including the index, the total length and the total width */
         sscanf(line, "%d %d %d", &(*layer)[l].num, &(*layer)[l].length, &(*layer)[l].width);
         l++;
      }
      else
      {
         /* get the information of nets(commodities), including the index of net, origin nodes and sink nodes  */
         sscanf(line, "%d %d %d %d %d", &commodity, &point.x, &point.y, &point.z, &demand);
         // ST.push_back(point);
         iter = hash_com.find(commodity);
         if (iter == hash_com.end())
         {
            Com com;
            com.commodity = commodity;
            hash_com[commodity] = m;
            if (demand == 1)
               com.S.push_back(point);
            else if (demand == -1)
               com.T.push_back(point);
            (*coms)[m] = com;
            m++;
         }
         else
         {
            if (demand == 1)
               (*coms)[iter->second].S.push_back(point);
            else if (demand == -1)
               (*coms)[iter->second].T.push_back(point);
         }
      }
   }
   (*count)->nvertexs     = nvertexs;
   (*count)->nedges       = nedges;
   (*count)->nlayers      = nlayers;
   (*count)->ncommodities = ncommodities;

   info::print_tab(debuginfo, "nlayers:", nlayers, "nnets:", ncommodities, "nvertexs:", nvertexs, "nedges:", nedges);
#if 1
   printf("Net info:");
   for(int ll = 0; ll < l; ll++)
      printf("%d %d %d\n", (*layer)[ll].num, (*layer)[ll].length, (*layer)[ll].width);
   for(int mm = 0; mm < m; mm++)
   {
      for(int s = 0; s < (int)(*coms)[mm].S.size(); s++)
         printf("%d %d %d %d\n", (*coms)[mm].commodity, (*coms)[mm].S[s].x, (*coms)[mm].S[s].y, (*coms)[mm].S[s].z);
      for(int t = 0; t < (int)(*coms)[mm].T.size(); t++)
         printf("%d %d %d %d\n", (*coms)[mm].commodity, (*coms)[mm].T[t].x, (*coms)[mm].T[t].y, (*coms)[mm].T[t].z);
   }
#endif
   fclose(infile);
   return 0;
}

static int ReadEdge(
   string              file, 
   Vertex**            vertex, 
   Edge**              edge
)
{
   FILE*        infile     = NULL;
   vector<Vertex> vertexSet;
   char         line[BUFSIZE];
   char         ch; 
   int          nrows      = 0;
   int          i          = 0;
   int          n;

   infile = fopen(file.c_str(), "r");
   if(infile == NULL)
      return -1;

   while(!feof(infile))
   {
      ch = fgetc(infile);
      if(ch == '\n')
         nrows++;
   }

   *vertex = new Vertex[nrows];
   *edge = new Edge[nrows];

   rewind(infile);
   for (n = 0; n < nrows; n++)
   {
      fgets(line, sizeof(line), infile); 
      sscanf(line, "%d %d %d %d %d %d %d %lf", &(*edge)[n].commodity, &(*edge)[n].tail.x, &(*edge)[n].tail.y, &(*edge)[n].tail.z, &(*edge)[n].head.x, &(*edge)[n].head.y, &(*edge)[n].head.z, &(*edge)[n].cost);
      (*edge)[n].index = n;
      vertexSet.push_back((*edge)[n].tail);
   }

   UniqueElement(vertexSet);  //remove duplicate elements
   sort(vertexSet.begin(), vertexSet.end());
   for( i = 0; i < (int)vertexSet.size(); i++ )
   {
      (*vertex)[i].index = i;
      (*vertex)[i].x = vertexSet[i].x;
      (*vertex)[i].y = vertexSet[i].y;
      (*vertex)[i].z = vertexSet[i].z;
      
   }

   info::print_tab("edges:", n, "vertexs:", vertexSet.size(), i);
#if 1
   for( int v = 0; v < (int)vertexSet.size(); v++ )
      printf("%d %d %d %d\n", (*vertex)[v].index, (*vertex)[v].x, (*vertex)[v].y, (*vertex)[v].z);
   for( int e = 0; e < n; e++ )
      printf("%d %d %d %d %d %d %d %.2f\n", (*edge)[e].commodity, (*edge)[e].tail.x, (*edge)[e].tail.y, (*edge)[e].tail.z, (*edge)[e].head.x, (*edge)[e].head.y, (*edge)[e].head.z, (*edge)[e].cost);
#endif
   fclose(infile);
   return 0;
}


bool MCFDRReadData(
   MCFDR*              mcfdr,
   string              demandfile,
   string              edgefile
)
{
   info::print(debuginfo, "enter MCFDRReadData");
   Layer* layer;
   Vertex* vertex;
   Edge* edge;
   Com* coms;
   Count* count;
   int status;

   status = ReadNet(demandfile, &layer, &coms, &count);
   status = ReadEdge(edgefile, &vertex, &edge);
   info::print_tab(status, count->ncommodities, count->nvertexs, count->nedges, count->nlayers);
   mcfdr = new MCFDR(vertex, edge, coms, layer, count);
   
   delete[] layer;
   delete[] vertex;
   delete[] edge;
   delete[] coms;
   delete[] count;

   return status;
}
