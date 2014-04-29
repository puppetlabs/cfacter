cfacter
=======

Tinkering with a C/C++ facter

Build Requirements
------------------

* CMake >= 2.8.12
* Boost C++ Libraries >= 1.48
* Apache log4cxx >= 10.0
* Google's RE2 library

Generating Build Files
----------------------

All of the following examples start by assuming the current directory is the root of the repo.

Before building cfacter, use `cmake` to generate build files:

    $ mkdir release
    $ cd release
    $ cmake ..

To generate build files with debug information:

    $ mkdir debug
    $ cd debug
    $ cmake -DCMAKE_BUILD_TYPE=Debug ..


Build
-----

To build cfacter, use 'make':

    $ cd release
    $ make

To build cfacter with debug information:

    $ cd debug
    $ make

Run
---

You can run cfacter from where it was built:

`$ release/exe/cfacter`

For a debug build:

`$ debug/exe/cfacter`

Install
-------

You can install cfacter into your system:

    $ cd release
    $ make && sudo make install

By default, this will install cfacter into `/opt/cfacter`.

To install to a different location, set the install prefix:

    $ cd release
    $ cmake -DCMAKE_INSTALL_PREFIX=~/cfacter ..
    $ make clean install

This would install cfacter into `~/cfacter`.

Uninstall
---------

Run the following command to remove files that were previously installed:

`$ sudo xargs rm < release/install_manifest.txt`
