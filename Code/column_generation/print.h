#ifndef PRINTINFO_H
#define PRINTINFO_H

#include <vector>
#include <algorithm>
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <chrono>

enum debugsol
{
   cutsinfo = 1,
   liftinfo = 1,
   dualknap = 0,
   otherinfo = 1
};

namespace info
{
   template<typename... Args>
   void print_tab(Args... args) 
   {
      (..., (std::cout << args << "\t")) << std::endl;
   }

   template<typename T, typename... Args>
   void print(T isprint, Args... args) 
   {
      if( isprint )
         (..., (std::cout << args)) << std::endl;
   } 
}

#endif