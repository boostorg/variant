//-----------------------------------------------------------------------------
// boost variant.hpp header file
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

#ifndef BOOST_VARIANT_HPP
#define BOOST_VARIANT_HPP

#include <cstddef> // for std::size_t
#include <new> // for placement new
#include <typeinfo> // for std::bad_cast, std::type_info
#include <utility> // for std::swap

#include "boost/config.hpp"
#include "boost/utility/addressof.hpp"
#include "boost/static_assert.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/enum.hpp"
#include "boost/preprocessor/enum_params.hpp"
#include "boost/preprocessor/enum_params_with_a_default.hpp"
#include "boost/preprocessor/enum_params_with_defaults.hpp"
#include "boost/preprocessor/repeat.hpp"
#include "boost/preprocessor/arithmetic/sub.hpp"
#include "boost/type_traits/add_const.hpp"
#include "boost/type_traits/alignment_of.hpp"
//#include "boost/type_traits/is_better_conversion.hpp"
#include "boost/type_traits/is_const.hpp"
#include "boost/type_traits/is_convertible.hpp"
#include "boost/type_traits/is_same.hpp"
#include "boost/type_traits/type_with_alignment.hpp"

#include "boost/mpl/advance.hpp"
#include "boost/mpl/apply.hpp"
#include "boost/mpl/apply_if.hpp"
#include "boost/mpl/assert_is_same.hpp"
#include "boost/mpl/begin_end.hpp"
#include "boost/mpl/bind.hpp"
#include "boost/mpl/bool_c.hpp"
#include "boost/mpl/contains.hpp"
#include "boost/mpl/distance.hpp"
#include "boost/mpl/empty.hpp"
#include "boost/mpl/erase.hpp"
//#include "boost/mpl/find.hpp"
#include "boost/mpl/comparison/equal_to.hpp"
#include "boost/mpl/identity.hpp"
#include "boost/mpl/if.hpp"
#include "boost/mpl/integral_c.hpp"
#include "boost/mpl/comparison/less.hpp"
#include "boost/mpl/list.hpp"
#include "boost/mpl/limits/list.hpp"
#include "boost/mpl/logical.hpp"
#include "boost/mpl/max_element.hpp"
#include "boost/mpl/size_of.hpp"
#include "boost/mpl/transform.hpp"
#include "boost/mpl/void.hpp"

// The following are new/in-progress headers or fixes to existing headers:
#include "boost/config/no_class_template_using_declarations.hpp"
#include "boost/detail/variant_workaround.hpp"
#include "boost/aligned_storage.hpp"
#include "boost/apply_visitor_fwd.hpp"
#include "boost/extract_fwd.hpp"
#include "boost/generic_visitor.hpp"
#include "boost/incomplete.hpp"
#include "boost/mpl/guarded_size.hpp"

namespace boost {
namespace mpl {

// Dummy implementation of mpl::is_sequence for now:
template <typename T>
struct is_sequence
{
    typedef false_c type;
     BOOST_STATIC_CONSTANT(bool, value = type::value);
};

} // namespace mpl
} // namespace boost

// BOOST_VARIANT_LIMIT_TYPES
//
// Implementation-defined preprocessor symbol describing the actual
// length of variant's pseudo-variadic template parameter list.
//
#define BOOST_VARIANT_LIMIT_TYPES \
    BOOST_MPL_LIMIT_LIST_SIZE

