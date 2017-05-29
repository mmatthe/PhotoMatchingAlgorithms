/*XXX******************************************************************************
 *   I3S: Interactive Individual Identification System                         *
 *                                                                             *
 *   Copyright (C) 2004-2013  Jurgen den Hartog & Renate Reijns                *
 *                                                                             *
 *   This program is free software; you can redistribute it and/or modify      *
 *   it under the terms of the GNU General Public License as published by      *
 *   the Free Software Foundation; either version 2 of the License, or         *
 *   (at your option) any later version.                                       *
 *                                                                             *
 *   This program is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 *   GNU General Public License for more details.                              *
 *                                                                             *
 *   You should have received a copy of the GNU General Public License         *
 *   along with this program; see the file COPYING GPL v2.txt. If not,         *
 *   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, *
 *   Boston, MA 02111-1307, USA.                                               *
 *                                                                             *
 *******************************************************************************/

// dllmain.cpp : Defines the entry point for the DLL application.
#include <iostream>
#include <stdio.h>

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>		// only for imwrite used when debugging
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/ml/ml.hpp>

#define sqr(a) ((a)*(a))


// external function to calculate the keypoints
int	calcSurfFeatures(const std::vector<cv::Point2f>& roi, const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors, int nrKeyPoints, float minDistBetweenKeypoints);

// external functions to calculate affine transformation matrix and carrying out the transform
void        calcAffine(float from1x, float from1y, float from2x, float from2y, float from3x, float from3y,
				       float to1x, float to1y, float to2x, float to2y, float to3x, float to3y, float *matrix);
cv::Point2f doAffine(float x, float y, float matrix[6]);
void        doAffine(float x, float y, float matrix[6], float& outx, float& outy);

// to be deleted in later version, moved to i3s_compare.dll
int isUserAccountControlOn();

void segmentImageSVM(const CvSVM& svm, cv::Mat& inputIm, cv::Mat& outputIm);
void trainSVM(const cv::Mat& trainingData, const cv::Mat& labels, const cv::Mat& inputIm, CvSVM& svm);

// local conversion functions
void dataToMatrixImage(signed char *data, int width, int height, cv::Mat& image);
void dataToMatrixImage(int *data,         int width, int height, cv::Mat& image);
void matrixImageToData(cv::Mat& image, char *data);
uchar getChannel(const cv::Mat& im, int x, int y, int shift);

// local helper functions
float maxDist(const std::vector<cv::Point2f>& roi);
void  roiArrayToList(std::vector<cv::Point2f>& roi, float* roix, float* roiy, float* matrix, float& maxX, float& maxY);
void userMessage(const char *mess, const char *title);


