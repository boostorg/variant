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

#include "boost/utility.hpp" // for checked_delete
#include "boost/mpl/if.hpp"

// The following are new/in-progress headers or fixes to existing headers:
#include "boost/incomplete_fwd.hpp"
#include "boost/aligned_storage.hpp"

namespace boost {

//////////////////////////////////////////////////////////////////////////
// class template incomplete
//
// Treats an incomplete type as a value type.
//  * incomplete<T> stores value on the heap;
//  * incomplete<T,Size> stores value in stack storage of capacity Size.
//
template <typename T, std::size_t Size>
class incomplete
{
    typedef typename mpl::if_c<
        Size == -1
      , heap_incomplete<T>
      , stack_incomplete<T,Size>
      >::type incomplete_t;

    incomplete_t value_;

public:
    incomplete()
      : value_()
    {
    }

    ~incomplete()
    {
    }

    incomplete(const incomplete& operand)
      : value_(operand.value_)
    {
    }

    incomplete& operator=(const incomplete& rhs)
    {
        incomplete temp(rhs);
        return swap(temp);
    }

    incomplete& swap(incomplete& operand)
    {
        value_.swap(operand.value_);
        return *this;
    }

    incomplete(const T& operand)
      : value_(operand)
    {
    }

    incomplete& operator=(const T& rhs)
    {
        incomplete temp(rhs);
        return swap(temp);
    }

    T& get() { return value_.get(); }
    const T& get() const { return value_.get(); }

    T* get_pointer() { return value_.get_pointer(); }
    const T* get_pointer() const { return value_.get_pointer(); }
};

// swap
//
// Swaps two incomplete<T> objects of the same type T (and Size, if applicable).
template <typename T, std::size_t Size>
void swap(incomplete<T,Size>& lhs, incomplete<T,Size>& rhs)
{
    lhs.swap(rhs);
}

//////////////////////////////////////////////////////////////////////////
// class template heap_incomplete
//
// Treats an incomplete type as a value type by dynamic allocation.
//
template <typename T>
class heap_incomplete
{
    T* p_;

public:
    heap_incomplete();
    ~heap_incomplete();

    heap_incomplete(const heap_incomplete& operand);

    heap_incomplete& operator=(const heap_incomplete& rhs)
    {
        heap_incomplete temp(rhs);
        return swap(temp);
    }

    heap_incomplete& swap(heap_incomplete& operand)
    {
        T* temp = operand.p_;
        operand.p_ = p_;
        p_ = temp;

        return *this;
    }

    heap_incomplete(const T& operand);

    heap_incomplete& operator=(const T& rhs)
    {
        heap_incomplete temp(rhs);
        return swap(temp);
    }

    T& get() { return *get_pointer(); }
    const T& get() const { return *get_pointer(); }

    T* get_pointer() { return p_; }
    const T* get_pointer() const { return p_; }
};

template <typename T>
heap_incomplete<T>::heap_incomplete()
    : p_(new T)
{
}

template <typename T>
heap_incomplete<T>::heap_incomplete(const heap_incomplete& operand)
    : p_(new T(operand.get()))
{
}

template <typename T>
heap_incomplete<T>::heap_incomplete(const T& operand)
    : p_(new T(operand))
{
}

template <typename T>
heap_incomplete<T>::~heap_incomplete()
{
    boost::checked_delete(p_);
}

// swap
//
// Swaps two heap_incomplete<T> objects of the same type T.
template <typename T, std::size_t Size>
void swap(heap_incomplete<T>& lhs, heap_incomplete<T>& rhs)
{
    lhs.swap(rhs);
}

//////////////////////////////////////////////////////////////////////////
// stack_incomplete
//
// Treats an incomplete type as a value type by allocating it
// opaquely on the stack.
//
template <typename T, std::size_t Size>
class stack_incomplete
{
    aligned_storage<Size> storage_;

public:
    stack_incomplete();
    ~stack_incomplete();

    stack_incomplete(const stack_incomplete&);
    stack_incomplete& operator=(const stack_incomplete&);
    void swap(stack_incomplete&);

    explicit stack_incomplete(const T&);
    stack_incomplete& operator=(const T&);

    T& get() { return storage_.template get_as<T>(); }
    const T& get() const { return storage_.template get_as<const T>(); }

    T* get_pointer() { return storage_.template get_pointer_as<T>(); }
    T* get_pointer() const { return storage_.template get_pointer_as<const T>(); }
};

template <typename T, std::size_t Size>
stack_incomplete<T, Size>::stack_incomplete()
{
    storage_.template construct_as<T>();
}

template <typename T, std::size_t Size>
stack_incomplete<T, Size>::~stack_incomplete()
{
    storage_.template destroy_as<T>();
}

template <typename T, std::size_t Size>
stack_incomplete<T, Size>::stack_incomplete(const stack_incomplete& operand)
{
    storage_.template construct_as<T>(operand.storage_);
}

template <typename T, std::size_t Size>
stack_incomplete<T,Size>& stack_incomplete<T, Size>::operator=(const stack_incomplete<T,Size>& operand)
{
    // Could use canonical copy-swap form, i.e.:
    // stack_incomplete temp(operand);
    // temp.swap(*this);
    // return *this;

    // But aligned_storage::assign_as is more efficient:
    storage_.template assign_as<T>(operand.storage_);
}

template <typename T, std::size_t Size>
void stack_incomplete<T, Size>::swap(stack_incomplete& operand)
{
    storage_.template swap_as<T>(operand.storage_);
}

template <typename T, std::size_t Size>
stack_incomplete<T, Size>::stack_incomplete(const T& operand)
{
    storage_.template construct_as<T>(operand);
}

template <typename T, std::size_t Size>
stack_incomplete<T,Size>& stack_incomplete<T, Size>::operator=(const T& operand)
{
    // Could use canonical copy-swap form, i.e.:
    // stack_incomplete temp(operand);
    // temp.swap(*this);
    // return *this;

    // But aligned_storage::assign_as is more efficient:
    storage_.template assign_as<T>(operand);
}

// swap
//
// Swaps two stack_incomplete<T,Size> objects of the same type T and Size.
template <typename T, std::size_t Size>
void swap(stack_incomplete<T,Size>& lhs, stack_incomplete<T,Size>& rhs)
{
    lhs.swap(rhs);
}

} // namespace boost

#endif // BOOST_INCOMPLETE_HPP