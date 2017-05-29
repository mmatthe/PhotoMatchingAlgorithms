#include <vector>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/python.hpp>



#include "i3s.h"

struct compareThread
{
   compareThread(const FingerList& toCompare_, const FingerList& against_, boost::python::object callback_, boost::mutex& mutex_)
      : toCompare(toCompare_), against(against_), callback(callback_), mutex(mutex_)
   {}

   const FingerList& toCompare, against;
   boost::python::object callback;
   boost::mutex& mutex;

   void operator()()
   {
      for(auto p1 : toCompare)
      {
	 for(auto p2: against)
	   {
	     double score = compareTwoFingers(*p1, *p2);
	     doCallback(p1, p2, score);
	   }
      }
   }

      void doCallback(boost::shared_ptr<finger> s1, boost::shared_ptr<finger> s2, float eq)
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



void compareAllPatterns(const FingerList &fromPatterns,
                        const FingerList &withPatterns,
                        boost::python::object callback) {
   const int numThreads = std::min(14u, boost::thread::hardware_concurrency());
   std::cout << "Running Comparison with " << numThreads << " threads in parallel...\n" << std::endl;

   int patternsPerThread = fromPatterns.size() / numThreads + 1;
   std::vector<FingerList> threadInput;

   for (FingerList::const_iterator it = fromPatterns.begin(); it < fromPatterns.end(); it += patternsPerThread)
   {
      FingerList::const_iterator other;
      if (it + patternsPerThread < fromPatterns.end())
	 other = it + patternsPerThread;
      else
	 other = fromPatterns.end();
      threadInput.push_back(FingerList(it, other));
   }

   Py_BEGIN_ALLOW_THREADS
   boost::thread_group threads;
   boost::mutex mutex;
   for(int i = 0; i < threadInput.size(); i++)
   {
      threads.create_thread(compareThread(threadInput[i], withPatterns, callback, mutex));
   }
   threads.join_all();

   Py_END_ALLOW_THREADS
}
