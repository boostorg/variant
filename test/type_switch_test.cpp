#include "boost/test/minimal.hpp"
#include "boost/type_switch.hpp"

#include "boost/variant/variant.hpp"
#include "boost/mpl/pair.hpp"
#include "boost/mpl/placeholders.hpp"

bool verify_pass_;

template <bool B>
struct verifier
{
    template <typename T>
    void operator()(const T&) const
    {
        verify_pass_ = B;
    }
};

typedef verifier<true> verify;
typedef verifier<false> verify_not;

template <bool B>
struct verifier_default
{
    template <typename T>
    void operator()() const
    {
        verify_pass_ = B;
    }
};

typedef verifier_default<true> verify_default;
typedef verifier_default<false> verify_not_default;


#define VERIFY_LAST \
    do { BOOST_CHECK( verify_pass_ ); verify_pass_ = false; } while(false)



int test_main(int , char* [])
{
    namespace mpl = boost::mpl;
    using namespace boost::type_switch;
    using namespace boost::mpl::placeholders;

    boost::variant<int, double> var1;

    switch_(var1)
      |= case_<int>( verify() )
      |= case_<double>( verify_not() )
      ;
    VERIFY_LAST;

    switch_(var1)
      |= case_<float>( verify_not() )
      |= default_( verify_default() )
      ;
    VERIFY_LAST;

    switch_(var1)
      |= case_<float>( verify_not() )
      |= case_< _ >( verify() )
      ;
    VERIFY_LAST;

    boost::variant< mpl::pair<int,double>, int > var2;

    switch_(var2)
      |= case_< int >( verify_not() )
      |= case_< mpl::pair<_,_> >( verify() )
      ;
    VERIFY_LAST;

    switch_(var2)
      |= case_< int >( verify_not() )
      |= case_< mpl::pair<_1,_1> >( verify_not() )
      |= case_< mpl::pair<_,_> >( verify() )
      ;
    VERIFY_LAST;

    return boost::exit_success;
}
