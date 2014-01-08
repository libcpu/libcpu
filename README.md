# Libcpu

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
