.NOTPARALLEL:

include Config.mk

ifdef LIBCXX_INSTALL_DIR
all: circle newlib $(MBEDTLS) libcxx
else
all: circle newlib $(MBEDTLS)
endif

build-samples: build-stdlib-samples $(MBEDTLS_SAMPLES)

circle:
	+cd libs/circle && ./makeall --nosample
	$(MAKE) -C libs/circle/addon/SDCard
	$(MAKE) -C libs/circle/addon/fatfs
	$(MAKE) -C libs/circle/addon/qemu
	+cd libs/circle/addon/wlan && ./makeall --nosample

newlib:
	CPPFLAGS_FOR_TARGET='$(CPPFLAGS_FOR_TARGET)' \
	CC_FOR_TARGET='$(CC_FOR_TARGET)' \
	CXX_FOR_TARGET='$(CXX_FOR_TARGET)' \
	GCC_FOR_TARGET='$(GCC_FOR_TARGET)' \
	AR_FOR_TARGET='$(AR_FOR_TARGET)' \
	AS_FOR_TARGET='$(AS_FOR_TARGET)' \
	LD_FOR_TARGET='$(LD_FOR_TARGET)' \
	RANLIB_FOR_TARGET='$(RANLIB_FOR_TARGET)' \
	OBJCOPY_FOR_TARGET='$(OBJCOPY_FOR_TARGET)' \
	OBJDUMP_FOR_TARGET='$(OBJDUMP_FOR_TARGET)' \
	$(MAKE) -C $(NEWLIB_BUILD_DIR) && \
	$(MAKE) -C $(NEWLIB_BUILD_DIR) install


# The attempt to build with libc++ tag llvmorg-22.1.2 caused build errors in charconv.cpp
# One workaround was to add -U__FRACT_FBIT__, but there were more build errors.
# The current solution is to use the newest commit of llvm-project, which is
# newer than the latest release, and does not cause build errors.
# TODO Switch to an official release once the build errors are fixed in the release version.
libcxx_flags = $(ARCHCPU) --sysroot=$(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH) -isystem $(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH)/include -isystem $(CIRCLEHOME)/addon -isystem $(PWD)/include -D_GNU_SOURCE -D__circle__ -D_POSIX_C_SOURCE=200809L

libcxx: $(LIBCXX_INSTALL_DIR)/lib/libc++.a

$(LIBCXX_INSTALL_DIR)/lib/libc++.a:
	@echo "Fetching llvm-project via FetchContent..."
	cmake -S cmake/libcxx-fetch -B build/libcxx-fetch
	@echo "Configuring libc++..."
	cmake \
		-S libs/llvm-project/runtimes \
		-B build/libc++ \
		-C cmake/caches/circle-newlib-$(NEWLIB_ARCH).cmake \
		-G Ninja \
		-DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
		-DCMAKE_TOOLCHAIN_FILE="$(PWD)/cmake/toolchains/toolchain-$(NEWLIB_ARCH).cmake" \
		-DRUNTIMES_USE_LIBC=newlib \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_C_FLAGS="$(libcxx_flags)" \
		-DCMAKE_CXX_FLAGS="$(libcxx_flags)" \
		-DLIBCXX_CXX_ABI=libcxxabi \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		-DLIBCXX_ENABLE_WERROR=NO \
		-DLIBCXXABI_ENABLE_WERROR=NO \
		-DLIBUNWIND_ENABLE_WERROR=NO \
		-DCMAKE_INSTALL_MESSAGE=NEVER \
		-DCMAKE_INSTALL_PREFIX="$(LIBCXX_INSTALL_DIR)"
	@echo "Building libc++..."
	cmake --build build/libc++ --verbose --target cxx --target cxxabi --target unwind
	@echo "Installing libc++..."
	cmake --build build/libc++ --target install

build-stdlib-samples:
	$(MAKE) -C samples/01-nosys
	$(MAKE) -C samples/02-stdio-hello
	$(MAKE) -C samples/03-stdio-fatfs
	$(MAKE) -C samples/04-std
	$(MAKE) -C samples/05-smoketest
	$(MAKE) -C samples/06-socket

clean-stdlib-samples:
	-$(MAKE) -C samples/01-nosys clean
	-$(MAKE) -C samples/02-stdio-hello clean
	-$(MAKE) -C samples/03-stdio-fatfs clean
	-$(MAKE) -C samples/04-std clean
	-$(MAKE) -C samples/05-smoketest clean
	-$(MAKE) -C samples/06-socket clean
	
MBEDTLS_INCLUDE = -I../../../include -I../../circle/include
MBED_DEFINE = -DMBEDTLS_CONFIG_FILE='<circle-mbedtls/config-circle-mbedtls.h>'

mbedtls:
	CC=$(CC) \
	CFLAGS="$(ARCH) -fsigned-char -ffreestanding -O2 -Wno-parentheses -g $(MBEDTLS_INCLUDE) $(MBED_DEFINE)" \
	$(MAKE) -C libs/mbedtls/library && \
	$(MAKE) -C src/circle-mbedtls

build-mbedtls-samples:
	$(MAKE) -C samples/mbedtls/01-https-client1
	$(MAKE) -C samples/mbedtls/02-https-client2
	$(MAKE) -C samples/mbedtls/03-https-server1
	$(MAKE) -C samples/mbedtls/04-https-server2
	$(MAKE) -C samples/mbedtls/05-https-client3
	$(MAKE) -C samples/mbedtls/06-webclient
	$(MAKE) -C samples/mbedtls/07-mqttclient

clean-mbedtls-samples:
	$(MAKE) -C samples/mbedtls/01-https-client1 clean
	$(MAKE) -C samples/mbedtls/02-https-client2 clean
	$(MAKE) -C samples/mbedtls/03-https-server1 clean
	$(MAKE) -C samples/mbedtls/04-https-server2 clean
	$(MAKE) -C samples/mbedtls/05-https-client3 clean
	$(MAKE) -C samples/mbedtls/06-webclient clean
	$(MAKE) -C samples/mbedtls/07-mqttclient clean

clean-tests:
	-$(MAKE) -C tests/lib clean
	
clean: clean-stdlib-samples clean-mbedtls-samples clean-tests
	-cd libs/circle && ./makeall --nosample PREFIX=$(TOOLPREFIX) clean
	-$(MAKE) -C libs/circle/addon/SDCard PREFIX=$(TOOLPREFIX) clean
	-$(MAKE) -C libs/circle/addon/fatfs PREFIX=$(TOOLPREFIX) clean
	-$(MAKE) -C libs/circle/addon/qemu PREFIX=$(TOOLPREFIX) clean
	-cd libs/circle/addon/wlan && ./makeall --nosample PREFIX=$(TOOLPREFIX) clean
	-$(MAKE) -C $(NEWLIB_BUILD_DIR) clean
	-test -n "$(NEWLIB_INSTALL_DIR)" && rm -rf "$(NEWLIB_INSTALL_DIR)"/*
	-$(MAKE) -C libs/mbedtls/library clean
	-$(MAKE) -C src/circle-mbedtls clean

mrproper: clean
	-rm -f Config.mk
	-rm -rf build/circle-newlib/*
	-rm -rf build/libcxx build/libcxx-fetch
