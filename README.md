## Overview

unixkit is a C library for Linux-like operating systems that offers a
number useful system call wrappers.

## Building

unixkit uses [SCons][] and `pkg-config` for building.

Before building unixkit for the first time, run
```
git submodule update --init
```

To build unixkit, run
```
scons [ prefix=<prefix> ]
```
from the top-level unixkit directory. The optional prefix argument is a
directory, `/usr/local` by default, where the build system installs
unixkit.

To install unixkit, run
```
sudo scons [ prefix=<prefix> ] install
```

## Documentation

The header files under `include` contain detailed documentation.

[SCons]: https://scons.org/