namespace boost {

namespace detail {
namespace variant {

// function template swap_impl
//
// Swaps using Koenig lookup but falls back to std::swap on non-conforming compilers.
//
template <typename T>
inline void swap_impl(T& lhs, T& rhs)
{
//    using std::swap;
    using namespace std;
    swap(lhs, rhs);
}

// metafunction max_value
//
// Applies ValueOp to the maximal element (as determined by ValueOp) of Sequence.
//
template <typename Sequence, typename ValueOp>
struct max_value
{
private:
    typedef typename mpl::max_element<
          typename mpl::transform<Sequence, ValueOp>::type
        >::type max_it;

public:
    typedef typename max_it::type
        type;
};

// class destroyer
//
// Generic Visitor that destroys the value it visits.
//
struct destroyer
    : generic_visitor<>
{
    template <typename T>
    void operator()(const T& operand) const
    {
        operand.~T();
    }
};

// class copy_into
//
// Generic Visitor that copies the value it visits into the given buffer.
//
class copy_into
    : generic_visitor<>
{
    void* storage_;

public:
    explicit copy_into(void* storage)
        : storage_(storage)
    {
    }

    template <typename T>
    void operator()(const T& operand) const
    {
        new(storage_) T(operand);
    }
};

// class swap_with
//
// Generic Visitor that swaps the value it visits with the given value.
//
struct swap_with
    : generic_visitor<>
{
private:
    void* toswap_;

public:
    explicit swap_with(void* toswap)
        : toswap_(toswap)
    {
    }

    template <typename T>
    void operator()(T& operand) const
    {
        swap_impl(operand, *reinterpret_cast<T*>(toswap_));
    }
};

// class reflect
//
// Generic Visitor that performs a typeid on the value it visits.
//
struct reflect
    : generic_visitor<const std::type_info&>
{
    template <typename T>
    const std::type_info& operator()(const T&)
    {
        return typeid(T);
    }
};

// class template invoke_visitor
//
// Invokes the given visitor using:
//  * for raw visits where the given value is a
//    boost::incomplete, the given value's held value.
//  * for all other visits, the given value itself.
//
template <typename Visitor>
struct invoke_visitor
{
public:
    typedef typename Visitor::result_type
        result_type;

private:
    Visitor& visitor_;

public:
    explicit invoke_visitor(Visitor& visitor)
        : visitor_(visitor)
    {
    }

#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

public:
    template <typename T>
    result_type operator()(incomplete<T>& operand)
    {
        return visitor_(operand.get());
    }

    template <typename T>
    result_type operator()(const incomplete<T>& operand)
    {
        return visitor_(operand.get());
    }

    template <typename T>
    result_type operator()(T& operand)
    {
        return visitor_(operand);
    }

#else// defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

private:
    template <typename T>
    result_type execute_impl(incomplete<T>& operand, long)
    {
        return visitor_(operand.get());
    }

    template <typename T>
    result_type execute_impl(const incomplete<T>& operand, long)
    {
        return visitor_(operand.get());
    }

    template <typename T>
    result_type execute_impl(T& operand, int)
    {
        return visitor_(operand);
    }

public:
    template <typename T>
    result_type operator()(T& operand)
    {
        return execute_impl(operand, 1L);
    }

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING workaround

};

//////////////////////////////////////////////////////////////////////////
// BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround
//
// Needed to work around compilers that don't support using-declarations
// in class templates. (See the variant::initializer workarounds below.)
//
#if !defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

// metafunction make_variant_list
//
// Provides a MPL-compatible sequence with the specified non-void types
// as arguments.
template < BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T) >
struct make_variant_list
{
	typedef typename mpl::list<
		  BOOST_PP_ENUM_PARAMS(
			  BOOST_VARIANT_LIMIT_TYPES
			, T
			)
		>::type type;
};

#else// defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

// class template convert_void
// 
// Provides the mechanism by which voidNN types are converted to
// mpl::void_ (and thus can be passed to mpl::list).
//
template <typename T> struct convert_void
{
    typedef T type;
};

// tags voidNN -- NN defined on [0, BOOST_VARIANT_LIMIT_TYPES)
//
// Defines void types that are each unique and specializations of
// convert_void that yields mpl::void_ for each voidNN type.
//
#define BOOST_VARIANT_DETAIL_DEFINE_VOID_N(N,_)            \
    struct BOOST_PP_CAT(void,N);                           \
                                                           \
    template <> struct convert_void<BOOST_PP_CAT(void,N)>  \
    {                                                      \
        typedef mpl::void_ type;                           \
    };

BOOST_PP_REPEAT(
      BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
    , BOOST_VARIANT_DETAIL_DEFINE_VOID_N
    , _
    )
#undef BOOST_VARIANT_DETAIL_DEFINE_VOID_N

// metafunction make_variant_list
//
// Provides a MPL-compatible sequence with the specified non-void types
// as arguments.
//
template < BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T) >
struct make_variant_list
{
	// [Define a macro to convert any voidNN tags to mpl::void...]
#   define BOOST_VARIANT_DETAIL_CONVERT_VOID(N,_)   \
        typename detail::variant::convert_void<BOOST_PP_CAT(T,N)>::type

	// [...so that the specified types can be passed to mpl::list...
	typedef typename mpl::list< 
		  BOOST_PP_ENUM(
			  BOOST_VARIANT_LIMIT_TYPES
			, BOOST_VARIANT_DETAIL_CONVERT_VOID
			, _
			)
		>::type type;

	// [...and, finally, the conversion macro can be undefined:]
#   undef BOOST_VARIANT_DETAIL_CONVERT_VOID
};

#endif // BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround

} // namespace variant
} // namespace detail

