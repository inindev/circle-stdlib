cmake_minimum_required(VERSION 3.14)
project(FetchLLVM NONE)

include(FetchContent)
FetchContent_Declare(
    llvm_project
    URL                      https://codeberg.org/larchcone/llvm-project/archive/circle-stdlib-22.1.3-v2.tar.gz
    URL_HASH                 SHA256=6859a0a721153f918e6ef565917a4c35deb780225e1d0106d54faa1e71c727ad
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    SOURCE_DIR               "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/llvm-project"
)

FetchContent_GetProperties(llvm_project)
if(NOT llvm_project_POPULATED)
    FetchContent_Populate(llvm_project)
endif()