#ifdef I3SSURF_EXPORTS
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{

	std::cout << "dllmain!" << std::endl;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#endif

// don't forget to change java.h if anything changes in the parameters or a new function is added

void calcSurfFeatures_Impl(signed char* imIn, int width, int height, int nrElts, float relKeypointDist,
							float* roix, float* roiy,
							float* x, float* y, float* size, float* descr,
							float warpSize)
{
	cv::Mat image, descriptors;
	std::vector<cv::KeyPoint> keypoints(10000);		// required otherwise dll crashes...
	std::vector<cv::Point2f> roi;

	float matrix[6], invmatrix[6];

	// int ii = 0;
	// while(true)
	//   {
	//     // std::cout << roix[ii] << " " << roiy[ii] << "\n";
	//     if (roix[ii] == roix[0] && roiy[ii] == roiy[0] && ii > 0)
	//       break;
	//     ii++;
	//   }
	// std::cout << relKeypointDist << " " << nrElts << " " << warpSize << std::endl;
	// std::cout << width << " " << height << std::endl;
	// for (int r = 0; r < height; r++)
	// {
	// 	for (int c = 0; c < width; c++)
	// 	{
	// 		char x = imIn[r*width + c];
	// 		if (x)
	// 			std::cout << ".";
	// 		else
	// 			std::cout << "X";
	// 	}
	// 	std::cout << "\n";
	// }

	// both matrices are initialized to unity matrix
	matrix[0] = invmatrix[0] = 1;
	matrix[1] = invmatrix[1] = 0;
	matrix[2] = invmatrix[2] = 0;
	matrix[3] = invmatrix[3] = 0;
	matrix[4] = invmatrix[4] = 1;
	matrix[5] = invmatrix[5] = 0;

	float maxX = 0, maxY = 0;
	roiArrayToList(roi, roix, roiy, matrix, maxX, maxY);

	float ratio = warpSize / maxDist(roi);		// warp images to about the same size, in the default case about 500 pix

	if (ratio < 1) {
		calcAffine(0, 1, 1, 0, 1, 1,
			0, ratio, ratio, 0, ratio, ratio, matrix);
		calcAffine(0, ratio, ratio, 0, ratio, ratio,
			0, 1, 1, 0, 1, 1, invmatrix);
		roiArrayToList(roi, roix, roiy, matrix, maxX, maxY);
	}

	dataToMatrixImage(imIn, width, height, image);	// construct openCV image from array

													// preparations for warping image
	cv::Mat affImage;
	cv::Mat m(2, 3, CV_32F);

	m.at<float>(0, 0) = matrix[0];
	m.at<float>(0, 1) = matrix[1];
	m.at<float>(0, 2) = matrix[2];
	m.at<float>(1, 0) = matrix[3];
	m.at<float>(1, 1) = matrix[4];
	m.at<float>(1, 2) = matrix[5];

	cv::Size sz((int)maxX + 400, (int)maxY + 400);

	// Warp the image
	cv::warpAffine(image, affImage, m, sz);

	calcSurfFeatures(roi, affImage, keypoints, descriptors, nrElts, maxDist(roi) * relKeypointDist);    // % of max distance in roi is minimum required distance between keypoints

	unsigned int i = 0;
	unsigned int max = 1000;
	if (keypoints.size() < max)
		max = keypoints.size();

	for (i = 0; i<max; i++) {
		// put the location and size of keypoints in image coordinates directly behind the coordinates in affine scale space
		doAffine(keypoints[i].pt.x, keypoints[i].pt.y, invmatrix, x[i], y[i]);

		// calculate the size of each keypoint in image coordinates
		float testx, testy;
		doAffine(keypoints[i].pt.x + keypoints[i].size / 2, keypoints[i].pt.y, invmatrix, testx, testy);
		size[i] = sqrt(sqr(testx - x[i]) + sqr(testy - y[i]));
	}
	x[max] = -1000;	     // set to impossible value so Java knows the end is reached
	y[max] = -1000;
	size[max] = -1000;

	int index = 0;
	for (int j = 0; j<max; j++) {
		float *ptr = descriptors.ptr<float>(j);
		for (int i = 0; i<128; i++, index++) {
			descr[index] = ptr[i];
		}
	}
	// std::cout << "start... surf out" << std::endl;
	// for (int i = 0; i < max; i++)
	// 	std::cout << x[i] << " " << y[i] << " " << size[i] << " " << descr[i * 128] << std::endl;
	// std::cout << "end surf out" << std::endl;
	return;

	cv::Mat img = image / 4;
	for(int i = 0; i < nrElts; i++)
	  {
	    cv::circle(img, cv::Point(x[i], y[i]), size[i]/2, cv::Scalar(255));
	  }
	cv::imshow("surf", img);
	cv::waitKey(0);



}


// return a specific channel given an image and x,y coordinates
uchar getChannel(const cv::Mat& im, int x, int y, int shift) {
	int val = im.at<int>(y, x);
	return (val >> shift) & 0xff;
}




/***
	Calculate maximum distance between the points which make up the ROI. This is used as indication of the size of the area in pixels.
***/
float maxDist(const std::vector<cv::Point2f>& roi) {
	float max = -1;

	for(unsigned int i=0; i<roi.size()-1; i++) {
		for(unsigned int j=i+1; j<roi.size(); j++) {
			float d = sqr(roi[j].x - roi[i].x) + sqr(roi[j].y - roi[i].y);
			if(d > max)
				max = d;
		}
	}
	return sqrt(max);
}

void roiArrayToList(std::vector<cv::Point2f>& roi, float* roix, float* roiy, float* matrix, float& maxX, float& maxY)
{
	maxX = 0;
	maxY = 0;
	bool negative = false;
	int i=0;
	roi.clear();

	// convert to list and do affine at the same time
	// in the input the first and last vertex are identical. the algorithm which decides whether a point is inside assumes all vertices are unique.
	// so the first point is skipped

	do {
		i++;
		cv::Point2f p = doAffine(roix[i], roiy[i], matrix);

		if(p.x > maxX)
			maxX = p.x;
		if(p.y > maxY)
			maxY = p.y;
		if(p.y < 0 || p.x < 0)
			negative = true;
		roi.push_back(p);
	} while(roix[i] != roix[0] || roiy[i] != roiy[0]);

	if(negative)
		std::cerr << "Warning: roi outside image range!\n";
}

void dataToMatrixImage(signed char *data, int width, int height, cv::Mat& image)
{
	image.create(height, width, CV_8U);		// create image of unsigned chars

	int index = 0;
	for(int j=0; j<height; j++) {
		unsigned char *ptr = image.ptr<unsigned char>(j);

		for(int i=0; i<width; i++, index++)
			ptr[i] = (unsigned char) data[index];
	}
}

void dataToMatrixImage(int *data, int width, int height, cv::Mat& image)
{
	image.create(height, width, CV_32S);		// create image of 4 channels of unsigned chars

	int index = 0;
	for(int j=0; j<height; j++)
	{
		int *ptr = image.ptr<int>(j);

		for(int i=0; i<width; i++, index++)
			ptr[i] = data[index];
	}
}

void matrixImageToData(cv::Mat& image, char *data)
{
	if(image.type() != CV_8U)
	{
		fprintf(stderr, "matrixImageToData(cv::Mat&, int*): Parameter error.");
		return;
	}
	int index = 0;
	for(int j=0; j<image.size().height; j++) {
		unsigned char *ptr = image.ptr<unsigned char>(j);
		memcpy(data+j*image.size().width, ptr, image.size().width);
	}
}
