//-----------------------------------------------------------------------------
// boost variant.hpp header file
// See http://www.boost.org for updates, documentation, and revision history.
//-----------------------------------------------------------------------------
//
// Copyright (c) 2002-2003
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

// MPL as shipped in 1.29 has bug with preprocessed headers, so define following:
#include "boost/version.hpp"
#if BOOST_VERSION <= 102900
#   define BOOST_MPL_AUX_CONFIG_USE_PREPROCESSED_HPP_INCLUDED
#endif

#include <cstddef> // for std::size_t
#include <new> // for placement new
#include <typeinfo> // for std::type_info

#include "boost/config.hpp"
#include "boost/compressed_pair.hpp"
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
#include "boost/type_traits/is_const.hpp"
#include "boost/type_traits/is_same.hpp"

#include "boost/mpl/apply_if.hpp"
#include "boost/mpl/begin_end.hpp"
#include "boost/mpl/bool_c.hpp"
#include "boost/mpl/distance.hpp"
#include "boost/mpl/empty.hpp"
#include "boost/mpl/comparison/equal_to.hpp"
#include "boost/mpl/identity.hpp"
#include "boost/mpl/integral_c.hpp"
#include "boost/mpl/iter_fold.hpp"
#include "boost/mpl/list.hpp"
#include "boost/mpl/limits/list.hpp"
#include "boost/mpl/logical.hpp"
#include "boost/mpl/max_element.hpp"
#include "boost/mpl/remove_if.hpp"
#include "boost/mpl/sizeof.hpp"
#include "boost/mpl/transform.hpp"
#include "boost/mpl/void.hpp"

// The following used in experimental mpl_list_initializer (below):
//#include "boost/type_traits/is_better_conversion.hpp"
//#include "boost/type_traits/is_convertible.hpp"
//#include "boost/mpl/assert_is_same.hpp"
//#include "boost/mpl/bind.hpp"
//#include "boost/mpl/erase.hpp"
//#include "boost/mpl/find.hpp"
//#include "boost/mpl/if.hpp"

// The following are new/in-progress headers or fixes to existing headers:
#include "boost/config/no_class_template_using_declarations.hpp"
#include "boost/aligned_storage.hpp"
#include "boost/extractable.hpp"
#include "boost/incomplete_fwd.hpp"
#include "boost/move.hpp"
#include "boost/static_visitable.hpp"
#include "boost/static_visitor.hpp"
#include "boost/mpl/guarded_size.hpp"
#include "boost/type_traits/has_nothrow_move.hpp"

// MPL as shipped in 1.29 does not provide is_sequence predicate, so define following:
#if BOOST_VERSION > 102900

#   if defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)
#      define BOOST_VARIANT_NO_TYPE_SEQUENCE_SUPPORT
#   endif
#
#   include "boost/mpl/is_sequence.hpp"

#else

#define BOOST_VARIANT_NO_TYPE_SEQUENCE_SUPPORT

namespace boost {
namespace mpl {

template <typename T>
struct is_sequence
{
    typedef false_c type;
    BOOST_STATIC_CONSTANT(bool, value = type::value);
};

} // namespace mpl
} // namespace boost

#endif

//////////////////////////////////////////////////////////////////////////
// BOOST_VARIANT_LIMIT_TYPES
//
// Implementation-defined preprocessor symbol describing the actual
// length of variant's pseudo-variadic template parameter list.
//
#define BOOST_VARIANT_LIMIT_TYPES \
    BOOST_MPL_LIMIT_LIST_SIZE

namespace boost {

namespace detail { namespace variant {

//////////////////////////////////////////////////////////////////////////
// (detail) metafunction max_value
//
// Applies ValueOp to the maximal element (as determined by ValueOp) of Sequence.
//
template <typename Sequence, typename ValueOp>
struct max_value
{
private: // helpers, for metafunction result (below)

    typedef typename mpl::max_element<
          typename mpl::transform<Sequence, ValueOp>::type
        >::type max_it;

public: // metafunction result

    typedef typename max_it::type
        type;

};

//////////////////////////////////////////////////////////////////////////
// (detail) metafunction make_storage
//
// Provides an aligned storage type capable of holding any of the types
// specified in the given type-sequence.
//
template <typename Types>
struct make_storage
{
private: // helpers, for metafunction result (below)

