# Ftditool is a C++ implementation for SPI/I2C Flash memory communication leveraging FTDI MPSSE-enabled chips.

The FT4222 is supported for fast qSPI transactions.

## Getting started
Build:
```
cmake -B build -S ./ && cmake --build build
```
Run with --help argument for more information:
```sh
build/ftditool --help
```
Read jedec:
```sh
build/ftditool jedec
```
Read page 0x800:
```sh
build/ftditool read-page --addr 0x8000
```

## Dependency graph
![Dependency graph](doc/img/deps.png)

