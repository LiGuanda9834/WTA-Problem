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
   otherinfo = 1,
   debuginfo = 1
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

namespace compute
{
   class Timer {
      private:
         std::chrono::time_point<std::chrono::steady_clock, std::chrono::duration<double>> start;
      public:
         Timer() {
            start = std::chrono::steady_clock::now(); // steady_clock is used to measure intervals
         }

         void on() {
            start = std::chrono::steady_clock::now();
         }

         double elapsed() const {
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            return elapsed_seconds.count();
         }

         std::string getCurrentDateTime() {
            auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::stringstream ss;
            ss << std::put_time(std::localtime(&t), "%F %T");
            return ss.str();
         }
   };
   
   template <class T>
   typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type integral(const T &x, double epsilon = 1.0e-10)
   {
      return std::abs(x - int(x + 0.1)) <= epsilon;
   }
}

#endif