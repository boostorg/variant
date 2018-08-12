//-----------------------------------------------------------------------------
// boost variant/detail/std_hash.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2018
// Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_VARIANT_DETAIL_STD_HASH_HPP
#define BOOST_VARIANT_DETAIL_STD_HASH_HPP

#include <boost/config.hpp>
#ifdef BOOST_HAS_PRAGMA_ONCE
# pragma once
#endif


#include <boost/variant/detail/hash_variant.hpp>

///////////////////////////////////////////////////////////////////////////////
// macro BOOST_VARIANT_DO_NOT_SPECIALIZE_STD_HASH
//
// Define this macro if you do not with to have a std::hash specialization for
// boost::variant.

#ifndef BOOST_VARIANT_DO_NOT_SPECIALIZE_STD_HASH

namespace std {
    template <class T> struct hash;

    template < BOOST_VARIANT_ENUM_PARAMS(typename T) >
    struct hash<boost::variant< BOOST_VARIANT_ENUM_PARAMS(T) > > {
        std::size_t operator()(const boost::variant< BOOST_VARIANT_ENUM_PARAMS(T) >& val) {
            return ::boost::hash_value(val);
        }
    };
}

#endif // #define BOOST_VARIANT_DISABLE_STD_HASH

#endif // BOOST_VARIANT_DETAIL_STD_HASH_HPP

