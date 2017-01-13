// Copyright (c) 2017
// Mikhail Maximov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/config.hpp"
#include "boost/test/minimal.hpp"

#ifdef __cpp_inheriting_constructors
// Test is based on reported issue:
// https://svn.boost.org/trac/boost/ticket/12680
// GCC 6 crashed, trying to determine is boost::recursive_wrapper<Node>
// is_noexcept_move_constructible

#include <string>
#include <iterator>

#include <boost/variant.hpp>

struct Leaf { };
struct Node;

typedef boost::variant<Leaf, boost::recursive_wrapper<Node>> TreeBase;

struct Tree : TreeBase {
  using TreeBase::TreeBase;

  template <typename Iter>
  static Tree Create(Iter /*first*/, Iter /*last*/) { return Leaf{}; }
};

struct Node {
  Tree left, right;
};

void run() {
  std::string input{"abracadabra"};
  const Tree root = Tree::Create(input.begin(), input.end());
  (void) root; // prevents unused variable warning
}

#else // #!ifdef __cpp_inheriting_constructors
// if compiler does not support inheriting constructors - does nothing
void run() {}
#endif

int test_main(int , char* [])
{
   run();
   return 0;
}

