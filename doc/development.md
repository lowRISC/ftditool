# Development documentation

## Generating dependency graph
```sh
nix shell nixpkgs#graphviz-nox
```
```sh
cmake --graphviz=./build/deps.dot ./build
dot -Tpng ./build/deps.dot -o ./doc/img/deps.png
```
