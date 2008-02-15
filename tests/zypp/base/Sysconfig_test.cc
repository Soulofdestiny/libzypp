
#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include "zypp/base/Logger.h"
#include "zypp/base/Exception.h"
#include "zypp/TmpPath.h"
#include "zypp/PathInfo.h"


#include <boost/test/unit_test.hpp>
#include <boost/test/parameterized_test.hpp>
#include <boost/test/unit_test_log.hpp>

#include "zypp/base/Sysconfig.h"

using boost::unit_test::test_suite;
using boost::unit_test::test_case;
using namespace boost::unit_test::log;

using namespace std;
using namespace zypp;


void sysconfig_test( const string &dir )
{
  Pathname file = Pathname(dir) + "proxy";
  map<string,string> values = zypp::base::sysconfig::read(file);
  BOOST_CHECK_EQUAL( values.size(), 6 );
  BOOST_CHECK_EQUAL( values["PROXY_ENABLED"], "no");
  BOOST_CHECK_EQUAL( values["GOPHER_PROXY"], "");
  BOOST_CHECK_EQUAL( values["NO_PROXY"], "localhost, 127.0.0.1");
}

test_suite*
init_unit_test_suite( int argc, char* argv[] )
{
  string datadir;
  if (argc < 2)
  {
    datadir = TESTS_SRC_DIR;
    datadir = (Pathname(datadir) + "/zypp/base/data/Sysconfig").asString();
    cout << "sysconfig_test:"
      " path to directory with test data required as parameter. Using " << datadir  << endl;
    //return (test_suite *)0;
  }
  else
  {
    datadir = argv[1];
  }

  std::string const params[] = { datadir };
    //set_log_stream( std::cout );
  test_suite* test= BOOST_TEST_SUITE( "SysconfigTest" );
  test->add(BOOST_PARAM_TEST_CASE( &sysconfig_test,
                              (std::string const*)params, params+1));
  return test;
}
