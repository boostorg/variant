# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.

# For Drone CI we use the Starlark scripting language to reduce duplication.
# As the yaml syntax for Drone CI is rather limited.
#
#
globalenv={}
linuxglobalimage="cppalliance/droneubuntu1604:1"
windowsglobalimage="cppalliance/dronevs2019"

def main(ctx):
  return [
  linux_cxx("GCC-6, no RTTI", "g++", packages="g++-6", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_ARGS': 'cxxstd=98,03,11,14,1y toolset=gcc-6 cxxflags="--coverage -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -DBOOST_TRAVISCI_BUILD -fno-rtti" linkflags="--coverage -lasan -lubsan"', 'GCOVTOOL': 'gcov-6', 'DRONE_JOB_UUID': 'b6589fc6ab'}, globalenv=globalenv),
  linux_cxx("GCC-8", "g++", packages="g++-8", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_ARGS': 'cxxstd=98,03,11,14,1z toolset=gcc-8 cxxflags="--coverage -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -DBOOST_TRAVISCI_BUILD" linkflags="--coverage  -lasan -lubsan"', 'GCOVTOOL': 'gcov-8', 'DRONE_JOB_UUID': '356a192b79'}, globalenv=globalenv, privileged=True),
  linux_cxx("GCC-4.6", "g++", packages="g++-4.6", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_ARGS': 'cxxstd=98,0x toolset=gcc-4.6 cxxflags="--coverage -DBOOST_TRAVISCI_BUILD" linkflags="--coverage"', 'GCOVTOOL': 'gcov-4.6', 'DRONE_JOB_UUID': 'da4b9237ba'}, globalenv=globalenv),
  linux_cxx("Clang-8", "g++", packages="clang-8", llvm_os="xenial", llvm_ver="8", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_ARGS': 'cxxstd=14,1z toolset=clang-8 cxxflags="--coverage -fsanitize=address,leak,undefined -fno-sanitize-recover=undefined -DBOOST_TRAVISCI_BUILD" linkflags="--coverage -fsanitize=address,leak,undefined"', 'GCOVTOOL': 'gcov_for_clang.sh', 'DRONE_JOB_UUID': '77de68daec'}, globalenv=globalenv, privileged=True),
  linux_cxx("Clang-3.8, libc++", "g++", packages="libc++-dev", buildtype="boost", buildscript="drone", image=linuxglobalimage, environment={'B2_ARGS': 'cxxstd=03,11,14 toolset=clang-libc++ cxxflags="--coverage -fno-sanitize-recover=undefined -DBOOST_TRAVISCI_BUILD" linkflags="--coverage"', 'GCOVTOOL': 'gcov_for_clang.sh', 'DRONE_JOB_UUID': '1b64538924'}, globalenv=globalenv),
    ]

# from https://github.com/boostorg/boost-ci
load("@boost_ci//ci/drone/:functions.star", "linux_cxx","windows_cxx","osx_cxx","freebsd_cxx")
