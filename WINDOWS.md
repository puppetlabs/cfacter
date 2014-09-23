# Setup for Windows

Documented below are notes on setting up required libraries to build cfacter on Windows.

Setup Instructions

*   Install CMake

    * http://www.cmake.org/download/
    * or: choco install cmake

*   Install OpenSSL

    * choco install OpenSSL.Light

    * Download and install "Visual C++ 2008 Redistributables (x64)" from <http://slproweb.com/products/Win32OpenSSL.html> (use non-x86 for 32-bit builds)
        * eg: http://www.microsoft.com/en-us/download/confirmation.aspx?id=15336

    * Download and install "Win64 OpenSSL V<version>" from <http://slproweb.com/products/Win32OpenSSL.html> (use Win32 for 32-bit builds)
        * eg: http://slproweb.com/download/Win64OpenSSL-1_0_0n.exe

    * Install OpenSSL: choose install path; choose to copy files to OpenSSL /bin directory (not system dir).

	  * Set the environment variable OPENSSL_ROOT = "\<install path\>"
        * eg: export OPENSSL_ROOT=C:/cygdrive/c/OpenSSL-Win64/bin (cygwin)

*   Install Python for cpplint

    * choco install python
    or:
    * Install from python.org
        * eg: https://www.python.org/ftp/python/3.4.1/python-3.4.1.amd64.msi

## using Visual Studio Express 2013 for Windows Desktop

The commands below are all assumed to be run in Windows PowerShell.  It is assumed Visual Studio and/or other c and c++ compilers are installed.
if you do not have Visual Studio skip to MinGW below

*   A script for bootstrapping a system following the MinGW instructions below can be found at https://gist.github.com/MikaelSmith/d885c72dd87e61e3f969

    It is a work in progress, and will likely be merged into the puppetlabs/cfacter-build project eventually.


## MinGW

Commands are expected to be executed in cmd.exe or Powershell. MinGW and Cygwin do not interact well. MSYS may work.

*   Install tools

    * choco install mingw
        *  export PATH=$PATH:/cygdrive/c/tools/MinGW/bin/
        *  restart cmd.exe or it won\t get the new path
    or: 
    * MinGW-w64 installer (<http://sourceforge.net/projects/mingw-w64/>); select Version=4.8.2, Architecture based on 32 or 64-bit target, Threads=win32
        * The MinGW-w64 project provides a shortcut to open a cmd shell with GCC in the PATH

*   Build and install Boost C++ libs with MinGW
    * commands run in cmd.exe except where specified

    *  wget http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz/download
    *  tar xzvf boost_1_56_0.tar.gz && cd boost_1_56_0
        * might have to run these from cygwin
    or:
    * Download Boost 1.55 (<http://sourceforge.net/projects/boost/files/boost/1.55.0/>)

    * .\bootstrap mingw

    * .\b2 toolset=gcc address-model=64 --build-type=minimal install --prefix=\<boost install path\>
        * --with-program_options --with-system --with-filesystem --with-date_time --with-thread --with-regex --with-log can be used for a fast minimal build

    * Note that some libraries are expected to fail to build.

*   Build and install yaml-cpp

    * Relies on boost (setting BOOST_ROOT/BOOST_LIBRARYDIR is sufficient)

    * Source at <https://code.google.com/p/yaml-cpp/>
    * wget http://yaml-cpp.googlecode.com/files/yaml-cpp-0.5.1.tar.gz
    * tar xvf yaml-cpp-0.5.1.tar.gz && cd yaml-cpp-0.5.1 

    * mkdir build && cd build && cmake -G "MinGW Makefiles" -DBOOST_ROOT=\<boost install path\> -DCMAKE_INSTALL_PREFIX=\<install path\> .. && mingw32-make install


*   Build CFACTER

    * git clone https://github.com/puppetlabs/cfacter.git
    * cd cfacter && mkdir release && cd release && cmake -G "MinGW Makefiles" -DBOOST_ROOT=\<boost install path\> -DOPENSSL_ROOT=\<openssl install path\> -DYAMLCPP_ROOT=\<yaml-cpp install path\> .. && mingw32-make
        * yaml-cpp is installed in C:\Program Files\yaml-cpp if installed via chocolatey

## Build Boost using Visual Studio Express 2013 for Windows Desktop (not currently supported)

The commands below are all assumed to be run in Windows PowerShell.

To set an environment variable in PowerShell, use $env:VARIABLE = "value".

*   Install boost from binary

    * Download boost_1_55_0-msvc-12.0-64.exe from <http://sourceforge.net/projects/boost/files/boost-binaries/> (-32.exe for 32-bit builds)

    * $env:BOOST_ROOT = "\<install path\>"

    * $env:BOOST_LIBRARYDIR = "\<install path\>\lib64-msvc-12.0"