//////////////////////////////////////////////////////////////////////////
// class template variant (concept inspired by Andrei Alexandrescu)
//
// Efficient, type-safe bounded discriminated union.
//
// Preconditions:
//  - Two or more types must be specified.
//  - Each type must be unique.
//
// Proper syntax:
//   variant<A,B,...>  (where A,B,... are not type-sequences)
// or
//   variant<types>  (where types is a type-sequence with size > 1)
//
template <
  typename A
, typename B = mpl::void_

#if !defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

, BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
	  BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
	, typename T
	, mpl::void_
	)

#else// defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

, BOOST_PP_ENUM_PARAMS_WITH_DEFAULTS(
      BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
    , typename T
    , detail::variant::void//NN
    )

#endif // BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround

>
class variant

#if defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
    : public detail::variant_workaround_base
#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION workaround

{
/*    typedef mpl::integral_c<unsigned, 2>
        min_list_size;

    // NOTE TO USER :
    // Compile error here indicates that variant's variadic
    // template parameter list was used inappropriately.
    BOOST_STATIC_ASSERT((
          mpl::logical_or< // (is_sequence<A> && size<A> >= 2) || B != void_
              mpl::logical_and<
                  mpl::is_sequence<A>
                , mpl::equal_to<
                      mpl::guarded_size<A, min_list_size>
                    , min_list_size
                    >
                >
            , mpl::logical_not<
                  is_same<B, mpl::void_>
                >
            >::type::value
        ));
*/
public:
    typedef typename mpl::apply_if<
          mpl::is_sequence<A>
        , mpl::identity<A>
		, detail::variant::make_variant_list<
			  A
			, B
			, BOOST_PP_ENUM_PARAMS(
				  BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
				, T
				)
			>
		>::type types;

private:
    BOOST_STATIC_CONSTANT(
          std::size_t
        , max_size = (detail::variant::max_value< types, mpl::size_of<_> >::type::value)
        );
    BOOST_STATIC_CONSTANT(
          std::size_t
        , max_alignment = (detail::variant::max_value< types, alignment_of<_> >::type::value)
        );
    typedef aligned_storage<max_size, max_alignment>
        aligned_storage_t;

    // which_ on:
    // * [0,  size<types>) indicates storage1
    // * [-size<types>, 0) indicates storage2
    // if which_ >= 0:
    // * then which() -> which_
    // * else which() -> -(which_ + 1)
    int which_;
    aligned_storage_t storage1_;
    aligned_storage_t storage2_;

    bool using_storage1() const
    {
        return which_ >= 0;
    }

    aligned_storage_t& active_storage()
    {
        return using_storage1() ? storage1_ : storage2_;
    }

    const aligned_storage_t& active_storage() const
    {
        return const_cast<variant* const>(this)->active_storage();
    }

    void assign(const variant& operand)
    {
        // Copy the content of operand into *this's inactive storage...
        operand.raw_apply_visitor(
              detail::variant::copy_into(
                  ( using_storage1() ? storage2_ : storage1_ ).address()
                )
            );

        // ...destroy the active storage of *this...
        raw_apply_visitor(detail::variant::destroyer());

        // ...and if *this _was_ using storage1...
        if (using_storage1())
            // ...then activate storage2:
            which_ = -(operand.which() + 1);
        else
            // ...otherwise, activate storage1:
            which_ = operand.which();
    }

public:
    int which() const
    {
        // If using storage1...
        if (using_storage1())
            // ...return which_ directly:
            return which_;

        // Otherwise, return adjusted which_:
        return -(which_ + 1);
    }

private:

// [On compilers where using declarations in class templates can correctly avoid name hiding...]
#if !defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

    // [...use an optimal converting initializer based on the variant typelist:]

    struct initializer_root
    {
    private:
        enum NotCallable { not_callable };

    public:
        static void initialize(NotCallable) { }
    };

    struct make_initializer_node
    {
        template <typename Base, typename Iterator>
        struct apply
        {
        private:
            struct initializer_node
                : Base
            {
                using Base::initialize;
                
                typedef typename Iterator::type
                    T;

                static int initialize(void* dest, const T& operand)
                {
                    new(dest) T(operand);

                    BOOST_STATIC_CONSTANT(
                          std::size_t
                        , idx = (mpl::distance<typename mpl::begin<types>::type, Iterator>::value)
                        );
                    return idx;
                }
            };

        public:
            typedef initializer_node
                type;
        };
    };

    typedef typename mpl::iter_fold<
          types
        , initializer_root
        , make_initializer_node
        >::type initializer;

#else // defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

    // [...otherwise, use a hackish workaround based on variant's template parameters:]

    struct preprocessor_list_initializer
    {
        static int initialize(void* dest, const A& operand)
        {
            new(dest) A(operand);
            return 0;
        }

        static int initialize(void* dest, const B& operand)
        {
            new(dest) B(operand);
            return 1;
        }

        #define BOOST_VARIANT_INITIALIZE_FUNCTION(N,_)         \
            static int initialize(                             \
                  void* dest                                   \
                , const BOOST_PP_CAT(T,N)& operand             \
                )                                              \
            {                                                  \
                typedef BOOST_PP_CAT(T,N) T;                   \
                BOOST_STATIC_CONSTANT(int, which = (N) + 2);   \
                                                               \
                new(dest) T(operand);                          \
                return which;                                  \
            }

        BOOST_PP_REPEAT(
              BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
            , BOOST_VARIANT_INITIALIZE_FUNCTION
            , _
            )
        #undef BOOST_VARIANT_INITIALIZE_FUNCTION
    };
/*
    struct mpl_list_initializer
    {
    private:
        template <typename T, typename Iterator>
        static int initialize_impl(
              void* dest
            , const T& operand
            , mpl::false_c// converting_initialization_flag
            , mpl::identity<Iterator>
            )
        {
            // [Assert the given type matches the type referenced by the specified iterator:]
            BOOST_MPL_ASSERT_IS_SAME(typename Iterator::type, T);

            // Initialize directly...
            new(dest) T(operand);

            // ...and return the type's index:
            return mpl::distance<typename mpl::begin<types>::type, Iterator>::value;
        }

        template <typename T, typename I>
        static int initialize_impl(
              void* dest
            , const T& operand
            , mpl::true_c// converting_initialization_flag
            , I
            )
        {
            using namespace mpl::placeholder;

            // [Bind is_better_conversion's "to" argument:]
            typedef mpl::bind3< // is_better_conversion< T,_1,_2 >
                  is_better_conversion<_,_,_>
                , T
                , _2
                , _1
                > conversion_compare_to_T;

            // [Find the best conversion to T from among the types:]
            typedef typename mpl::max_element<types, conversion_compare_to_T>::type
                best_conversion_it;
            typedef typename best_conversion_it::type
                best_conversion;

            // NOTE TO USER :
            // Compile error here indicates the given type cannot be converted to 
            // any of the variant's bounded types because such a conversion does
            // not exist.
            BOOST_STATIC_ASSERT((is_convertible<T, best_conversion>::value));

            // [Remove the _best_ conversion from the list...]
            typedef typename mpl::erase<types, best_conversion_it>::type
                types_without_best;

            // [...in order to find the next-best conversion:]
            typedef typename mpl::max_element<types_without_best, conversion_compare_to_T>::type
                next_best_conversion_it;

            // NOTE TO USER :
            // Compile error here indicates the given type cannot be converted to
            // any of the variant's bounded types because such a conversion would
            // be ambiguous.
            BOOST_STATIC_ASSERT((
                  is_better_conversion<
                      T
                    , typename best_conversion_it::type
                    , typename next_best_conversion_it::type
                    >::value
                ));

            // Finally, convert the operand to a bounded (destination) type...
            new(dest) best_conversion(operand);

            // ...and return the destination type's index:
            return mpl::distance<typename mpl::begin<types>::type, best_conversion_it>::value;
        }

    public:
        template <typename T>
        static int initialize(void* dest, const T& operand)
        {
            // [Attempt to find the given type in the variant's bounds...]
            typedef typename mpl::find<types, T>::type
                found_it;

            // [...because converting-initialization is needed if it could not be found:]
            typedef mpl::bool_c<
                  is_same<found_it, typename mpl::end<types>::type>::value
                > converting_initialization_flag;

            // Perform the appropriate initialization depending on whether the type was found:
            return initialize_impl(
                  dest
                , operand
                , converting_initialization_flag()
                , mpl::identity<found_it>()
                );
        }
    };

    typedef typename mpl::if_<
          mpl::is_sequence<A>
        , mpl_list_initializer
        , preprocessor_list_initializer
        >::type initializer;
*/
    typedef preprocessor_list_initializer
        initializer;

#endif // BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround

public:
    variant()
    {
        // NOTE TO USER :
        // Compile error from here indicates that the first bound
        // type is default-constructible, and so variant cannot
        // support its own default-construction
        storage1_.template construct_as<A>();
        which_ = 0;
    }

    variant(const variant& operand)
    {
        operand.raw_apply_visitor(detail::variant::copy_into(storage1_.address()));
        which_ = operand.which();
    }

private:
    class convert_copy_into
        : generic_visitor<int>
    {
        void* storage_;

    public:
        explicit convert_copy_into(void* storage)
            : storage_(storage)
        {
        }

        template <typename T>
        int operator()(const T& operand) const
        {
            // NOTE TO USER :
            // Compile error here indicates one of the source variant's types 
            // cannot be unambiguously converted to the destination variant's
            // types (or that no conversion exists).
            return initializer::initialize(storage_, operand);
        }
    };

#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

public:
    template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename U)>
    variant(const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, U)>& operand)
    {
        // Attempt a converting copy into *this's storage:
        which_ = operand.raw_apply_visitor(
              convert_copy_into(storage1_.address())
            );
    }

    template <typename T>
    variant(const T& operand)
    {
        // NOTE TO USER :
        // Compile error here indicates that the given type is not 
        // unambiguously convertible to one of the variant's types
        // (or that no conversion exists).
        which_ = initializer::initialize(
              storage1_.address()
            , operand
            );
    }

