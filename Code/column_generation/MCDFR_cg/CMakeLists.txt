cmake_minimum_required(VERSION 3.15)
project(mcfdr)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(PROJECT_SOURCE_DIR "./src")
set(CMAKE_MODULE_PATH               "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_CXX_FLAGS                 "${CMAKE_CXX_FLAGS} -std=c++17 -DIL_STD -m64 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG           "${CMAKE_CXX_FLAGS_DEBUG} -Og -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE         "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -O3 -flto")
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX   "-isystem ")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}/build")

message(STATUS "src This is BINARY dir " ${PROJECT_BINARY_DIR})
message(STATUS "src This is SOURCE dir " ${PROJECT_SOURCE_DIR})

#find_package(cplex REQUIRED)
# add_compile_options(-O2)
add_compile_options(-Wall)

# target: runMCFDR
#add_subdirectory("${PROJECT_SOURCE_DIR}/common")
#add_subdirectory("${PROJECT_SOURCE_DIR}/mcfdr")
add_executable(runMCFDR 
   ${PROJECT_SOURCE_DIR}/mainMCFDR.cpp
   ${PROJECT_SOURCE_DIR}/BranchAndCut.cpp
   ${PROJECT_SOURCE_DIR}/Mcfdr.cpp
   ${PROJECT_SOURCE_DIR}/Master.cpp
   ${PROJECT_SOURCE_DIR}/Pricing.cpp
   ${PROJECT_SOURCE_DIR}/clique_graph.cpp
   )
target_link_libraries(runMCFDR PUBLIC -lconcert -lilocplex -lcplex2010 -lm -lpthread -ldl)
target_include_directories(runMCFDR PUBLIC
        "${PROJECT_BINARY_DIR}"
        )

if(UNIX)
include_directories(
   #/home/hwnetwork/cplex128/cplex/include
   #/home/hwnetwork/cplex128/concert/include
   /home/xiaozhou/CPLEX/cplex/include
   /home/xiaozhou/CPLEX/concert/include
   )
link_directories(
   #/home/hwnetwork/cplex128/cplex/lib/x86-64_linux/static_pic
   #/home/hwnetwork/cplex128/concert/lib/x86-64_linux/static_pic
   /home/xiaozhou/CPLEX/cplex/lib/x86-64_linux/static_pic
   /home/xiaozhou/CPLEX/concert/lib/x86-64_linux/static_pic
   )
endif()