    BOOST_STATIC_CONSTANT(
          std::size_t
        , max_size = (max_value< Types, mpl::sizeof_<mpl::_1> >::type::value)
        );
    BOOST_STATIC_CONSTANT(
          std::size_t
        , max_alignment = (max_value< Types, alignment_of<mpl::_1> >::type::value)
        );

public: // metafunction result

    typedef aligned_storage<max_size, max_alignment>
        type;

};

//////////////////////////////////////////////////////////////////////////
// (detail) class null_storage
//
// Simulates aligned_storage's interface, but with nothing underneath.
//
struct null_storage
{
public: // queries

    void* address()
    {
        return 0;
    }

    const void* address() const
    {
        return 0;
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class destroyer
//
// Generic static visitor that destroys the value it visits.
//
struct destroyer
    : public static_visitor<>
{
public: // visitor interfaces

    template <typename T>
    void operator()(const T& operand) const
    {
        operand.~T();
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class copy_into
//
// Generic static visitor that copies the value it visits into the given buffer.
//
class copy_into
    : public static_visitor<>
{
private: // representation

    void* storage_;

public: // structors

    explicit copy_into(void* storage)
        : storage_(storage)
    {
    }

public: // visitor interfaces

    template <typename T>
    void operator()(const T& operand) const
    {
        new(storage_) T(operand);
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class move_into
//
// Generic static visitor that moves the value it visits into the given buffer.
//
class move_into
    : public static_visitor<>
{
private: // representation

    void* storage_;

public: // structors

    explicit move_into(void* storage)
        : storage_(storage)
    {
    }

public: // visitor interfaces

    template <typename T>
    void operator()(T& operand) const
    {
        new(storage_) T( move(operand) );
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class swap_with
//
// Generic static visitor that swaps the value it visits with the given value.
//
struct swap_with
    : public static_visitor<>
{
private: // representation

    void* toswap_;

public: // structors

    explicit swap_with(void* toswap)
        : toswap_(toswap)
    {
    }

public: // visitor interfaces

    template <typename T>
    void operator()(T& operand) const
    {
        boost::move_swap(operand, *static_cast<T*>(toswap_));
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class reflect
//
// Generic static visitor that performs a typeid on the value it visits.
//
struct reflect
    : public static_visitor<const std::type_info&>
{
public: // visitor interfaces

    template <typename T>
    const std::type_info& operator()(const T&)
    {
        return typeid(T);
    }

};

//////////////////////////////////////////////////////////////////////////
// (detail) class cast_to
//
// Generic static visitor that: if the value is of the specified type,
// returns a pointer to the value it visits; else a null pointer.
//
template <typename T>
struct cast_to
    : public static_visitor<T*>
{
public: // visitor interfaces

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

//////////////////////////////////////////////////////////////////////////
// (detail) class template invoke_visitor
//
// Generic static visitor that invokes the given visitor using:
//  * for raw visits where the given value is a
//    boost::incomplete, the given value's held value.
//  * for all other visits, the given value itself.
//
template <typename Visitor>
class invoke_visitor
    : public static_visitor< typename Visitor::result_type >
{
private: // representation

    Visitor& visitor_;

public: // typedefs

    typedef typename Visitor::result_type
        result_type;

public: // structors

    explicit invoke_visitor(Visitor& visitor)
        : visitor_(visitor)
    {
    }

#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

public: // visitor interfaces

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

private: // helpers, for visitor interfaces (below)

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

public: // visitor interfaces

    template <typename T>
    result_type operator()(T& operand)
    {
        return execute_impl(operand, 1L);
    }

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING workaround

};

//////////////////////////////////////////////////////////////////////////
// (detail) class void_ and class template convert_void
// 
// Provides the mechanism by which void(NN) types are converted to
// mpl::void_ (and thus can be passed to mpl::list).
//
// Rationale: This is particularly needed for the using-declarations
// workaround (below), but also to avoid associating mpl namespace with
// variant in argument dependent lookups (which used to happen because of
// defaulting of template parameters to mpl::void_).
//

struct void_;

template <typename T>
struct convert_void
{
    typedef T type;
};

template <>
struct convert_void< void_ >
{
    typedef mpl::void_ type;
};

//////////////////////////////////////////////////////////////////////////
// (detail) metafunction make_variant_list
//
// Provides a MPL-compatible sequence with the specified non-void types
// as arguments.
//
// Rationale: see class template convert_void (above) and using-
// declaration workaround (below).
//
template < BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T) >
struct make_variant_list
{
public: // metafunction result

    // [Define a macro to convert any void(NN) tags to mpl::void...]
#   define BOOST_VARIANT_DETAIL_CONVERT_VOID(z, N,_)   \
        typename detail::variant::convert_void<BOOST_PP_CAT(T,N)>::type

    // [...so that the specified types can be passed to mpl::list...]
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

//////////////////////////////////////////////////////////////////////////
// BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround
//
// Needed to work around compilers that don't support using-declarations
// in class templates. (See the variant::initializer workarounds below.)
//

#if defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

// (detail) tags voidNN -- NN defined on [0, BOOST_VARIANT_LIMIT_TYPES - 2)
//
// Defines void types that are each unique and specializations of
// convert_void that yields mpl::void_ for each voidNN type.
//

#define BOOST_VARIANT_DETAIL_DEFINE_VOID_N(z,N,_)          \
    struct BOOST_PP_CAT(void,N);                           \
                                                           \
    template <>                                            \
    struct convert_void< BOOST_PP_CAT(void,N) >            \
    {                                                      \
        typedef mpl::void_ type;                           \
    };                                                     \
    /**/

BOOST_PP_REPEAT(
      BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
    , BOOST_VARIANT_DETAIL_DEFINE_VOID_N
    , _
    )

#undef BOOST_VARIANT_DETAIL_DEFINE_VOID_N

#endif // BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS workaround

// (detail) class template variant_base
//
// Derives the base classes of variant. Avoids having to repeat template
// parameters for variant when passing off to base classes.
//
template <typename Variant>
class variant_base
    : public ::boost::static_visitable<Variant>
    , public ::boost::extractable<Variant>
    , public ::boost::moveable<Variant>
{
};

}} // namespace detail::variant

//////////////////////////////////////////////////////////////////////////
// class template variant (concept inspired by Andrei Alexandrescu)
//
// Efficient, type-safe bounded discriminated union.
//
// Preconditions:
//  - Two or more types must be specified.
//  - Each type must be unique.
//
// Proper declaration form:
//   variant<A,B,...>  (where A,B,... are NOT type-sequences)
// or
//   variant<types>    (where types is a type-sequence with size > 1)
//
template <
    typename A
  , typename B = detail::variant::void_

#if !defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

  , BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(
        BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
      , typename T
      , detail::variant::void_
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
    : public detail::variant::variant_base<
          boost::variant<
              A
            , B
            , BOOST_PP_ENUM_PARAMS(
                  BOOST_PP_SUB(BOOST_VARIANT_LIMIT_TYPES, 2)
                , T
                )
            >
        >
{
private: // static precondition assertions

#if !defined(BOOST_VARIANT_NO_TYPE_SEQUENCE_SUPPORT)

    typedef mpl::integral_c<unsigned, 2>
        min_list_size;

    // NOTE TO USER :
    // Compile error here indicates that variant's variadic
    // template parameter list was used inappropriately.
    BOOST_STATIC_ASSERT((
          mpl::logical_or< // B != void_ || (is_sequence<A> && size<A> >= 2)
              mpl::logical_not<
                  is_same<B, detail::variant::void_>
                >
            , mpl::logical_and<
                  mpl::is_sequence<A>
                , mpl::equal_to<
                      mpl::guarded_size<A, min_list_size>
                    , min_list_size
                    >
                >
            >::type::value
        ));

#else // defined(BOOST_VARIANT_NO_TYPE_SEQUENCE_SUPPORT)

    // Until mpl_list_initializer (below) works, sequences are not supported
    // for compilers that do not support using declarations in templates.
    BOOST_STATIC_ASSERT((!mpl::is_sequence<A>::type::value));

#endif // BOOST_VARIANT_NO_TYPE_SEQUENCE_SUPPORT

public: // typedefs

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

private: // representation

    typedef typename detail::variant::make_storage<types>::type
        storage1_t;

    typedef typename mpl::remove_if<
          types
        , has_nothrow_move_constructor<mpl::_1>
        >::type throwing_types;

    // [storage2_t = empty(throwing_types) ? null_storage : make_storage<throwing_types>]
    typedef typename mpl::apply_if<
          mpl::empty<throwing_types>
        , mpl::identity<detail::variant::null_storage>
        , detail::variant::make_storage<throwing_types>
        >::type storage2_t;

    // which_ on:
    // * [0,  size<types>) indicates storage1
    // * [-size<types>, 0) indicates storage2
    // if which_ >= 0:
    // * then which() -> which_
    // * else which() -> -(which_ + 1)
    int which_;
    compressed_pair< storage1_t,storage2_t > storage_;

    bool using_storage1() const
    {
        // If a true second storage is in use (i.e. NOT null_storage)...
        if (!is_same<storage2_t, detail::variant::null_storage>::value)
            // ...then we must check the which_ value:
            return which_ >= 0;

        // Otherwise, we know storage1 is in use because there is no storage2:
        return true;
    }

    void activate_storage1(int which)
    {
        which_ = which;
    }

    void activate_storage2(int which)
    {
        which_ = -(which + 1);
    }

    void* active_storage()
    {
        if (using_storage1() == false)
            return storage_.second().address();
        
        return storage_.first().address();
    }

    const void* active_storage() const
    {
        return const_cast<variant * const>(this)->active_storage();
    }

    void* inactive_storage()
    {
        if (using_storage1() == false)
            return storage_.first().address();
        
        return storage_.second().address();
    }

public: // queries

    int which() const
    {
        // If NOT using storage1...
        if (using_storage1() == false)
            // ...then return adjusted which_:
            return -(which_ + 1);
        
        // Otherwise, return which_ directly:
        return which_;
    }

private: // helpers, for structors (below)

// [On compilers where using declarations in class templates can correctly avoid name hiding...]
#if !defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

    // [...use an optimal converting initializer based on the variant typelist:]

    struct initializer_root
    {
    private: // helpers, for static functions (below)

        enum NotCallable { not_callable };

    public: // static functions

        // Root type must expose name "initialize," so
        // the following dummy function is provided:

        static void initialize(NotCallable) { }

    };

    struct make_initializer_node
    {
        template <typename Base, typename Iterator>
        struct apply
        {
        private: // helpers, for metafunction result (below)

            struct initializer_node
                : Base
            {
            private: // helpers, for static functions (below)

                typedef typename Iterator::type
                    T;

            public: // static functions

                using Base::initialize;

                static int initialize(void* dest, const T& operand)
                {
                    new(dest) T(operand);

                    BOOST_STATIC_CONSTANT(
                          std::size_t
                        , idx = (
                              mpl::distance<
                                  typename mpl::begin<types>::type
                                , Iterator
                                >::type::value
                            )
                        );

                    return idx;
                }

            };

        public: // metafunction result

            typedef initializer_node
                type;

        };
    };

    typedef typename mpl::iter_fold<
          types
        , initializer_root
        , mpl::protect< make_initializer_node >
        >::type initializer;

#else // defined(BOOST_NO_CLASS_TEMPLATE_USING_DECLARATIONS)

    // [...otherwise, use a hackish workaround based on variant's template parameters:]

    struct preprocessor_list_initializer
    {
    public: // static functions

        static int initialize(void* dest, const A& operand)
        {
            typedef A T;
            BOOST_STATIC_CONSTANT(int, which = 0);

            new(dest) T(operand);
            return which;
        }

        static int initialize(void* dest, const B& operand)
        {
            typedef B T;
            BOOST_STATIC_CONSTANT(int, which = 1);

            new(dest) T(operand);
            return which;
        }

        #define BOOST_VARIANT_INITIALIZE_FUNCTION(z,N,_)       \
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
    private: // helpers, for static functions (below)

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

    public: // static functions

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

    void destroy_content()
    {
        detail::variant::destroyer visitor;
        raw_apply_visitor(visitor);
    }

public: // structors

    ~variant()
    {
        destroy_content();
    }

    variant()
    {
        // NOTE TO USER :
        // Compile error from here indicates that the first bound
        // type is default-constructible, and so variant cannot
        // support its own default-construction

        new(storage_.first().address()) A;
        activate_storage1(0); // zero is the index of the first bounded type
    }

    variant(const variant& operand)
    {
        // Copy the value of operand into *this...
        detail::variant::copy_into visitor(storage_.first().address());
        operand.raw_apply_visitor(visitor);

        // ...and activate the *this's primary storage on success:
        activate_storage1(operand.which());
    }

    variant(move_source<variant> source)
    {
        variant& operand = source.get();

        // Move the value of operand into *this...
        detail::variant::move_into visitor(storage_.first().address());
        operand.raw_apply_visitor(visitor);

        // ...and activate the *this's primary storage on success:
        activate_storage1(operand.which());         
    }

private: // helpers, for structors, cont. (below)

    class convert_copy_into
        : public static_visitor<int>
    {
    private: // representation

        void* storage_;

    public: // structors

        explicit convert_copy_into(void* storage)
            : storage_(storage)
        {
        }

    public: // visitor interfaces

        template <typename T>
        int operator()(const T& operand) const
        {
            // NOTE TO USER :
            // Compile error here indicates one of the source variant's types 
            // cannot be unambiguously converted to the destination variant's
            // types (or that no conversion exists).
            //
            return initializer::initialize(storage_, operand);
        }

    };

#if !defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

public: // structors, cont.

    template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename U)>
    variant(const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, U)>& operand)
    {
        // Attempt a converting copy into *this's storage:
        convert_copy_into visitor(storage_.first().address());
        activate_storage1(
              operand.raw_apply_visitor(visitor)
            );
    }

    template <typename T>
    variant(const T& operand)
    {
        // NOTE TO USER :
        // Compile error here indicates that the given type is not 
        // unambiguously convertible to one of the variant's types
        // (or that no conversion exists).
        //
        activate_storage1(
              initializer::initialize(
                  storage_.first().address()
                , operand
                )
            );
    }

#else// defined(BOOST_NO_FUNCTION_TEMPLATE_ORDERING)

private: // workaround, for structors, cont. (below)

    template <BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename U)>
    void constructor_simulated_partial_ordering(
          const boost::variant<BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, U)>& operand
        , long)
    {
        // Attempt a converting copy into *this's storage:
        convert_copy_into visitor(storage_.first().address());
        activate_storage1(
              operand.raw_apply_visitor(visitor)
            );
    }

    template <typename T>
    void constructor_simulated_partial_ordering(const T& operand, int)
    {
        // NOTE TO USER :
        // Compile error here indicates that the given type is not 
        // unambiguously convertible to one of the variant's types
        // (or that no conversion exists).
        //
        activate_storage1(
              initializer::initialize(
                  storage_.first().address()
                , operand
                )
            );
    }

public: // structors, cont.

    template <typename T>
    variant(const T& operand)
    {
        constructor_simulated_partial_ordering(operand, 1L);
    }

#endif // BOOST_NO_FUNCTION_TEMPLATE_ORDERING workaround

private: // helpers, for modifiers (below)

    // class assign_into
    //
    // Generic visitor that assigns the value it visits to the variant it is
    // given, maintaining the strong guarantee of exception safety.
    //

    friend class assign_into;

    class assign_into
        : public static_visitor<>
    {
    private: // representation

        variant& target_;
        int source_which_;

    public: // structors

        assign_into(variant& target, int source_which)
            : target_(target)
            , source_which_(source_which)
        {
        }

    private: // helpers, for visitor interfaces (below)

        template <typename T>
        void assign_impl(
              const T& operand
            , mpl::true_c// has_nothrow_move_constructor
            )
        {
            // Attempt to make a temporary copy...
            T temp(operand);

            // ...and upon success destroy the target's active storage...
            target_.destroy_content(); // nothrow

            // ...move the temporary copy into the target's storage1...
            new(target_.storage_.first().address())  // nothrow
                T( move(temp) );

            // ...and activate the target's storage1:
            target_.activate_storage1(source_which_); // nothrow
        }

        template <typename T>
        void assign_impl(
              const T& operand
            , mpl::false_c// has_nothrow_move_constructor
            )
        {
            // Attempt a copy into target's inactive storage...
            new(target_.inactive_storage()) T(operand);

            // ...and upon success destroy the target's active storage...
            target_.destroy_content(); // nothrow

            // ...and if the target _was_ using storage1...
            if (target_.using_storage1())
                // ...then activate storage2:
                target_.activate_storage2(source_which_); // nothrow
            else
                // ...otherwise, activate storage1:
                target_.activate_storage1(source_which_); // nothrow
        }

    public: // visitor interfaces

        template <typename T>
        void operator()(const T& operand)
        {
            assign_impl(
                  operand
                , mpl::bool_c< has_nothrow_move_constructor<T>::value >()
                );
        }

    };

    void assign(const variant& operand)
    {
        assign_into visitor(*this, operand.which());
        operand.raw_apply_visitor(visitor);        
    }

public: // modifiers

    variant& operator=(const variant& rhs)
    {
        assign(rhs);
        return *this;
    }

    template <typename T>
    variant& operator=(const T& rhs)
    {
        // While potentially inefficient, the following (implicit)
        // construction of a variant allows T as any type convertible
        // to a bounded type (i.e., opposed to an exact match).

        assign(rhs);
        return *this;
    }

private: // helpers, for modifiers, cont. (below)

    // class move_assign_into
    //
    // Generic visitor that moves the value it visits to the variant it is
    // given, maintaining the strong guarantee of exception safety.
    //

    friend class move_assign_into;

    class move_assign_into
        : public static_visitor<>
    {
    private: // representation

        variant& target_;
        int source_which_;

    public: // structors

        move_assign_into(variant& target, int source_which)
            : target_(target)
            , source_which_(source_which)
        {
        }

    private: // helpers, for visitor interfaces (below)

        template <typename T>
        void move_assign_impl(
              T& operand
            , mpl::true_c// has_nothrow_move_constructor
            )
        {
            // Destroy the target's active storage...
            target_.destroy_content(); // nothrow

            // ...move the operand into the target's storage1...
            new(target_.storage_.first().address())  // nothrow
                T( move(operand) );

            // ...and activate the target's storage1:
            target_.activate_storage1(source_which_); // nothrow
        }

        template <typename T>
        void move_assign_impl(
              T& operand
            , mpl::false_c// has_nothrow_move_constructor
            )
        {
            // Attempt a move into target's inactive storage...
            new(target_.inactive_storage()) T( move(operand) );

            // ...and upon success destroy the target's active storage...
            target_.destroy_content(); // nothrow

            // ...and if the target _was_ using storage1...
            if (target_.using_storage1())
                // ...then activate storage2:
                target_.activate_storage2(source_which_); // nothrow
            else
                // ...otherwise, activate storage1:
                target_.activate_storage1(source_which_); // nothrow
        }

    public: // visitor interfaces

        template <typename T>
        void operator()(T& operand)
        {
            move_assign_impl(
                  operand
                , mpl::bool_c< has_nothrow_move_constructor<T>::value >()
                );
        }

    };

public: // modifiers, cont.

    variant& operator=(move_source<variant> source)
    {
        variant& operand = source.get();

        move_assign_into visitor(*this, operand.which());
        operand.raw_apply_visitor(visitor);

        return *this;
    }

private: // helpers, for modifiers, cont. (below)

    // class swap_variants
    //
    // Generic static visitor that swaps given lhs and rhs variants.
    //
    // NOTE: Must be applied to the rhs variant.
    //

    friend class swap_variants;

    class swap_variants
        : public static_visitor<>
    {
    private: // representation

        variant& lhs_;
        variant& rhs_;

    public: // structors

        swap_variants(variant& lhs, variant& rhs)
            : lhs_(lhs)
            , rhs_(rhs)
        {
        }

    private: // helpers, for visitor interfaces (below)

        template <typename T>
        void swap_impl(
              T& rhs_content
            , mpl::true_c// has_nothrow_move_constructor
            )
        {
            // Cache rhs's which-index (because it will be overwritten)...
            int rhs_old_which = rhs_.which();

            // ...move rhs_content to the side...
            T rhs_old_content( move(rhs_content) ); // nothrow

            try
            {
                // ...attempt to move-assign lhs to (now-moved) rhs:
                rhs_ = move(lhs_);
            }
            catch(...)
            {
                // In case of failure, restore rhs's old contents...
                new(boost::addressof(rhs_content))     // nothrow
                    T( move(rhs_old_content) );

                // ...and rethrow:
                throw;
            }

            // In case of success, destroy lhs's active storage...
            lhs_.destroy_content(); // nothrow

            // ...move rhs's old contents to lhs's storage1...
            new(lhs_.storage_.first().address())   // nothrow
                T( move(rhs_old_content) );

            // ...and activate lhs's storage1:
            lhs_.activate_storage1(rhs_old_which); // nothrow
        }

        template <typename T>
        void swap_impl(
              T& rhs_content
            , mpl::false_c// has_nothrow_move_constructor
            )
        {
            // Cache rhs's which-index (because it will be overwritten)...
            int rhs_old_which = rhs_.which();

            // ...move rhs's content into lhs's inactive storage...
            new(lhs_.inactive_storage()) T(move(rhs_content));

            try
            {
                // ...attempt to move-assign lhs to (now-copied) rhs:
                rhs_ = move(lhs_);
            }
            catch(...)
            {
                // In case of failure, destroy the copied value...
                static_cast<T*>(lhs_.inactive_storage())->~T(); // nothrow
            }

            // In case of success, destroy lhs's active storage...
            lhs_.destroy_content(); // nothrow

            // ...and if lhs _was_ using storage1...
            if (lhs_.using_storage1()) // nothrow
            {
                // ...then activate storage2:
                lhs_.activate_storage2(rhs_old_which); // nothrow
            }
            else
            {
                // ...otherwise, activate storage1:
                lhs_.activate_storage1(rhs_old_which); // nothrow
            }
        }

    public: // visitor interfaces

        template <typename T>
        void operator()(T& rhs_content)
        {
            swap_impl(
                  rhs_content
                , mpl::bool_c< has_nothrow_move_constructor<T>::value >()
                );
        }

    };

public: // modifiers, cont.

    void swap(variant& rhs)
    {
        // If the types are the same...
        if (which() == rhs.which())
        {
            // ...then swap the values directly:
            detail::variant::swap_with visitor(active_storage());
            rhs.raw_apply_visitor(visitor);
        }
        else
        {
            // Otherwise, perform general variant swap:
            swap_variants visitor(*this, rhs);
            rhs.raw_apply_visitor(visitor);
        }
    }

public: // queries

    //
    // NOTE: member which() defined above.
    //

    bool empty() const
    {
        return false;
    }

    const std::type_info& type() const
    {
        detail::variant::reflect visitor;
        return this->apply_visitor(visitor);
    }

private: // helpers, for visitation support (below)

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
            return visitor(
                  *static_cast<T*>( var.active_storage() )
                );
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

public: // helpers, for visitation support (below)

    //////////////////////////////////////////////////////////////////////
    // WARNING TO USER :
    // The following functions are not part of the public interface,
    // despite their public access (which may change in the future).
    //
    template <typename Visitor>
        typename Visitor::result_type
    raw_apply_visitor(Visitor& visitor)
    {
        return apply_visitor_impl<
              mpl::integral_c<unsigned long, 0>
            , typename mpl::begin<types>::type
            , typename mpl::end<types>::type
            >(which(), *this, visitor, mpl::false_c());
    }

    template <typename Visitor>
        typename Visitor::result_type
    raw_apply_visitor(Visitor& visitor) const
    {
        return apply_visitor_impl<
              mpl::integral_c<unsigned long, 0>
            , typename mpl::begin<types>::type
            , typename mpl::end<types>::type
            >(which(), *this, visitor, mpl::false_c());
    }

public: // visitation support

    template <typename Visitor>
        typename Visitor::result_type
    apply_visitor(Visitor& visitor)
    {
        detail::variant::invoke_visitor<Visitor> invoker(visitor);
        return raw_apply_visitor(invoker);
    }

    template <typename Visitor>
        typename Visitor::result_type
    apply_visitor(Visitor& visitor) const
    {
        detail::variant::invoke_visitor<Visitor> invoker(visitor);
        return raw_apply_visitor(invoker);
    }

public: // value extraction support

    template <typename T>
    T* extract()
    {
        detail::variant::cast_to<T> visitor;
        return this->apply_visitor(visitor);
    }

    template <typename T>
    T* extract() const
    {
        detail::variant::cast_to<T> visitor;
        return this->apply_visitor(visitor);
    }

};

//////////////////////////////////////////////////////////////////////////
// function template swap
//
// Swaps two variants of the same type (i.e., identical bounded type set).
//
template < BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, typename T) >
void swap(
      boost::variant< BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T) >& lhs
    , boost::variant< BOOST_PP_ENUM_PARAMS(BOOST_VARIANT_LIMIT_TYPES, T) >& rhs
    )
{
    lhs.swap(rhs);
}

} // namespace boost

#endif // BOOST_VARIANT_HPP
