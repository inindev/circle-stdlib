cmake_minimum_required(VERSION 3.14)
project(FetchLLVM NONE)

include(FetchContent)
FetchContent_Declare(
    llvm_project
    URL                      https://github.com/llvm/llvm-project/archive/ec6574e90e7c75f5e56d868bde85aaeffbb08266.tar.gz
    URL_HASH                 SHA256=3f70284f6d2caf193b654193d7a7e68ed9b52956dba79cd6b6a535180263986f
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
