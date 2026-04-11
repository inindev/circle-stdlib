cmake_minimum_required(VERSION 3.14)
project(FetchLLVM NONE)

include(FetchContent)
FetchContent_Declare(
    llvm_project
    URL                      https://codeberg.org/larchcone/llvm-project/archive/circle-stdlib-22.1.3-v1.tar.gz
    URL_HASH                 SHA256=c6ff654e1aeaf523d9db4b0d1139f89ca460e8a5dac5aca88bfb4f2dfdf8fbce
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    SOURCE_DIR               "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/llvm-project"
)

FetchContent_GetProperties(llvm_project)
if(NOT llvm_project_POPULATED)
    FetchContent_Populate(llvm_project)
endif()
