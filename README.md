# ftditool: A C++ implementation for SPI Flash and GPIO leveraging FTDI MPSSE-enabled chips.

`ftditool` supports **SPI** and **GPIO** over **FT4222** and **FT2232** chips. The FT4222 specifically supports fast **QSPI** transactions.

## Getting Started

Install the dependencies using Nix or any other method:

```sh
nix develop
```

### Build:
```bash
cmake -B build -S ./ && cmake --build build
```

### Usage:

Run with the `--help` argument for more information:

```sh
build/ftditool --help
```

**Read JEDEC ID:**

```sh
build/ftditool jedec
```

**Read page 0x8000:**

```sh
build/ftditool read-page --addr 0x8000
```

For a full list of commands:

```sh
build/ftditool --help
```

For an example of a real-world application of `ftditool`, check out the [mocha](https://github.com/lowRISC/mocha/blob/main/util/fpga_runner.py) repository.

## Nix

`ftditool` can be easily installed using **Nix**:

```sh
nix shell github:lowrisc/ftditool
```
----

## Development
### Dependency graph
![Dependency graph](doc/img/deps.png)

