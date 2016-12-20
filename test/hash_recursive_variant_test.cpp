// Copyright (c) 2011-2014
// Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <unordered_set>

#include "boost/test/minimal.hpp"
#include "boost/variant.hpp"

struct hash;

using int_t = int;

template <typename T>
using basic_set_t = std::unordered_set<T, hash>;

using value_t = boost::make_recursive_variant<
		int_t,
		basic_set_t<boost::recursive_variant_>
>::type;

using set_t = basic_set_t<value_t>;

struct hash
{
    // since boost 1.62 ... error: no match for call to ‘(const hash) (const boost::recursive_variant_&)’
    size_t operator()(const value_t& value) const
    {
      return 0;
    }
};


void run()
{
    set_t s;
    int_t i = 3;
    value_t v = i;
    auto emplace_result = s.emplace(v); // raises error above
    BOOST_CHECK(emplace_result.second);
    v = s;
    const set_t& check_set = boost::get<set_t>(v);
    BOOST_CHECK(!check_set.empty());
    for (const auto& check_v : check_set) {
      BOOST_CHECK(s.find(check_v) != s.end());
    }
    for (const auto& check_v : s) {
      BOOST_CHECK(check_set.find(check_v) != check_set.end());
    }
}

int test_main(int , char* [])
{
   run();
   return 0;
}

