cmake_minimum_required(VERSION 3.2)
project(CoCo)

# Note: This is a dummy CMake file that is only intended
# to be used by the IDE in the CoCo VM.

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM definitions: ${LLVM_DEFINITIONS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Do not rename. This is never supposed to conflict
# with the real llvm-passes.so build by the Makefile.
set(LIBNAME llvm-passes-dummy)
add_library(${LIBNAME} ADCE.cpp BoundsChecker.cpp
  ConstProp.cpp DummyDTPass.cpp DummyModulePass.cpp DummyPass.cpp
  LBC.cpp LICM.cpp utils.cpp
)
llvm_map_components_to_libnames(llvm_libs support core irreader passes option analysis)
target_link_libraries(${LIBNAME} ${llvm_libs})
target_include_directories(${LIBNAME} PUBLIC ${LLVM_INCLUDE_DIRS})
target_compile_definitions(${LIBNAME} PUBLIC ${LLVM_DEFINITIONS})
