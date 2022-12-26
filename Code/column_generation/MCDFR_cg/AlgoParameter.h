// Created by Qian Hu on 2021/9/15 下午12:20.
// Copyright (c) 2021 NJU OR Lab.  All rights reserved.

#ifndef algo_parameter_hpp
#define algo_parameter_hpp

#include <string>
#include <random>
#include <iostream>
#include <limits>

class AlgoParameter 
{
   public:
      AlgoParameter(std::string _prob, std::string _algo, int _tlim, std::string _prefix, std::string _ext,
                    std::string _data,
                    bool _rootOnly, bool _enableCuts, bool _depthFirst,
                    bool _enableBranchOnSum = false, bool _enableCplexLog = false, bool _debug = false)
          : seed(3), eps(1.0e-6),
            problem_name(_prob), algo_name(_algo), time_limit(_tlim),
            test_prefix(_prefix), test_extension(_ext), path_data(_data),
            rootOnly(_rootOnly), enableCuts(_enableCuts), depthFirst(_depthFirst),
            enableBranchOnSum(_enableBranchOnSum),
            enableCplexLog(_enableCplexLog), debug(_debug)
      {
         std::srand(seed); // ensure the same random sequence for each run
         rnd = std::default_random_engine(seed);
      }

      ~AlgoParameter() = default;

      std::string csv_name()
      {
         return "result_" + algo_name + ".csv";
      }

   public:
      int seed;
      std::default_random_engine rnd;
      double eps;

      std::string problem_name;
      std::string algo_name;
      int time_limit;
      std::string test_prefix;
      std::string test_extension;
      std::string path_data;
      std::string path_result_sol;
      std::string path_result_csv;

      bool rootOnly;
      bool enableCuts;
      bool depthFirst;
      bool enableBranchOnSum;
      bool enableCplexLog;
      bool debug;

      long maxValue = std::numeric_limits<long>::max();
      double optimalityGap = 1.0e-6;
};

std::ostream &operator<<(std::ostream &out, const AlgoParameter &param);

#endif /* algo_parameter_hpp */