#include "BranchAndCut.h"
#include "Mcfdr.h"
#include "print.h"

int main(int argc, char *argv[]) {
   info::print(true, "AAAA");
   MCFDR* mcfdr = NULL;
   // 保存数据
   string DemandFile = argv[1];
   string EdgeFile   = argv[2];
   MCFDRReadData(mcfdr, DemandFile, EdgeFile);
   //设置参数
   AlgoParameter param(
      "MCFDR",            // problem name
      "BranchAndPrice",   // algorithm Name
      3600,               // time_limit = 1800 seconds
      "AA", "",           // teston_prefix, teston_extension
      "./src/mcfdr/data", // path_data
      true,               // rootOnly
      false,              // enableCuts
      false,              // depthFirst if true, otherwise it is primalBoundFirst
      true,               // branchOnSum
      false,              // enableCplexLog
      true                // debug
   );

   // 列生成
   // BranchAndCut algo(mcfdr, param);
   // algo.Run();

   delete mcfdr;
   return 0;
}