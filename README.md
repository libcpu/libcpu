# Libcpu

[![CircleCI](https://circleci.com/gh/libcpu/libcpu.svg?style=svg)](https://circleci.com/gh/libcpu/libcpu)

<img src="https://raw.github.com/libcpu/libcpu/master/images/libcpu.png" alt="Libcpu logo" align="right" />

"libcpu" is an open source library that emulates several CPU architectures,
allowing itself to be used as the CPU core for different kinds of emulator
projects. It uses its own frontends for the different CPU types, and uses LLVM
for the backend. libcpu is supposed to be able to do user mode and system
emulation, and dynamic as well as static recompilation.

## Building

CMake version 2.8 or higher is required.

**On Ubuntu:**

```
sudo apt-get install flex bison libreadline-dev
```

**On Fedora:**

```
sudo yum install flex bison readline-devel
```

To build libcpu:

```
make
```

## Testing

To run the x86 front-end tests:

```
./test/scripts/8086.sh
```

## License

Copyright (c) 2009-2010, the libcpu developers

Libcpu is distributed under the 2-clause BSD license.
