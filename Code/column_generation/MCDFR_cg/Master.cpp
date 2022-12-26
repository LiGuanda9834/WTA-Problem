#include "Master.h"

Master::Master(
   MCFDR* _mcfdr, 
   const AlgoParameter &_param
) : mcfdr(_mcfdr), param(_param), model(env), cplex(model)
{
   numLp = 0;
   time = 0;
   if (!param.enableCplexLog)
   {
      cplex.setOut(env.getNullStream());
      cplex.setWarning(env.getNullStream());
   }
   cplex.setParam(IloCplex::Param::RandomSeed, param.seed);
   cplex.setParam(IloCplex::Param::TimeLimit, param.time_limit);
   cplex.setParam(IloCplex::Param::Threads, 1);

   y = IloNumVarArray(env);
   obj = IloMinimize(env);
   model.add(obj);

   rng = IloRangeArray(env, mcfdr->count->nedges + 1, 1, param.maxValue);
   model.add(rng);
   
   if (param.enableBranchOnSum)
      ptrNumVehicle = &rng[mcfdr->count->nedges];
}

void Master::Set(const Node &node) 
{
}

bool Master::Solve() 
{
   numLp++;
   bool result = cplex.solve();

   return result;
}

void Master::GetDualValues(vector<double> &dual) 
{
}

void Master::AddSlackToCardinality()
{
   IloNumColumn column(env);
   column += obj(1 << 30);
   column += (*ptrNumVehicle)(-1); // todo: check
   slack = IloNumVar(column, 0, param.maxValue, ILOFLOAT, "slack");
}

void Master::AddCol(vector<Path> &paths) 
{
   for( Path &path: paths )
   {
      auto pathId = pathpool.size();
      IloNumColumn column(env);  
      column += obj(path.cost);
      for( int i = 1; i < path.size(); i++ )
      {
         int p = path[i];
         // column +=   
      }
      y.add(IloNumVar(column, 0, param.maxValue, ILOFLOAT, ("y"+std::to_string(pathId)).c_str()));
   }
}

void Master::GetSol(Node &node) 
{
   node.lpsol = new LpSol();
   node.lpsol->obj = cplex.getObjValue();
   for( int i = 0; i < pathpool.size(); i++ )
   {
      double val = cplex.getValue(y[i]);
      if( val > param.eps )
      {
         node.lpsol->push_back(std::make_pair(&pathpool[i], val));
      }
   }
}