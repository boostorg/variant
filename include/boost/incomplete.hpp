//-----------------------------------------------------------------------------
// boost incomplete.hpp header file
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

#ifndef BOOST_INCOMPLETE_HPP
#define BOOST_INCOMPLETE_HPP

#include "boost/incomplete_fwd.hpp"

#include "boost/checked_delete.hpp"
#include "boost/mpl/if.hpp"

#include "boost/move_fwd.hpp"

namespace boost {

//////////////////////////////////////////////////////////////////////////
// class template incomplete
//
// Treats an incomplete type as a value type.
//
template <typename T>
class incomplete
{
    T* p_;

public:
    incomplete();
    ~incomplete();

    incomplete(const incomplete& operand);

    incomplete& operator=(const incomplete& rhs)
    {
        incomplete temp(rhs);
        return swap(temp);
    }

    incomplete& swap(incomplete& operand)
    {
        T* temp = operand.p_;
        operand.p_ = p_;
        p_ = temp;

        return *this;
    }

    incomplete(const T& operand);

    incomplete& operator=(const T& rhs)
    {
        incomplete temp(rhs);
        return swap(temp);
    }

    T& get() { return *get_pointer(); }
    const T& get() const { return *get_pointer(); }

    T* get_pointer() { return p_; }
    const T* get_pointer() const { return p_; }

private:
    enum DoMove { do_move };
    incomplete(DoMove, incomplete& source)
        : p_(source.p_)
    {
        // The following is allowed because move semantics do not
        // require the source to be useable after moving:
        source.p_ = 0;
    }

public:
    incomplete& move_to(void* dest)
    {
        return *(
              new(dest) incomplete(do_move, *this) // nothrow
            );
    }
};

template <typename T>
incomplete<T>::incomplete()
    : p_(new T)
{
}

template <typename T>
incomplete<T>::incomplete(const incomplete& operand)
    : p_(new T(operand.get()))
{
}

template <typename T>
incomplete<T>::incomplete(const T& operand)
    : p_(new T(operand))
{
}

template <typename T>
incomplete<T>::~incomplete()
{
    boost::checked_delete(p_);
}

// function template swap
//
// Swaps two incomplete<T> objects of the same type T.
//
template <typename T>
inline void swap(incomplete<T>& lhs, incomplete<T>& rhs)
{
    lhs.swap(rhs);
}

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

// class template move_traits specialization
//
// Enables use of move semantics with incomplete.
//
template <typename T>
struct move_traits< incomplete<T> >
{
    void move(void* dest, incomplete<T>& src)
    {
        src.move_to(dest);
    }
};

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION unsupported

} // namespace boost

#endif // BOOST_INCOMPLETE_HPP
