//  Boost.Varaint
//  Multivisitors defined here 
//
//  See http://www.boost.org for most recent version, including documentation.
//
//  Copyright Antony Polukhin, 2013-2014.
//
//  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt).

#ifndef BOOST_VARIANT_MULTIVISITORS_HPP
#define BOOST_VARIANT_MULTIVISITORS_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include <boost/variant.hpp>

#if !defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)


#include <utility>
#include <tuple>

namespace boost { 

namespace detail { namespace variant {

    template <size_t... I, typename Tuple>
    decltype(auto) tuple_subset(const Tuple& tpl, std::index_sequence<I...>)
    {
        return std::make_tuple(std::get<I + 1>(tpl)...);
    }

    template <typename Head, typename... Tail>
    std::tuple<Tail...> tuple_tail(const std::tuple<Head, Tail...>& tpl)
    {
        return tuple_subset(tpl, std::make_index_sequence<sizeof...(Tail)>());
    }

    template <typename Visitor, typename... Values, std::size_t Delimiter, typename... Visitables>
    class values_combiner_and_extractor
    {
        Visitor&                        visitor_;
        std::tuple<Values&...>          values_;
        std::tuple<Visitables&...>      visitables_;

    public: // structors

        values_combiner_and_extractor(
                    Visitor& visitor, std::tuple<Values&...> values, std::tuple<Visitables&...> visitables
                ) BOOST_NOEXCEPT
            : visitor_(visitor)
            , values_(values)
            , visitables_(visitables)
        {}

    public: // visitor interfaces

        template <typename Value>
        decltype(auto) operator()(Value& value)
        {
            return ::boost::apply_visitor(
                make_values_combiner_and_extractor(
                    visitor,
                    std::tuple_cat(values_, std::tuple<Value&>()),
                    tuple_tail(visitables_)
                )
                , std::get<0>(visitables_)
            );
        }

    private:
        values_combiner_and_extractor& operator=(const values_combiner_and_extractor&);
    };

    template <typename Visitor, typename... Values, std::size_t Delimiter>
    class values_combiner_and_extractor
    {
        Visitor&                        visitor_;
        std::tuple<Values&...>          values_;
    public:

        values_combiner_and_extractor(
                    Visitor& visitor, std::tuple<Values&...> values, std::tuple<> /*visitables*/
                ) BOOST_NOEXCEPT
            : visitor_(visitor)
            , values_(values)
        {}

        template <typename Value>
        decltype(auto) operator()(Value& value)
        {
            return do_call(
                std::tuple_cat(values_, std::tuple<Value&>()),
                std::make_index_sequence<sizeof...(Values) + 1
            );
        }

        template <class Tuple, std::size_t... I>
        decltype(auto) do_call(Tuple& t, std::index_sequence<I...>) const {
            return visitor_(std::get<I>(t)...);
        }
    };



    template <class Visitor, class T1, class T2, class T3, class... TN>
    inline decltype(auto) apply_visitor(const Visitor& visitor, T1& v1, T2& v2, TN&... vn)
    {
        return boost::apply_visitor(
            make_values_combiner_and_extractor(
                visitor,
                std::tuple<>(),
                std::forward_as_tuple(v2, vn...)
            ),
            v1
        );
    }
    
} // namespace boost


#else // !defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)

#include <boost/bind.hpp>

#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/sub.hpp>

#ifndef BOOST_VARAINT_MAX_MULTIVIZITOR_PARAMS
#   define BOOST_VARAINT_MAX_MULTIVIZITOR_PARAMS 4
#endif

namespace boost { 

namespace detail { namespace variant {

    template <class VisitorT, class Visitable1T, class Visitable2T>
    struct two_variables_holder {
    private:
        VisitorT&       visitor_;
        Visitable1T&    visitable1_;
        Visitable2T&    visitable2_;

        // required to supress warnings and enshure that we do not copy 
        // this visitor
        two_variables_holder& operator=(const two_variables_holder&);

    public:
        typedef BOOST_DEDUCED_TYPENAME VisitorT::result_type result_type;

