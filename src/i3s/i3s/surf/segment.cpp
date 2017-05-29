#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/ml/ml.hpp>

using namespace cv;


/***
	Pixel bij pixel classification using RGB values and a trained SVM. Output is stored in a binary image 
	(byte per pixel).
	Possible imporvements: using other colour space, and or including the XY-pixel information (sample of R,G,B and X&Y)
	to include changes in the pattern over the animals body.
***/
void segmentImageSVM(const CvSVM& svm, Mat& inputIm, Mat& outputIm)
{
	Mat sampleMat(1, 3, CV_32FC1);

	// init pointers to memory of sample for SVM classification
	float *ptr0 = sampleMat.ptr<float>(0);
	float *ptr1 = ptr0+1;
	float *ptr2 = ptr0+2;

	for(int y=0; y<outputIm.rows; y++)
	{
		unsigned int  *rowIn  = inputIm.ptr<unsigned int>(y);
		unsigned char *rowOut = outputIm.ptr<unsigned char>(y);

		for(int x = 0; x < outputIm.cols; x++, rowIn++, rowOut++)
		{
			// put RGB values in sample
			float r = (float) (*rowIn & 0xff);
			float g = (float) ((*rowIn >> 8)  & 0xff);
			float b = (float) ((*rowIn >> 16) & 0xff);

			// direct operations in memory for efficiency
			*ptr0 = r;
			*ptr1 = g;
			*ptr2 = b;

			float response = svm.predict(sampleMat);

			if (response == 1)
				*rowOut = 255;
			else
				*rowOut = 0;
		}
	}

	// some morphological operations to remove noise
	int erosion_size = 2;
	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2*erosion_size + 1, 2*erosion_size + 1), Point(erosion_size, erosion_size));
	Mat result, result2;
	cv::erode(outputIm, result, element, Point(-1,-1), 1);
	cv::dilate(result, result2, element, Point(-1,-1), 2);
	cv::erode(result2, outputIm, element, Point(-1,-1), 1);
}