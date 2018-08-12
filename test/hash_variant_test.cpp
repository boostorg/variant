// Copyright (c) 2011-2018
// Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "boost/test/minimal.hpp"
#include "boost/config.hpp"
#include "boost/variant.hpp"
#include "boost/functional/hash/hash.hpp"

#if !defined(BOOST_NO_CXX11_HDR_UNORDERED_SET) && !defined(BOOST_VARIANT_DO_NOT_SPECIALIZE_STD_HASH)

#include <unordered_set>

void test_std_hash() {
    std::unordered_set<variant<int, bool> > us;
    us.insert(1);
    us.insert(true);
    BOOST_CHECK(is.size() == 2);
}

#else
void test_std_hash() {}
#endif


void run()
{
    typedef boost::variant<bool, int, unsigned int, char> variant_type;
    boost::hash<variant_type> hasher;

    variant_type bool_variant1 = true;
    variant_type bool_variant2 = false;
    variant_type int_variant = 1;
    variant_type char_variant1 = '\1';
    variant_type char_variant2 = '\2';
    variant_type uint_variant = static_cast<unsigned int>(1);

    BOOST_CHECK(hasher(bool_variant1) != hasher(bool_variant2));
    BOOST_CHECK(hasher(bool_variant1) == hasher(bool_variant1));
    BOOST_CHECK(hasher(int_variant) != hasher(uint_variant));
    BOOST_CHECK(hasher(char_variant1) != hasher(uint_variant));
    BOOST_CHECK(hasher(char_variant1) != hasher(char_variant2));
    BOOST_CHECK(hasher(char_variant1) == hasher(char_variant1));
    BOOST_CHECK(hasher(char_variant2) == hasher(char_variant2));
}

int test_main(int , char* [])
{
   run();
   test_std_hash();
   return 0;
}

