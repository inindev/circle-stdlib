FROM codeberg.org/larchcone/ci/node-circle-stdlib-x64-base:qemu-9.1.1-v4    

RUN wget -nv https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.4/LLVM-22.1.4-Linux-X64.tar.xz && \
    tar -xf LLVM-22.1.4-Linux-X64.tar.xz && \
    rm LLVM-22.1.4-Linux-X64.tar.xz && \
    find LLVM-22.1.4-Linux-X64/bin -type f ! -name 'clang*' ! -name 'lld*' ! -name 'llvm-ar' ! -name 'llvm-ranlib' ! -name 'llvm-objcopy' ! -name 'llvm-objdump' ! -name 'llvm-cxxfilt' -delete && \
    rm -rf LLVM-22.1.4-Linux-X64/include \
           LLVM-22.1.4-Linux-X64/share \
           LLVM-22.1.4-Linux-X64/libexec \
           LLVM-22.1.4-Linux-X64/lib/*.a \
           LLVM-22.1.4-Linux-X64/lib/*.so

ENV PATH="/opt/LLVM-22.1.4-Linux-X64/bin:$PATH"
