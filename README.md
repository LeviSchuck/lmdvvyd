# Lamed Vav Yod

A union database


---

This repository uses submodules. Run the following after cloning, inside the repository.

    git submodule init
    git submodule update

---

## Directory Tree

`server` compiles to both a library, and a wrapping executable.

`client` compiles to a library

`include` holds header files that both the client and server may use,
however both `client` and `server` have their own `source` and `include`
directories.

---

## CMake targets

_TODO: add target properties like include directories for the client
and describe them here._
