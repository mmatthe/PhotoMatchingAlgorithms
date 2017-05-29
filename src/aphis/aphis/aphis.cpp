#include <iostream>
#include <string>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "aphis.h"

#include <boost/make_shared.hpp>

using namespace boost::python;


void compareAllPatterns(const PatternList &fromPatterns,
                        const PatternList &withPatterns,
                        boost::python::object callback,
			CompareParameters& params)
{
  compareAllPatterns_impl(fromPatterns, withPatterns, params, callback);
}



AphisPattern* loadPattern(const char* filename)
{
  AphisPattern *p = new AphisPattern;
  loadPattern_impl(filename, *p);
  return p;
}



double compareTwoPatterns(AphisPattern& p1, AphisPattern& p2, const CompareParameters& params)
{
  return compareTwoPatterns_impl(p1, p2, params);
}


BOOST_PYTHON_MODULE(aphis)
{
  class_<AphisPattern, boost::noncopyable>("AphisPattern", no_init)
    .def("__init__", make_constructor(loadPattern))
    .def("show", &AphisPattern::show)
    // .staticmethod("createFromImage", return_value_policy<manage_new_object>())
    .def_readwrite("name", &AphisPattern::name);

  class_<CompareParameters, boost::noncopyable>("CompareParameters")
    .def_readwrite("rows", &CompareParameters::rows)
    .def_readwrite("cols", &CompareParameters::cols)
    .def_readwrite("border_top", &CompareParameters::border_top)
    .def_readwrite("border_bottom", &CompareParameters::border_bottom)
    .def_readwrite("border_left", &CompareParameters::border_left)
    .def_readwrite("border_right", &CompareParameters::border_right);



  def("loadPattern", loadPattern, return_value_policy<manage_new_object>());

  def("comparePatterns", compareTwoPatterns);
  def("compareAllPatterns", compareAllPatterns);

  class_<PatternList>("PatternList")
    .def(vector_indexing_suite<PatternList>());
}
