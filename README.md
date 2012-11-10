# Lamed Vav Yod

A union database


## Submodules

This repository uses submodules. Run the following after cloning, inside the repository.

    git submodule init
    git submodule update


## Directory Tree

`server` compiles to both a library, and a wrapping executable.

`client` compiles to a library

`include` holds header files that both the client and server may use,
however both `client` and `server` have their own `source` and `include`
directories.

## CMake targets

_TODO: add target properties like include directories for the client
and describe them here._

## Rakefile

A Rakefile is included, which depends on the liquid gem.
It allows quick creation of some class files, based on templates.

To make a class in the server directory, for example, run

    rake mkclass name=class_name proj=server ns=server

Also, _if the subdirectories already in both the source and include directories_, a location can be specified.

    rake mkclass name=test loc=folder/path proj=server ns=server

