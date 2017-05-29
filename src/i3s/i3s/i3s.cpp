#include <iostream>
#include <string>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include <boost/make_shared.hpp>

#include "i3s.h"

using namespace boost::python;


void loadFingerprint_impl(const char* filename, finger& f)
{
  FILE* fp = fopen(filename, "rb");
  if (fp == NULL)
    throw std::runtime_error("File not found!");
  fread(&(f.raw_), sizeof(finger::raw), 1, fp);
  fclose(fp);
}

finger* loadFingerprint(const char* filename)
{
  finger *f = new finger;
  loadFingerprint_impl(filename, *f);
  return f;
}

void saveFingerprint(const char* filename, const finger& f)
{
  FILE* fp = fopen(filename, "wb");
  fwrite(&(f.raw_), sizeof(finger::raw), 1, fp);
  fclose(fp);
}


void rawToFinger(float* x, float* y, float* size, float* descr, int imWidth, int imHeight, finger& out)
{
  out.raw_.ref[0] = imWidth-1;
  out.raw_.ref[1] = 0;
  out.raw_.ref[2] = 0;
  out.raw_.ref[3] = 0;
  out.raw_.ref[4] = imWidth/2;
  out.raw_.ref[5] = imHeight-1;

  int i = 0;
  while(x[i] > -100)
    {
      double s = size[i];
      out.raw_.data[i*9 + 0] = x[i] - s/2;
      out.raw_.data[i*9 + 1] = y[i];
      out.raw_.data[i*9 + 2] = x[i] + s/2;
      out.raw_.data[i*9 + 3] = y[i];
      out.raw_.data[i*9 + 4] = x[i];
      out.raw_.data[i*9 + 5] = y[i] - s/2;
      out.raw_.data[i*9 + 6] = x[i];
      out.raw_.data[i*9 + 7] = y[i] + s/2;
      out.raw_.data[i*9 + 8] = descr[i*128];
      i++;
    }
  out.raw_.nPoints = i;
}


double compareTwoFingers(finger& f1, finger& f2)
{
  double score = compareTwo_Impl(f1.raw_.ref, f1.raw_.data, f1.raw_.nPoints, f2.raw_.ref, f2.raw_.data, f2.raw_.nPoints, NULL, 100);
  return score;
}

void createFingerprintFromImage_Impl(cv::Mat& bin, finger& out, char* p="bla")
{
  bin = bin.clone();
  const int nrElts = 25;
  std::vector<float> x(nrElts+1), y(nrElts+1), size(nrElts+1);
  std::vector<float> descr(128*(nrElts+1));

  float roix[6] = {0, bin.cols-1, bin.cols-1, 0, 0, 0};
  float roiy[6] = {0, 0, bin.rows-1, bin.rows-1, 0, 0};

  signed char* data = new signed char[bin.cols * bin.rows];
  int index = 0;
  for(int r = 0; r < bin.rows; r++)
    {
      for(int c = 0; c < bin.cols; c++)
	data[index++] = bin.at<char>(r,c);
    }

  calcSurfFeatures_Impl(data, bin.cols, bin.rows, nrElts, 0.05, roix, roiy, &x[0], &y[0], &size[0], &descr[0], 500);

  rawToFinger(&x[0], &y[0], &size[0], &descr[0], bin.cols, bin.rows, out);
  return;

  bin = bin / 4;
  for(int i = 0; i < nrElts; i++)
    {
      cv::circle(bin, cv::Point(x[i], y[i]), size[i]/2, cv::Scalar(255));
    }
  cv::imshow(p, bin);
}

finger* createFingerprintFromImage(const char* filename)
{
  cv::Mat bin = cv::imread(filename, CV_LOAD_IMAGE_UNCHANGED);
  if (bin.rows == 0)
    throw std::runtime_error("Image cannot be loaded!");
  if (bin.type() != CV_8U)
    throw std::runtime_error("Wrong image format!");

  finger* f = new finger;
  createFingerprintFromImage_Impl(bin, *f);
  return f;
}

void drawFingerprint(const char* filenameIn, const finger& f, const char* filenameOut)
{
  cv::Mat img = cv::imread(filenameIn, CV_LOAD_IMAGE_UNCHANGED);

  cv::Scalar color = cv::Scalar(255);
  if (img.channels() == 3)
    {
      int x = 200;
      color = cv::Scalar(x, x, x);
    }
  else
    {
      img = img / 4;
    }

  for (int p = 0; p < f.raw_.nPoints; p++)
    {
      float px = f.raw_.data[9*p+4];
      float py = f.raw_.data[9*p+1];
      float rad = f.raw_.data[9*p+2] - px;

      cv::circle(img, cv::Point(px, py), rad, color);
    }
  cv::imwrite(filenameOut, img);

}

BOOST_PYTHON_MODULE(i3s)
{
  class_<finger, boost::noncopyable>("finger", no_init)
    .def("__init__", make_constructor(loadFingerprint))
    // .staticmethod("createFromImage", return_value_policy<manage_new_object>())
    .def_readwrite("name", &finger::name);


  def("loadFingerprint", loadFingerprint, return_value_policy<manage_new_object>());
  def("saveFingerprint", saveFingerprint);

  def("createFingerprintFromImage", createFingerprintFromImage, return_value_policy<manage_new_object>());



  def("compareFingerprints", compareTwoFingers);

  typedef std::vector<boost::shared_ptr<finger> > FingerList;
  class_<FingerList>("PatternList")
    .def(vector_indexing_suite<FingerList>());
   def("compareAllPatterns", compareAllPatterns);

   def("drawFingerprint", drawFingerprint);
}