#else// defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

private:
    template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename U)>
    void constructor_simulated_partial_ordering(
          const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, U)>& operand
        , long)
    {
        // Attempt a converting copy into *this's storage:
        which_ = operand.raw_apply_visitor(
              convert_copy_into(storage1_.address())
            );
    }

    template <typename T>
    void constructor_simulated_partial_ordering(const T& operand, int)
    {
        // NOTE TO USER :
        // Compile error here indicates that the given type is not 
        // unambiguously convertible to one of the variant's types
        // (or that no conversion exists).
        which_ = initializer::initialize(
              storage1_.address()
            , operand
            );
    }

public:
    template <typename T>
    variant(const T& operand)
    {
        constructor_simulated_partial_ordering(operand, 1L);
    }

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING workaround

public:
    ~variant()
    {
        // Visit *this's contents with a 'destroyer' visitor:
        raw_apply_visitor(detail::variant::destroyer());
    }

public: // modifiers
    template <typename T>
    variant& operator=(const T& rhs)
    {
        assign(rhs);
        return *this;
    }

    variant& operator=(const variant& rhs)
    {
        assign(rhs);
        return *this;
    }

    variant& swap(variant& operand)
    {
        // If the types are the same...
        if (which_ == operand.which_)
        {
            // ...then swap the values directly:
            raw_apply_visitor(detail::variant::swapper(operand.active_storage().address()));
            return *this;
        }

        // Otherwise, perform normal swap:
        variant temp(operand);
        operand.assign(*this);
        assign(temp);

        return *this;
    }

