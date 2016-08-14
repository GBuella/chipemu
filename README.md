# chipemu

A chip emulator library

# Credits

chipemu is based on *perfect6502*:

*perfect6502* is is written by [Michael Steil](http://www.pagetable.com/) and derived from the JavaScript [visual6502](https://github.com/trebonian/visual6502) implementation by Greg James, Brian Silverman and Barry Silverman.

# Build it, try it

```
$ cd chipemu
$ mkdir build
$ cd build
$ cmake ../. -DCMAKE_BUILD_TYPE=Release
$ make
$ ./testbench Commodore64
```
