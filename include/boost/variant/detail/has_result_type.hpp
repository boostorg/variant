#ifndef BOOST_VARIANT_DETAIL_HAS_RESULT_TYPE_HPP
#define BOOST_VARIANT_DETAIL_HAS_RESULT_TYPE_HPP

#include "boost/config.hpp"


namespace boost { namespace detail { namespace variant {

template <typename T >
struct has_result_type {
private:
    typedef char                      yes;
    typedef struct { char array[2]; } no;

    template<typename C> static yes test(typename C::result_type*);
    template<typename C> static no  test(...);

public:
    BOOST_STATIC_CONSTANT(bool, value = sizeof(test<T>(0)) == sizeof(yes));
};

}}} // namespace boost::detail::variant

#endif // BOOST_VARIANT_DETAIL_HAS_RESULT_TYPE_HPP

