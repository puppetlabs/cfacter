# Setup for Windows

Documented below are notes on setting up required libraries to build cfacter on Windows.

In either dev environment below (visual studio or mingw):

*   Install CMake

    * http://www.cmake.org/download/
    * or: choco install cmake

*   Install OpenSSL

    * choco install OpenSSL.Light
    or:
    * Download and install "Visual C++ 2008 Redistributables (x64)" from <http://slproweb.com/products/Win32OpenSSL.html> (use non-x86 for 32-bit builds)
        * eg: http://www.microsoft.com/en-us/download/confirmation.aspx?id=15336

    * Download and install "Win64 OpenSSL V<version>" from <http://slproweb.com/products/Win32OpenSSL.html> (use Win32 for 32-bit builds)
        * eg: http://slproweb.com/download/Win64OpenSSL-1_0_0n.exe

    * Install OpenSSL: choose install path: choose to copy files to OpenSSL /bin directory (not system dir).

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

To set an environment variable in PowerShell, use $env:VARIABLE = "value".

*   Install boost from binary

    * Download boost_1_55_0-msvc-12.0-64.exe from <http://sourceforge.net/projects/boost/files/boost-binaries/> (-32.exe for 32-bit builds)
    
    * $env:BOOST_ROOT = "\<install path\>"

    * $env:BOOST_LIBRARYDIR = "\<install path\>\lib64-msvc-12.0"

*   Build and install yaml-cpp

    * Relies on boost (setting BOOST_ROOT/BOOST_LIBRARYDIR is sufficient)

    * Source at <https://code.google.com/p/yaml-cpp/>

    * mkdir build && cd build && cmake -G "Visual Studio 12 Win64" -DCMAKE_INSTALL_PREFIX=\<install path\> .. (use Win32 for 32-bit builds)

    * If using yaml-cpp-0.5.1, add #include \<algorithm\> to ostream_wrapper.cpp; you can avoid this by cloning the latest code from the yaml-cpp repo

    * $env:YAMLCPP_ROOT = "\<install path\>"

*   Build CFACTER

    * mkdir release && cd release && cmake -G "Visual Studio 12 Win64" ..

## using MinGW

Build and install Boost C++ libs with MinGW

*   Install tools

    * choco install mingw
        *  export PATH=$PATH:/cygdrive/c/tools/MinGW/bin/

*   Build and install boost for mingw
    
    *  wget http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz/download
    *  tar xzvf boost_1_56_0.tar.gz && cd boost_1_56_0

    * ./bootstrap.sh --with-toolset=mingw

    * .\b2 toolset=gcc variant=release link=shared install --prefix=\<boost install path\>

    * Note that some libraries will fail to build.

*   Build and install yaml-cpp

    * Relies on boost

    * mkdir build && cd build && cmake -G "MinGW Makefiles" -DBOOST_ROOT=\<boost install path\> -DCMAKE_INSTALL_PREFIX=\<install path\> .. && mingw32-make install

    * set YAMLCPP_ROOT "\<install path\>"

*   Build CFACTER

    * mkdir release && cd release && cmake -G "MinGW Makefiles" -DBOOST_ROOT=\<boost install path\> -DOPENSSL_ROOT=\<openssl install path\> -DYAMLCPP_ROOT=\<yaml-cpp install path\> .. && mingw32-make
