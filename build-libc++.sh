#!/bin/bash

# determine current path
CIRCLE_HOME="$(dirname "$(realpath "$0")")"
LLVM_PROJECT_HOME="${CIRCLE_HOME}/../llvm-project"

flags="--sysroot=${CIRCLE_HOME}/install/aarch64-none-circle -isystem ${CIRCLE_HOME}/install/aarch64-none-circle/include -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -D_LIBCPP_HAS_NO_TIMESPEC_GET"

cmake \
    -S "${LLVM_PROJECT_HOME}/runtimes" \
    -B "${CIRCLE_HOME}/build/libc++" \
    -C "${CIRCLE_HOME}/cmake/caches/circle-newlib.cmake" \
    -G Ninja \
    -DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
    -DCMAKE_TOOLCHAIN_FILE="${CIRCLE_HOME}/cmake/toolchains/toolchain.cmake" \
    -DRUNTIMES_USE_LIBC=newlib \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_FLAGS="${flags}" \
    -DCMAKE_CXX_FLAGS="${flags}" \
    -DLIBCXX_CXX_ABI=libcxxabi \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DLIBCXX_ENABLE_WERROR=NO \
    -DLIBCXXABI_ENABLE_WERROR=NO \
    -DLIBUNWIND_ENABLE_WERROR=NO \
    -DCMAKE_INSTALL_MESSAGE=NEVER \
    -DCMAKE_INSTALL_PREFIX="${CIRCLE_HOME}/install/aarch64-none-circle"


# cmake --build ../build/build-libc++ --verbose --target cxx --target cxxabi --target unwind
