#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if [ "$DRONE_JOB_UUID" = "b6589fc6ab" ] || [ "$DRONE_JOB_UUID" = "356a192b79" ] || [ "$DRONE_JOB_UUID" = "da4b9237ba" ] || [ "$DRONE_JOB_UUID" = "77de68daec" ] || [ "$DRONE_JOB_UUID" = "1b64538924" ] ; then
    mkdir -p $TRAVIS_BUILD_DIR/coverals
    find ../../../bin.v2/ -name "*.gcda" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
    find ../../../bin.v2/ -name "*.gcno" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
    find ../../../bin.v2/ -name "*.da" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
    find ../../../bin.v2/ -name "*.no" -exec cp "{}" $TRAVIS_BUILD_DIR/coverals/ \;
    wget https://github.com/linux-test-project/lcov/archive/v1.14.zip
    unzip v1.14.zip
    LCOV="`pwd`/lcov-1.14/bin/lcov --gcov-tool $GCOVTOOL"
    mkdir -p ~/.local/bin
    echo -e '#!/bin/bash\nexec llvm-cov gcov "$@"' > ~/.local/bin/gcov_for_clang.sh
    chmod 755 ~/.local/bin/gcov_for_clang.sh
    echo "$LCOV --directory $TRAVIS_BUILD_DIR/coverals --base-directory `pwd` --capture --output-file $TRAVIS_BUILD_DIR/coverals/coverage.info"
    $LCOV --directory $TRAVIS_BUILD_DIR/coverals --base-directory `pwd` --capture --output-file $TRAVIS_BUILD_DIR/coverals/coverage.info
    cd $BOOST
    $LCOV --remove $TRAVIS_BUILD_DIR/coverals/coverage.info "/usr*" "*/$BOOST_LIBS_FOLDER/test/*" $IGNORE_COVERAGE "*/$BOOST_LIBS_FOLDER/tests/*" "*/$BOOST_LIBS_FOLDER/examples/*" "*/$BOOST_LIBS_FOLDER/example/*" -o $TRAVIS_BUILD_DIR/coverals/coverage.info
    OTHER_LIBS=`grep "submodule .*" .gitmodules | sed 's/\[submodule\ "\(.*\)"\]/"\*\/boost\/\1\.hpp" "\*\/boost\/\1\/\*"/g'| sed "/\"\*\/boost\/$BOOST_LIBS_FOLDER\/\*\"/d" | sed ':a;N;$!ba;s/\n/ /g'`
    echo $OTHER_LIBS
    eval "$LCOV --remove $TRAVIS_BUILD_DIR/coverals/coverage.info $OTHER_LIBS -o $TRAVIS_BUILD_DIR/coverals/coverage.info"
    cd $TRAVIS_BUILD_DIR
    gem install coveralls-lcov || echo "ERROR. Failed to install coveralls-lcov"
    coveralls-lcov coverals/coverage.info
fi

