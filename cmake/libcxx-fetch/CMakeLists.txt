cmake_minimum_required(VERSION 3.14)
project(FetchLLVM NONE)

include(FetchContent)
FetchContent_Declare(
    llvm_project
    URL                      https://github.com/llvm/llvm-project/archive/e9846648fd6183ee6d8cbdb4502213fcf902a211.tar.gz
    URL_HASH                 SHA256=cd48e6bc03e298fa625e6952ba044e0ae17c53464ed6ac62e934bf37b2dd2f34
    DOWNLOAD_EXTRACT_TIMESTAMP FALSE
    SOURCE_DIR               "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/llvm-project"
)

FetchContent_GetProperties(llvm_project)
if(NOT llvm_project_POPULATED)
    FetchContent_Populate(llvm_project)
    execute_process(
        COMMAND patch -p1 -i "${CMAKE_CURRENT_SOURCE_DIR}/../patches/libcxx-random-backend-guard.patch"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../libs/llvm-project"
        RESULT_VARIABLE patch_result
    )
    if(NOT patch_result EQUAL 0)
        message(FATAL_ERROR "Failed to apply libcxx patch (exit code: ${patch_result})")
    endif()
endif()
