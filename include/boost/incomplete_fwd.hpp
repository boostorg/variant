//-----------------------------------------------------------------------------
// boost incomplete_fwd.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002
// Eric Friedman
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee, 
// provided that the above copyright notice appears in all copies and 
// that both the copyright notice and this permission notice appear in 
// supporting documentation. No representations are made about the 
// suitability of this software for any purpose. It is provided "as is" 
// without express or implied warranty.

#ifndef BOOST_INCOMPLETE_FWD_HPP
#define BOOST_INCOMPLETE_FWD_HPP

#include <cstddef> // for std::size_t

namespace boost {

// class template incomplete
//
// Treats an incomplete type as a value type.
//  * incomplete<T> stores value on the heap;
//  * incomplete<T,Size> stores value in stack storage of capacity Size.
template <typename T, std::size_t Size = -1> class incomplete;

// class template heap_incomplete
//
// Treats an incomplete type as a value type by dynamic allocation.
//
template <typename T> class heap_incomplete;

// stack_incomplete
//
// Treats an incomplete type as a value type by allocating it
// opaquely on the stack.
//
template <typename T, std::size_t Size> class stack_incomplete;

} // namespace boost

#endif // BOOST_INCOMPLETE_FWD_HPP
