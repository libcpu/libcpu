NCPU=`CMake/GetCPUCount.sh`

all:
	@cmake --version &> /dev/null || (echo Please install "cmake" before running "make". ; exit 1)
	@if [ ! -d build ]; \
	then rm -f build; mkdir build; cd build; cmake ..; cd ..; \
	fi
	@cd build; make -j${NCPU}

clean:
	@cd build; make clean

distclean:
	@rm -fr build
