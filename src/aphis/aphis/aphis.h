#ifndef APHIS_H
#define APHIS_H

#include <opencv2/core/core.hpp>


struct AphisPattern {

  cv::Mat img;
  std::string name;

  void show();
};
typedef std::vector<boost::shared_ptr<AphisPattern> > PatternList;

struct CompareParameters {
  int rows;
  int cols;
  float border_top, border_bottom;
  float border_left, border_right;
};



void loadPattern_impl(const char* filename, AphisPattern& f);
double compareTwoPatterns_impl(AphisPattern& p1, AphisPattern& p2, const CompareParameters& params);
void compareAllPatterns_impl(const PatternList &fromPatterns,
			     const PatternList &withPatterns,
			     CompareParameters& params,
			     boost::python::object callback);



#endif /* APHIS_H */
