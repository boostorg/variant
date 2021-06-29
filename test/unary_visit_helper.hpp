#if defined(DECLTYPE_VISITOR)
{
    struct visitor_type
    {
    #if !DECLTYPE_VISITOR
        typedef int result_type;
    #endif
        int operator()(udt1 VARIANT_QUALIFIERS) VISITOR_QUALIFIERS { return 0; }
        int operator()(udt2 VARIANT_QUALIFIERS) VISITOR_QUALIFIERS { return 1; }
    } visitor;

    typedef boost::variant<udt1, udt2> variant_type;
    variant_type v = udt2();

    BOOST_TEST_EQ(1,
        (boost::apply_visitor(
            static_cast<visitor_type VISITOR_QUALIFIERS>(visitor),
            static_cast<variant_type VARIANT_QUALIFIERS>(v))
        ));

    v = udt1();

    BOOST_TEST_EQ(0,
        (boost::apply_visitor(
            static_cast<visitor_type VISITOR_QUALIFIERS>(visitor),
            static_cast<variant_type VARIANT_QUALIFIERS>(v))
        ));
}
#elif defined(VARIANT_QUALIFIERS)

#define DECLTYPE_VISITOR 0
#include "unary_visit_helper.hpp"
#undef DECLTYPE_VISITOR
#ifdef BOOST_VARIANT_HAS_DECLTYPE_APPLY_VISITOR_RETURN_TYPE
#define DECLTYPE_VISITOR 1
#include "unary_visit_helper.hpp"
#undef DECLTYPE_VISITOR
#endif

#elif defined(VISITOR_QUALIFIERS)

#define VARIANT_QUALIFIERS
#include "unary_visit_helper.hpp"
#undef VARIANT_QUALIFIERS
#ifndef BOOST_NO_CXX11_REF_QUALIFIERS // BOOST_NO_CXX11_RVALUE_REFERENCES is not enough for disabling buggy GCCs < 4.8
#define VARIANT_QUALIFIERS &
#include "unary_visit_helper.hpp"
#undef VARIANT_QUALIFIERS
#define VARIANT_QUALIFIERS const&
#include "unary_visit_helper.hpp"
#undef VARIANT_QUALIFIERS
#define VARIANT_QUALIFIERS &&
#include "unary_visit_helper.hpp"
#undef VARIANT_QUALIFIERS
#define VARIANT_QUALIFIERS const&&
#include "unary_visit_helper.hpp"
#undef VARIANT_QUALIFIERS
#endif

#elif !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

//#define VISITOR_QUALIFIERS
//#include "unary_visit_helper.hpp"
//#undef VISITOR_QUALIFIERS
#define VISITOR_QUALIFIERS const
#include "unary_visit_helper.hpp"
#undef VISITOR_QUALIFIERS
#ifndef BOOST_NO_CXX11_REF_QUALIFIERS
#define VISITOR_QUALIFIERS &
#include "unary_visit_helper.hpp"
#undef VISITOR_QUALIFIERS
#define VISITOR_QUALIFIERS const&
#include "unary_visit_helper.hpp"
#undef VISITOR_QUALIFIERS
//#define VISITOR_QUALIFIERS &&
//#include "unary_visit_helper.hpp"
//#undef VISITOR_QUALIFIERS
//#define VISITOR_QUALIFIERS const&&
//#include "unary_visit_helper.hpp"
//#undef VISITOR_QUALIFIERS
#endif

#endif
