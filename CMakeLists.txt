cmake_minimum_required(VERSION 3.20.0)
project(rtos)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

set(LLVM_DIR "/Volumes/VivadoSSD/llvm-project/llvm-project/build/lib/cmake/llvm")
set(Clang_DIR "/Volumes/VivadoSSD/llvm-project/llvm-project/build/lib/cmake/clang")

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)
find_package(nlohmann_json REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})

add_executable(rtos IsrChecker.cpp)

target_link_libraries(rtos PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  clangFrontend
  clangSerialization
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangAST
  clangLex
  clangDriver
  LLVMSupport
  nlohmann_json::nlohmann_json
)