        explicit two_variables_holder(VisitorT& visitor, Visitable1T& visitable1, Visitable2T& visitable2) BOOST_NOEXCEPT 
            : visitor_(visitor)
            , visitable1_(visitable1)
            , visitable2_(visitable2)
        {}

#define BOOST_VARIANT_OPERATOR_BEG()                            \
    return ::boost::apply_visitor(                              \
    ::boost::bind<result_type>(boost::ref(visitor_), _1, _2     \
    /**/

#define BOOST_VARIANT_OPERATOR_END()                            \
    ), visitable1_, visitable2_);                               \
    /**/

#define BOOST_VARANT_VISITORS_VARIABLES_PRINTER(z, n, data)     \
    BOOST_PP_COMMA() boost::ref( BOOST_PP_CAT(vis, n) )         \
    /**/

#define BOOST_VARIANT_VISIT(z, n, data)                                                     \
    template <BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n, 1), class VisitableUnwrapped)>           \
    BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(result_type) operator()(                          \
        BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_ADD(n, 1), VisitableUnwrapped, & vis)          \
    ) const                                                                                 \
    {                                                                                       \
        BOOST_VARIANT_OPERATOR_BEG()                                                        \
        BOOST_PP_REPEAT(BOOST_PP_ADD(n, 1), BOOST_VARANT_VISITORS_VARIABLES_PRINTER, ~)     \
        BOOST_VARIANT_OPERATOR_END()                                                        \
    }                                                                                       \
    /**/

BOOST_PP_REPEAT( BOOST_PP_SUB(BOOST_VARAINT_MAX_MULTIVIZITOR_PARAMS, 2), BOOST_VARIANT_VISIT, ~)
#undef BOOST_VARIANT_OPERATOR_BEG
#undef BOOST_VARIANT_OPERATOR_END
#undef BOOST_VARANT_VISITORS_VARIABLES_PRINTER
#undef BOOST_VARIANT_VISIT

    };

    template <class VisitorT, class Visitable1T, class Visitable2T>
    inline two_variables_holder<VisitorT, Visitable1T, Visitable2T> make_two_variables_holder(
            VisitorT& visitor, Visitable1T& visitable1, Visitable2T& visitable2
        ) BOOST_NOEXCEPT
    {
        return two_variables_holder<VisitorT, Visitable1T, Visitable2T>(visitor, visitable1, visitable2);
    }

    template <class VisitorT, class Visitable1T, class Visitable2T>
    inline two_variables_holder<const VisitorT, Visitable1T, Visitable2T> make_two_variables_holder(
            const VisitorT& visitor, Visitable1T& visitable1, Visitable2T& visitable2
        ) BOOST_NOEXCEPT
    {
        return two_variables_holder<const VisitorT, Visitable1T, Visitable2T>(visitor, visitable1, visitable2);
    }

}} // namespace detail::variant

#define BOOST_VARIANT_APPLY_VISITOR_BEG()                                               \
    return ::boost::apply_visitor(                                                      \
            boost::detail::variant::make_two_variables_holder(visitor, var0 , var1),    \
            var2                                                                        \
    /**/

#define BOOST_VARIANT_APPLY_VISITOR_END()                       \
    );                                                          \
    /**/

#define BOOST_VARANT_VISITORS_VARIABLES_PRINTER(z, n, data)     \
    BOOST_PP_COMMA() BOOST_PP_CAT(var, BOOST_PP_ADD(n, 3))      \
    /**/

#define BOOST_VARIANT_VISIT(z, n, data)                                                                 \
    template <class Visitor BOOST_PP_COMMA() BOOST_PP_ENUM_PARAMS(BOOST_PP_ADD(n, 3), class T)>         \
    inline BOOST_VARIANT_AUX_GENERIC_RESULT_TYPE(BOOST_DEDUCED_TYPENAME Visitor::result_type) apply_visitor( \
        data BOOST_PP_COMMA() BOOST_PP_ENUM_BINARY_PARAMS(BOOST_PP_ADD(n, 3), T, & var)     \
    )                                                                                                   \
    {                                                                                                   \
        BOOST_VARIANT_APPLY_VISITOR_BEG()                                                               \
        BOOST_PP_REPEAT(n, BOOST_VARANT_VISITORS_VARIABLES_PRINTER, ~)                                  \
        BOOST_VARIANT_APPLY_VISITOR_END()                                                               \
    }                                                                                                   \
    /**/

BOOST_PP_REPEAT( BOOST_PP_SUB(BOOST_VARAINT_MAX_MULTIVIZITOR_PARAMS, 2), BOOST_VARIANT_VISIT, const Visitor& visitor)
BOOST_PP_REPEAT( BOOST_PP_SUB(BOOST_VARAINT_MAX_MULTIVIZITOR_PARAMS, 2), BOOST_VARIANT_VISIT, Visitor& visitor)

#undef BOOST_VARIANT_APPLY_VISITOR_BEG
#undef BOOST_VARIANT_APPLY_VISITOR_END
#undef BOOST_VARANT_VISITORS_VARIABLES_PRINTER
#undef BOOST_VARIANT_VISIT
    
} // namespace boost

#endif // !defined(BOOST_VARIANT_DO_NOT_USE_VARIADIC_TEMPLATES)

#endif // BOOST_VARIANT_MULTIVISITORS_HPP

