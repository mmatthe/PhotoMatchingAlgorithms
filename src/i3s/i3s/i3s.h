struct finger
{
  finger() {};

  struct raw
  {
  double ref[6];
  double data[9*(35+3+1)];
  int nPoints;
  };
  raw raw_;
  std::string name;

private:
  finger(const finger&);
  finger& operator=(const finger&);
};


double compareTwo_Impl(double* uref, double* udata, int unr,
						double* kref, double* kdata, int knr,
		       long* pairs, int nPairs);

void calcSurfFeatures_Impl(signed char* imIn, int width, int height, int nrElts, float relKeypointDist,
			   float* roix, float* roiy,
			   float* x, float* y, float* size, float* descr,
			   float warpSize);

double compareTwoFingers(finger& f1, finger& f2);



typedef std::vector<boost::shared_ptr<finger> > FingerList;
void compareAllPatterns(const FingerList &fromPatterns,
                        const FingerList &withPatterns,
                        boost::python::object callback);
