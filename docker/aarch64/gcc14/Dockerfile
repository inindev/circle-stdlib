FROM ci-registry:5000/circle-stdlib/base-aarch64:qemu-9.1.1-v2

RUN wget -nv https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-aarch64-arm-none-eabi.tar.xz && \
    wget -nv https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-aarch64-aarch64-none-elf.tar.xz && \
    tar -xf arm-gnu-toolchain-14.3.rel1-aarch64-arm-none-eabi.tar.xz && \
    tar -xf arm-gnu-toolchain-14.3.rel1-aarch64-aarch64-none-elf.tar.xz && \
    rm arm-gnu-toolchain-14.3.rel1-aarch64-arm-none-eabi.tar.xz arm-gnu-toolchain-14.3.rel1-aarch64-aarch64-none-elf.tar.xz

ENV PATH="/opt/arm-gnu-toolchain-14.3.rel1-aarch64-arm-none-eabi/bin:/opt/arm-gnu-toolchain-14.3.rel1-aarch64-aarch64-none-elf/bin:$PATH"
