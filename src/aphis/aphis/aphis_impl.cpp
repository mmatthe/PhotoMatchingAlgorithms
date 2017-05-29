#include <vector>
#include <iostream>
#include <string>
#include <boost/thread.hpp>
#include <boost/python.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


#include "aphis.h"


void AphisPattern::show()
{
  cv::namedWindow(name);
  cv::imshow(name, img);
  cv::waitKey(0);
}

void loadPattern_impl(const char* filename, AphisPattern& f)
{
  cv::Mat img = cv::imread(filename);
  if (img.channels() == 3)
    cv::cvtColor(img, f.img, cv::COLOR_BGR2GRAY);
  else
    f.img = img;
}

double compareTwoPatterns_impl(AphisPattern& p1, AphisPattern& p2, const CompareParameters& params)
{
  if (p1.img.size != p2.img.size)
    throw std::runtime_error("Both patterns have a different size!");

  float borderTop = params.border_top;
  float borderBottom = params.border_bottom;
  float borderLeft = params.border_left;
  float borderRight = params.border_right;

  int X = p1.img.cols * (1-borderLeft-borderRight);
  int Y = p1.img.rows * (1-borderTop - borderBottom);
  int dx = X / params.cols;
  int dy = Y / params.rows;
  int startX = p1.img.cols * borderLeft;
  int startY = p1.img.rows * borderTop;
  int py = startY;

  int border = 30;

  float overallEQ = 0;
  for (int r = 0; r < params.rows; r++)
    {
      int px = startX;
      for (int c = 0; c < params.cols; c++)
	{
	  cv::Mat matched;
	  int origXstart = std::max(0, px - border);
	  int origXend = std::min(p1.img.cols, px + dx + border);
	  int origYstart = std::max(0, py - border);
	  int origYend = std::min(p1.img.rows, py + dy + border);

	  cv::Mat part = p1.img(cv::Range(py, py+dy), cv::Range(px, px+dx));
	  cv::Mat orig = p2.img(cv::Range(origYstart, origYend), cv::Range(origXstart, origXend));
	  // orig = p2.img;
	  // std::cout << part.rows << " " << part.cols << "   " << orig.rows << " " << orig.cols << std::endl;
	  cv::matchTemplate(orig, part, matched, cv::TM_CCOEFF_NORMED);
	  double maxVal;
	  cv::Point maxPos;
	  cv::minMaxLoc(matched, NULL, &maxVal, NULL, &maxPos);
	  // std::cout << matched.rows << " " << matched.cols << " " << px << " " << py << " PartEQ: " << maxVal << "(" << maxPos << ")" << std::endl;

	  maxVal = std::max(0.0, maxVal);
	  overallEQ += maxVal;

	  px += dx;
	}
      py += dy;
    }

  return overallEQ;
}

struct compareThread
{
  compareThread(const PatternList& toCompare_, const PatternList& against_, const CompareParameters& params, boost::python::object callback_, boost::mutex& mutex_)
    : toCompare(toCompare_), against(against_), callback(callback_), mutex(mutex_), params(params)
   {}

   const PatternList& toCompare, against;
  const CompareParameters& params;
   boost::python::object callback;
   boost::mutex& mutex;

   void operator()()
   {
      for(auto p1 : toCompare)
      {
	 for(auto p2: against)
	   {
	     double score = compareTwoPatterns_impl(*p1, *p2, params);
	     doCallback(p1, p2, score);
	   }
      }
   }

      void doCallback(boost::shared_ptr<AphisPattern> s1, boost::shared_ptr<AphisPattern> s2, float eq)
   {
      boost::lock_guard<boost::mutex> guard(mutex);
      PyGILState_STATE state = PyGILState_Ensure();
      try{
	callback(s1, s2, eq);
      }
      catch(boost::python::error_already_set& e)
	{
	  PyErr_Print();
	}
      PyGILState_Release(state);
      // std::cout << s1 << s2 << eq << std::endl;
   }
};



void compareAllPatterns_impl(const PatternList &fromPatterns,
			     const PatternList &withPatterns,
			     CompareParameters& params,
			     boost::python::object callback) {
   const int numThreads = std::min(14u, boost::thread::hardware_concurrency());
   std::cout << "Running Comparison with " << numThreads << " threads in parallel...\n" << std::endl;

   int patternsPerThread = fromPatterns.size() / numThreads + 1;
   std::vector<PatternList> threadInput;

   for (PatternList::const_iterator it = fromPatterns.begin(); it < fromPatterns.end(); it += patternsPerThread)
   {
      PatternList::const_iterator other;
      if (it + patternsPerThread < fromPatterns.end())
	 other = it + patternsPerThread;
      else
	 other = fromPatterns.end();
      threadInput.push_back(PatternList(it, other));
   }

   Py_BEGIN_ALLOW_THREADS
   boost::thread_group threads;
   boost::mutex mutex;
   for(int i = 0; i < threadInput.size(); i++)
   {
     threads.create_thread(compareThread(threadInput[i], withPatterns, params, callback, mutex));
   }
   threads.join_all();

   Py_END_ALLOW_THREADS
}
