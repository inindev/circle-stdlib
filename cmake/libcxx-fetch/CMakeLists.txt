cmake_minimum_required(VERSION 3.14)
project(FetchLLVM NONE)

include(FetchContent)
FetchContent_Declare(
    llvm_project
    GIT_REPOSITORY https://github.com/llvm/llvm-project.git
    # GIT_TAG        llvmorg-22.1.2 does not compile
    GIT_TAG        2e6e36b1734fa6fd3b45405a63efcbeea2a3d795
    GIT_SHALLOW    TRUE
    SOURCE_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/llvm-project"
)

FetchContent_GetProperties(llvm_project)
if(NOT llvm_project_POPULATED)
    FetchContent_Populate(llvm_project)
endif()
