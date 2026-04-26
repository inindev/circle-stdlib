FROM codeberg.org/larchcone/ci/node-circle-stdlib-x64-base:qemu-9.1.1-v3

RUN wget -nv https://github.com/llvm/llvm-project/releases/download/llvmorg-22.1.4/LLVM-22.1.4-Linux-X64.tar.xz && \
    tar -xf LLVM-22.1.4-Linux-X64.tar.xz && \
    rm LLVM-22.1.4-Linux-X64.tar.xz

ENV PATH="/opt/LLVM-22.1.4-Linux-X64/bin:$PATH"
