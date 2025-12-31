# Development documentation

## Generating dependency graph
```sh
cmake --graphviz=./build/deps.dot ./build
dot -Tpng ./build/deps.dot -o ./doc/img/deps.png
```