public: // queries
    bool empty() const
    {
        return false;
    }

    const std::type_info& type() const
    {
        return this->apply_visitor(detail::variant::reflect());
    }

private:
    template <typename Which, typename Iterator, typename LastIterator, typename Variant, typename Visitor>
    static
        typename Visitor::result_type
    apply_visitor_impl(
          const int var_which // [const-ness may aid in optimization by compiler]
        , Variant& var
        , Visitor& visitor
        , mpl::false_c// is_last
        )
    {
        typedef typename mpl::next<Which>::type next_which;
        typedef typename mpl::next<Iterator>::type next_iter;
        typedef mpl::bool_c<is_same<next_iter, LastIterator>::value> next_is_last;
        typedef typename mpl::apply_if<
              is_const<Variant>
            , add_const<typename Iterator::type>
            , mpl::identity<typename Iterator::type>
            >::type T;

        if (var_which == Which::value)
        {
            return visitor(var.active_storage().template get_as<T>());
        }

        return apply_visitor_impl<next_which, next_iter, LastIterator>(
              var_which
            , var
            , visitor
            , next_is_last()
            );
    }

    template <typename W, typename I, typename LI, typename Variant, typename Visitor>
    static
        typename Visitor::result_type
    apply_visitor_impl(
          const int
        , Variant&
        , Visitor&
        , mpl::true_c// is_last
        )
    {
        // / This is never called at runtime: a visitor must handle at \
        // | least one of the variant's types. Throw to circumvent the |
        // \ compile-time requirement that a value is returned:        /
        throw;
    }

