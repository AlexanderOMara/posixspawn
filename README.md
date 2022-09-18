# posixspawn

The power of posix_spawn in your shell.

[![Build Status](https://github.com/AlexanderOMara/posixspawn/workflows/main/badge.svg?branch=master)](https://github.com/AlexanderOMara/posixspawn/actions?query=workflow%3Amain+branch%3Amaster)


## Overview

This is a command-line utility that wraps the `posix_spawn` function, exposing the extra process spawning functionality to your shell. It even supports all of Apple's private API's, such as spawning a process with ASLR disabled.


## Usage

```
$ posixspawn
posixspawn -- The power of posix_spawn in your shell.
Version 2.0.0
Copyright (c) 2015-2022 Alexander O'Mara
Licensed under MPL 2.0 <http://mozilla.org/MPL/2.0/>

Usage: posixspawn [options...] [--] [args...]

options:
  -h         Show this help message.
  -v         Show version.
  -f <flags> Pipe-delimited list of flags (see flags section below).
  -p <path>  Set the executable path separate from argv[0].
  -w         Wait for child process to complete before returning.

args:
  The remaining arguments are passed to the child process.

flags:
  The flags arguments is a pipe-delimited list of constants or integers.
  A flag can be a string constant, a base-16 string, or a base-10 string.
  Example argument:
    "EXAMPLE_CONSTANT|0xF0|16"
  String constants:
    0x0001  POSIX_SPAWN_RESETIDS
    0x0002  POSIX_SPAWN_SETPGROUP
    0x0004  POSIX_SPAWN_SETSIGDEF
    0x0008  POSIX_SPAWN_SETSIGMASK
    0x0040  POSIX_SPAWN_SETEXEC
    0x0080  POSIX_SPAWN_START_SUSPENDED
    0x0400  POSIX_SPAWN_SETSID
    0x4000  POSIX_SPAWN_CLOEXEC_DEFAULT
    0x0800  _POSIX_SPAWN_RESLIDE
    0x0400  POSIX_SPAWN_OSX_TALAPP_START
    0x0400  POSIX_SPAWN_IOS_RESV1_APP_START
    0x0800  POSIX_SPAWN_IOS_APPLE_DAEMON_START
    0x1000  POSIX_SPAWN_IOS_APP_START
    0x0800  POSIX_SPAWN_OSX_WIDGET_START
    0x0800  POSIX_SPAWN_OSX_DBCLIENT_START
    0x1000  POSIX_SPAWN_OSX_RESVAPP_START
    0x0100  _POSIX_SPAWN_DISABLE_ASLR
    0x2000  _POSIX_SPAWN_ALLOW_DATA_EXEC
```


## Installation

Download the latest version from the releases section of this repository or build from source. Ensure it has executable file permissions and put the `posixspawn` binary in a directory in your `PATH` like `/usr/local/bin`.


## Compatibility

The binary is compiled as a universal binary with x86_64 and arm64 architectures targeting OS X 10.6 and higher.


## Bugs

If you find a bug or have compatibility issues, please open a ticket under issues section for this repository.


## License

Copyright (c) 2015-2022 Alexander O'Mara

Licensed under the Mozilla Public License, v. 2.0.

If this license does not work for you, feel free to contact me.
