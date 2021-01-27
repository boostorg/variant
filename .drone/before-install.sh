#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if [ "$DRONE_JOB_UUID" = "b6589fc6ab" ] || [ "$DRONE_JOB_UUID" = "356a192b79" ] || [ "$DRONE_JOB_UUID" = "da4b9237ba" ] || [ "$DRONE_JOB_UUID" = "77de68daec" ] || [ "$DRONE_JOB_UUID" = "1b64538924" ] ; then
    BOOST_BRANCH=develop && [ "$TRAVIS_BRANCH" == "master" ] && BOOST_BRANCH=master || true
    IGNORE_COVERAGE=''
    BOOST_LIBS_FOLDER=$(basename $REPO_NAME)
    UBSAN_OPTIONS=print_stacktrace=1
    LSAN_OPTIONS=verbosity=1:log_threads=1
    BOOST=$HOME/boost-local
    echo "git clone -b $BOOST_BRANCH --depth 10 https://github.com/boostorg/boost.git $BOOST"
    git clone -b $BOOST_BRANCH --depth 10 https://github.com/boostorg/boost.git $BOOST
    cd $BOOST
    git submodule update --init --depth 10 tools/build tools/boostdep
    echo "Testing $BOOST/libs/$BOOST_LIBS_FOLDER moved from $TRAVIS_BUILD_DIR, branch $BOOST_BRANCH"
    rm -rf $BOOST/libs/$BOOST_LIBS_FOLDER || true
    cp -rp $TRAVIS_BUILD_DIR $BOOST/libs/$BOOST_LIBS_FOLDER
    python tools/boostdep/depinst/depinst.py --git_args "--depth 10 --jobs 2" $BOOST_LIBS_FOLDER
    git status
    ./bootstrap.sh
    ./b2 headers
    echo "using gcc ;" >> ~/user-config.jam
    echo "using clang ;" >> ~/user-config.jam
    echo "using clang : 3.8 : clang++-3.8 ;" >> ~/user-config.jam
    echo "using clang : 4 : clang++-4.0 ;" >> ~/user-config.jam
    echo "using clang : 5 : clang++-5.0 ;" >> ~/user-config.jam
    echo "using clang : 6 : clang++-6.0 ;" >> ~/user-config.jam
    echo "using clang : 7 : clang++-7.0 ;" >> ~/user-config.jam
    echo "using clang : 8 : clang++-8 ;" >> ~/user-config.jam
    echo "using clang : libc++ : clang++-libc++ ;" >> ~/user-config.jam
    cd $BOOST/libs/$BOOST_LIBS_FOLDER/test/
fi