public:
    //////////////////////////////////////////////////////////////////////
    // WARNING TO USER :
    // The following functions are not part of the public interface,
    // despite their public access (which may change in the future).
    //
    template <typename Visitor>
        typename Visitor::result_type
    raw_apply_visitor(Visitor visitor)
    {
        return apply_visitor_impl<
              mpl::integral_c<unsigned long, 0>
            , typename mpl::begin<types>::type
            , typename mpl::end<types>::type
            >(which(), *this, visitor, mpl::false_c());
    }

    template <typename Visitor>
        typename Visitor::result_type
    raw_apply_visitor(Visitor visitor) const
    {
        return apply_visitor_impl<
              mpl::integral_c<unsigned long, 0>
            , typename mpl::begin<types>::type
            , typename mpl::end<types>::type
            >(which(), *this, visitor, mpl::false_c());
    }

    template <typename Visitor>
        typename Visitor::result_type
    apply_visitor(Visitor visitor)
    {
        detail::variant::invoke_visitor<Visitor> invoker(visitor);
        return raw_apply_visitor(invoker);
    }

    template <typename Visitor>
        typename Visitor::result_type
    apply_visitor(Visitor visitor) const
    {
        detail::variant::invoke_visitor<Visitor> invoker(visitor);
        return raw_apply_visitor(invoker);
    }
};

//////////////////////////////////////////////////////////////////////////
// class template is_variant
//
// Metafunction to determine if specified type is a variant.
//

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template <typename T>
struct is_variant
{
    typedef mpl::bool_c<false> type;
    BOOST_STATIC_CONSTANT(bool, value = type::value);
};

template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
struct is_variant<
  boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>
>
{
    typedef mpl::bool_c<true> type;
    BOOST_STATIC_CONSTANT(bool, value = type::value);
};

#else// defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

// [is_variant is defined in the variant workaround header.]

#endif // !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

//////////////////////////////////////////////////////////////////////////
// function overload swap
//
// Swaps two variants of the same type (i.e., identical bounded type set).
//

template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
void swap(
      boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& lhs
    , boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& rhs
    )
{
    lhs.swap(rhs);
}

//////////////////////////////////////////////////////////////////////////
// class template apply_visitor_traits<variant> specialization
//
// Enables the apply_visitor facility for variants.
//

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
struct apply_visitor_traits<
  boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>
>
{
private:
    typedef boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>
        variant_t;

public:
    template <typename Visitor>
    static
        typename Visitor::result_type
    execute(Visitor& visitor, variant_t& operand)
    {
        return operand.apply_visitor(visitor);
    }

    template <typename Visitor>
    static
        typename Visitor::result_type
    execute(Visitor& visitor, const variant_t& operand)
    {
        return operand.apply_visitor(visitor);
    }
};

#else// defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

namespace detail {

template <typename Visitor, BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
    typename Visitor::result_type
variant_apply_visitor(
      Visitor& visitor
    , boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& operand
    )
{
    return operand.apply_visitor(visitor);
}

template <typename Visitor, BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
    typename Visitor::result_type
variant_apply_visitor(
      Visitor& visitor
    , const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& operand
    )
{
    return operand.apply_visitor(visitor);
}

} // namespace detail

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION workaround

//////////////////////////////////////////////////////////////////////////
// class template extract_traits<variant> specialization
//
// Enables the extract<T> facility for variants.
//

namespace detail {
namespace variant {

template <typename T>
struct caster
    : generic_visitor<T*>
{
    template <typename U>
    T* operator()(U&) const
    {
        return static_cast<T*>(0);
    }

    T* operator()(T& operand) const
    {
        return boost::addressof(operand);
    }
};

} // namespace variant
} // namespace detail

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
struct extract_traits<
  boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>
>
{
private:
    typedef boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>
        variant_t;

public:
    template <typename T>
    static T* execute(variant_t& operand)
    {
        return operand.apply_visitor(detail::variant::caster<T>());
    }

    template <typename T>
    static T* execute(const variant_t& operand)
    {
        return operand.apply_visitor(detail::variant::caster<T>());
    }
};

#else// defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

namespace detail {

template <typename T, BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
T* variant_extract_pointer(
      boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& operand
    )
{
    return operand.apply_visitor(detail::variant::caster<T>());
}

template <typename T, BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T)>
T* variant_extract_pointer(
      const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T)>& operand
    )
{
    return operand.apply_visitor(detail::variant::caster<T>());
}

} // namespace detail

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION workaround

} // namespace boost

#endif // BOOST_VARIANT_HPP
