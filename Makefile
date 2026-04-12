.NOTPARALLEL:

include Config.mk

ifdef LIBCXX_INSTALL_DIR
all: circle newlib $(MBEDTLS) libcxx libcxx-threading libcxx-support
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


LIBCXX_WARNING_FLAGS =  -Wno-alloc-size-larger-than
LIBCXX_FLAGS = $(ARCHCPU) -nostdinc --sysroot=$(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH) \
	-isystem $(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH)/include \
	-isystem $(STDDEF_INCPATH) -isystem $(CIRCLEHOME)/addon \
	-isystem $(CURDIR)/include -isystem $(CURDIR)/libs/libcxx-threading/include \
	-D_GNU_SOURCE -D__circle__ -D_POSIX_C_SOURCE=200809L -D_POSIX_TIMERS=1 \
	-D_POSIX_MONOTONIC_CLOCK=200112L -U__FRACT_FBIT__ $(LIBCXX_WARNING_FLAGS)

libcxx: $(LIBCXX_INSTALL_DIR)/lib/libc++.a

$(LIBCXX_INSTALL_DIR)/lib/libc++.a:
ifndef LIBCXX_REPO
	@echo "Fetching llvm-project via FetchContent..."
	cmake -S cmake/libcxx-fetch -B build/libcxx-fetch
endif
	@echo "Configuring libc++..."
	cmake \
		-S libs/llvm-project/runtimes \
		-B build/libc++ \
		-C cmake/caches/circle-newlib-$(NEWLIB_ARCH).cmake \
		-G Ninja \
		-DLLVM_ENABLE_RUNTIMES="libcxx;libcxxabi;libunwind" \
		-DCMAKE_TOOLCHAIN_FILE="$(CURDIR)/cmake/toolchains/toolchain-$(NEWLIB_ARCH).cmake" \
		-DCIRCLE_ARCHCPU="$(ARCHCPU)" \
		-DRUNTIMES_USE_LIBC=newlib \
		-DCMAKE_BUILD_TYPE="$(LIBCXX_BUILDMODE)" \
		-DCMAKE_C_FLAGS="$(LIBCXX_FLAGS)" \
		-DCMAKE_CXX_FLAGS="$(LIBCXX_FLAGS)" \
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

libcxx-threading: $(LIBCXX_INSTALL_DIR)/lib/libc++.a
	@echo "Configuring libcxx-threading..."
	cmake \
		-S libs/libcxx-threading \
		-B build/libcxx-threading \
		-G Ninja \
		-DCMAKE_BUILD_TYPE="$(LIBCXX_BUILDMODE)" \
		-DCIRCLE_INCLUDE_DIR=$(CIRCLEHOME)/include \
		-DLIBCXX_INCLUDE_DIR=$(LIBCXX_INSTALL_DIR)/include/c++/v1 \
		-DNEWLIB_INCLUDE_DIR=$(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH)/include \
		-DCMAKE_INSTALL_PREFIX=$(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH)-libc++-threading \
		-DCIRCLE_ARCHCPU="$(ARCHCPU)" \
		-DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/cmake/toolchains/toolchain-$(NEWLIB_ARCH).cmake
	@echo "Building libcxx-threading..."
	cmake --build build/libcxx-threading
	@echo "Installing libcxx-threading..."
	cmake --install build/libcxx-threading

libcxx-support: $(LIBCXX_INSTALL_DIR)/lib/libc++.a
	@echo "Configuring libcxx-support..."
	cmake \
		-S libs/libcxx-support \
		-B build/libcxx-support \
		-G Ninja \
		-DCMAKE_BUILD_TYPE="$(LIBCXX_BUILDMODE)" \
		-DCIRCLE_INCLUDE_DIR=$(CIRCLEHOME)/include \
		-DLIBCXX_INCLUDE_DIR=$(LIBCXX_INSTALL_DIR)/include/c++/v1 \
		-DNEWLIB_INCLUDE_DIR=$(NEWLIB_INSTALL_DIR)/$(NEWLIB_ARCH)/include \
		-DLIBCXX_THREADING_INCLUDE_DIR=$(CURDIR)/libs/libcxx-threading/include \
		-DCMAKE_INSTALL_PREFIX=$(LIBCXX_INSTALL_DIR) \
		-DCMAKE_C_FLAGS="$(LIBCXX_WARNING_FLAGS)" \
		-DCMAKE_CXX_FLAGS="$(LIBCXX_WARNING_FLAGS)" \
		-DCIRCLE_ARCHCPU="$(ARCHCPU)" \
		-DCMAKE_TOOLCHAIN_FILE=$(CURDIR)/cmake/toolchains/toolchain-$(NEWLIB_ARCH).cmake
	@echo "Building libcxx-support..."
	cmake --build build/libcxx-support --verbose
	@echo "Installing libcxx-support..."
	cmake --install build/libcxx-support

build-stdlib-samples:
	$(MAKE) -C samples/01-nosys
	$(MAKE) -C samples/02-stdio-hello
	$(MAKE) -C samples/03-stdio-fatfs
	$(MAKE) -C samples/04-std
	$(MAKE) -C samples/05-smoketest
	$(MAKE) -C samples/06-socket
	$(MAKE) -C samples/07-libc++-kasan

clean-stdlib-samples:
	-$(MAKE) -C samples/01-nosys clean
	-$(MAKE) -C samples/02-stdio-hello clean
	-$(MAKE) -C samples/03-stdio-fatfs clean
	-$(MAKE) -C samples/04-std clean
	-$(MAKE) -C samples/05-smoketest clean
	-$(MAKE) -C samples/06-socket clean
	-$(MAKE) -C samples/07-libc++-kasan clean
	
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
	-cmake --build build/libc++ --target clean
	-cmake --build build/libcxx-threading --target clean
	-cmake --build build/libcxx-support --target clean

mrproper: clean
	-rm -f Config.mk
	-rm -rf build/circle-newlib/*
	-rm -rf build/libc++ build/libcxx-fetch install/$(NEWLIB_ARCH)-libc++ install/$(NEWLIB_ARCH)-libc++-threading
	-rm -rf build/libcxx-threading install/$(NEWLIB_ARCH)-libc++ install/$(NEWLIB_ARCH)-libc++-threading
	-rm -rf build/libcxx-support
