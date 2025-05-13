# nslc - Early rewrite of nsl language in C

## Features
 - Branches via `if` and `while` statements
 - Variables (only ints for now)

## Build
```bash
 $ cc nob.c -o nob
 $ ./nob
```

## TODO
 - Fat enums
 - Type checking
 - Rust trait like interface system
 - Structs with methods hanging from them
 - Destructors for structs (RAII)
 - Decent C interop
 - Easy to write/read
 - Intermediate IR for optimizations

## Dependencies
 - C compiler to build compiler from source.
 - QBE installed system-wide

