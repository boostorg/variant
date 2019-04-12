//-----------------------------------------------------------------------------
// boost variant/recursive_wrapper.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
// Eric Friedman, Itay Maman
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_VARIANT_RECURSIVE_WRAPPER_HPP
#define BOOST_VARIANT_RECURSIVE_WRAPPER_HPP

#include <boost/variant/recursive_wrapper_fwd.hpp>
#include <boost/variant/detail/move.hpp>
#include <boost/checked_delete.hpp>

namespace boost {

//////////////////////////////////////////////////////////////////////////
// class template recursive_wrapper
//
// See docs and recursive_wrapper_fwd.hpp for more information.
//

template <typename T>
class recursive_wrapper
{
public: // typedefs

    typedef T type;

private: // representation

    T* p_;

public: // structors

    ~recursive_wrapper();
    recursive_wrapper();

    recursive_wrapper(const recursive_wrapper& operand);
    recursive_wrapper(const T& operand);

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES 
#ifndef BOOST_VARIANT_NO_RECURSIVE_WRAPPER_POINTER_STEALING
    recursive_wrapper(recursive_wrapper&& operand) BOOST_NOEXCEPT;
#else
    recursive_wrapper(recursive_wrapper&& operand);
#endif
    recursive_wrapper(T&& operand);
#endif

private: // helpers, for modifiers (below)

    void assign(const T& rhs);

public: // modifiers

    recursive_wrapper& operator=(const recursive_wrapper& rhs)
    {
        assign( rhs.get() );
        return *this;
    }

    recursive_wrapper& operator=(const T& rhs)
    {
        assign( rhs );
        return *this;
    }

    void swap(recursive_wrapper& operand) BOOST_NOEXCEPT
    {
        T* temp = operand.p_;
        operand.p_ = p_;
        p_ = temp;
    }

    
#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES 
    recursive_wrapper& operator=(recursive_wrapper&& rhs) BOOST_NOEXCEPT
    {
        swap(rhs);
        return *this;
    }

    recursive_wrapper& operator=(T&& rhs)
    {
        get() = detail::variant::move(rhs);
        return *this;
    }
#endif

public: // queries

#ifndef BOOST_VARIANT_NO_RECURSIVE_WRAPPER_POINTER_STEALING
    bool empty() const BOOST_NOEXCEPT { return get_pointer() == NULL; }
#else
    bool empty() const BOOST_NOEXCEPT { return false; }
#endif

    // Expects: `!empty()`.
    T& get() BOOST_NOEXCEPT { BOOST_ASSERT(!empty()); return *get_pointer(); }
    const T& get() const BOOST_NOEXCEPT { BOOST_ASSERT(!empty()); return *get_pointer(); }

    T* get_pointer() BOOST_NOEXCEPT { return p_; }
    const T* get_pointer() const BOOST_NOEXCEPT { return p_; }

};

template <typename T>
recursive_wrapper<T>::~recursive_wrapper()
{
    boost::checked_delete(p_);
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper()
    : p_(new T)
{
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(const recursive_wrapper& operand)
    : p_(new T( operand.get() ))
{
}

template <typename T>
recursive_wrapper<T>::recursive_wrapper(const T& operand)
    : p_(new T(operand))
{
}

#ifndef BOOST_NO_CXX11_RVALUE_REFERENCES 
#ifndef BOOST_VARIANT_NO_RECURSIVE_WRAPPER_POINTER_STEALING
template <typename T>
recursive_wrapper<T>::recursive_wrapper(recursive_wrapper&& operand) BOOST_NOEXCEPT
    : p_(operand.p_)
{
    operand.p_ = static_cast<T*>(NULL);
}
#else
template <typename T>
recursive_wrapper<T>::recursive_wrapper(recursive_wrapper&& operand)
    : p_(new T( detail::variant::move(operand.get()) ))
{
}
#endif

template <typename T>
recursive_wrapper<T>::recursive_wrapper(T&& operand)
    : p_(new T( detail::variant::move(operand) ))
{
}
#endif

template <typename T>
void recursive_wrapper<T>::assign(const T& rhs)
{
    this->get() = rhs;
}

// function template swap
//
// Swaps two recursive_wrapper<T> objects of the same type T.
//
template <typename T>
inline void swap(recursive_wrapper<T>& lhs, recursive_wrapper<T>& rhs) BOOST_NOEXCEPT
{
    lhs.swap(rhs);
}

} // namespace boost

#endif // BOOST_VARIANT_RECURSIVE_WRAPPER_HPP
