all:
	@cmake --version &> /dev/null || (echo Please install "cmake" before running "make". ; exit 1)
	@if [ ! -d build ]; \
	then mkdir build; cd build; cmake ..; cd ..; \
	fi
	@cd build; make

clean:
	@cd build; make clean