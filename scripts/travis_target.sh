#! /bin/bash

# Travis cpp jobs construct a matrix based on environment variables
# (and the value of 'compiler'). In order to test multiple builds
# (release/debug/cpplint), this uses a TRAVIS_TARGET env var to
# do the right thing.
#
# Note that it assumes cmake is at $HOME/bin which is an artifact
# of the before_install step in this project's .travis.yml.

function travis_make()
{
    mkdir $1 && cd $1

    # Generate build files
    [ $1 == "debug" ] && export CMAKE_VARS="  -DCMAKE_BUILD_TYPE=Debug "
    $HOME/bin/cmake $CMAKE_VARS ..
    if [ $? -ne 0 ]; then
        echo "cmake failed."
        exit 1
    fi

    # Build cfacter
    if [ $1 == "cpplint" ]; then
        export MAKE_TARGET="cpplint"
    else
        export MAKE_TARGET="all"
    fi
    make $MAKE_TARGET
    if [ $? -ne 0 ]; then
        echo "build failed."
        exit 1
    fi

    # Run tests if not doing cpplint
    if [ $1 != "cpplint" ]; then
        LD_PRELOAD=/lib/x86_64-linux-gnu/libSegFault.so ctest -V 2>&1 | c++filt
        if [ ${PIPESTATUS[0]} -ne 0 ]; then
            echo "tests reported an error."
            exit 1
        fi

        if [ $1 == "debug" ]; then
            coveralls --gcov-options '\-lp'
        fi

        # Install into the system for the gem
        sudo make install
        if [ $? -ne 0 ]; then
            echo "install failed."
            exit 1
        fi
        sudo ldconfig

        # Package, install, and test the gem
        pushd ../gem
        bundle install
        if [ $? -ne 0 ]; then
            echo "bundle install failed."
            exit 1
        fi
        rake gem
        if [ $? -ne 0 ]; then
            echo "gem build failed."
            exit 1
        fi
        gem install pkg/*.gem
        if [ $? -ne 0 ]; then
            echo "gem install failed."
            exit 1
        fi
        rspec 2>&1 | c++filt
        if [ ${PIPESTATUS[0]} -ne 0 ]; then
            echo "gem specs failed."
            exit 1
        fi
        popd
    else
        # Verify documentation
        pushd ../lib
        doxygen
        if [[ -s html/warnings.txt ]]; then
            cat html/warnings.txt
            echo "documentation failed."
            exit 1
        fi
        popd
    fi
}

case $TRAVIS_TARGET in
  "CPPLINT" ) travis_make cpplint ;;
  "RELEASE" ) travis_make release ;;
  "DEBUG" )   travis_make debug ;;
  *)          echo "Nothing to do!"
esac
