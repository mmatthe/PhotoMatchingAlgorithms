/*******************************************************************************
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

// I3SSurf.cpp : Defines the exported functions for the DLL application.
//
#include "Line2D.hpp"

#include <iostream>

#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>		// only for imwrite used when debugging
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/legacy/legacy.hpp>

#define sqr(a) ((a)*(a))

double sqDistance(const std::vector<cv::Point2f>& pgon, const cv::Point2f& test);
bool   inside(const std::vector<cv::Point2f>& pgon, const cv::Point2f& test);
void   filterKeyPoints(const std::vector<cv::Point2f>& roi, std::vector<cv::KeyPoint>& keypoints, int nrKeyPoints, float minDistBetweenKeypoints);

float averageValueKeypoint(const cv::Mat& image, int cx, int cy, int radius);
void scanLines(const cv::Mat& image, int cx, int cy, int x, int y, long& accValue, int& totPix);
void scanLine(const cv::Mat& image, int startx, int starty, int nr, long& accValue);

bool keyComp(cv::KeyPoint k1, cv::KeyPoint k2);

wchar_t* charConvert(const char *s);
void userMessage(const char *mess, const char *title);	// debugging only


int calcSurfFeatures(const std::vector<cv::Point2f>& roi, const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, cv::Mat& descriptors,
	                 int nrKeyPoints, float minDistBetweenKeypoints)
{

	if(!image.data)
		return 0;

	double thr = 300;

	do {
		// Construction of the SURF feature detector
		cv::SurfFeatureDetector surf(  thr,  // Hessian threshold
									     4,  // The number of a gaussian pyramid octaves that the detector uses. It is set to 4 by default.
										     // If you want to get very large features, use the larger value.
										     // If you want just small features, decrease it.
									     2,  // The number of images within each octave of a gaussian pyramid. It is set to 2 by default.
									 false,  // false means that the basic descriptors (64 elements each) shall be computed. JdH: does not have any effect???
									         // true means that the extended descriptors (128 elements each) shall be computed
									 false); // false means that detector computes orientation of each feature. true means that the orientation is not computed
											 // (which is much, much faster). For example, if you match images from a stereo pair, or do image stitching,
											 // the matched features likely have very similar angles, and you can speed up feature extraction by setting upright=true.

		// Detection of the SURF features
		keypoints.resize(10000);	// just to be really sure, bug in C++ STL/OpenCV 2.4. there must be enough room for keypoints before calculation starts otherwise it crashes
		//surf.
		surf.detect(image, keypoints);
		filterKeyPoints(roi, keypoints, nrKeyPoints, minDistBetweenKeypoints);	// remove everything outside roi and keep only strongest

		if(thr < 1)
			break;

		if(keypoints.size() < (unsigned int) nrKeyPoints)
			thr = 0.75 * thr;
		else
			break;
	} while(1);

	/***
		Keypoint descriptors do not work under water. This part has been removed from the code or commented as below, JdH 31-5-2013
	***/
	// Construction of the SURF descriptor extractor
	//cv::SurfDescriptorExtractor extractor;
	//extractor.compute(image, keypoints, descriptors);
	descriptors.create(keypoints.size(), 128, CV_32F);
	for(unsigned int i=0; i<keypoints.size(); i++) {
		float *ptr = descriptors.ptr<float>(i);
		ptr[0] = averageValueKeypoint(image, (int) keypoints[i].pt.x, (int) keypoints[i].pt.y, (int) keypoints[i].size / 4);
		for(int i=1; i<128; i++)
			ptr[i] = 0;
	}

	return 1;
}


float averageValueKeypoint(const cv::Mat& image, int cx, int cy, int radius)
{
    int error = -radius;
    int x = radius;
    int y = 0;

	long accValue = 0;
	int  totPix = 0;

    while(x >= y)
    {
        int lastY = y;

        error += y;
        ++y;
        error += y;

        scanLines(image, cx, cy, x, lastY, accValue, totPix);

        if (error >= 0)
        {
            if (x != lastY)
                scanLines(image, cx, cy, lastY, x, accValue, totPix);

            error -= x;
            --x;
            error -= x;
        }
    }
	return (float) (accValue / totPix) / 255;	// normalize to value between 0 and 1. Pixel values are 0..255
}

void scanLines(const cv::Mat& image, int cx, int cy, int x, int y, long& accValue, int& totPix)
{
	scanLine(image, cx - x, cy + y, 2*x + 1, accValue);
	totPix += 2*x + 1;
    if (x != 0 && y != 0) {
        scanLine(image, cx - x, cy - y, 2*x + 1, accValue);
		totPix += 2*x + 1;
	}
}

void scanLine(const cv::Mat& image, int startx, int starty, int nr, long& accValue) {
	const unsigned char *ptr = image.ptr<unsigned char>(starty);
	ptr += startx;
	for(int i=0; i<nr; i++, ptr++)
		accValue += *ptr;
}

// filter out everything outside the region of interest and with a small scale
void filterKeyPoints(const std::vector<cv::Point2f>& roi, std::vector<cv::KeyPoint>& keypoints, int nrKeyPoints, float minDistBetweenKeypoints)
{
	// sort keypoints on best response (strongest response first)
	std::sort(keypoints.begin(), keypoints.end(), keyComp);

	// delete everything outside the roi
	for(unsigned int i=0; i<keypoints.size(); ) {
		if(inside(roi, keypoints[i].pt))
			i++;
		else
			keypoints.erase(keypoints.begin()+i);
	}

	// remove everything in the direct neigborhood of a keypoint with higher response, this works because list is sorted
	for(unsigned int i=1; i<keypoints.size(); i++) {
		for(unsigned int j=0; j<i; j++) {
			float dist = sqr(keypoints[j].pt.x - keypoints[i].pt.x) + sqr(keypoints[j].pt.y - keypoints[i].pt.y);
			dist = sqrt(dist);
			if(dist < minDistBetweenKeypoints) {
				keypoints.erase(keypoints.begin()+i);
				i--;
				break;
			}
		}
	}

	if(keypoints.size() > (unsigned int) nrKeyPoints)
		keypoints.erase(keypoints.begin()+nrKeyPoints, keypoints.end());
}

bool inside(const std::vector<cv::Point2f>& pgon, const cv::Point2f& test)
{
	bool res = false;
	unsigned int i, j;
	for(i=0, j=pgon.size()-1; i < pgon.size(); j = i, i++) {
		if (((pgon[i].y > test.y) != (pgon[j].y > test.y)) &&
			(test.x < (pgon[j].x - pgon[i].x) * (test.y-pgon[i].y) / (pgon[j].y-pgon[i].y) + pgon[i].x) )
            res = !res;
	}
	return res;
}

double sqDistance(const std::vector<cv::Point2f>& pgon, const cv::Point2f& test)
{
	double sqDist = 1000000000;

	unsigned int i;
	for(i=0; i < pgon.size()-1; i++) {
		Line2D l(pgon[i].x, pgon[i].y, pgon[i+1].x, pgon[i+1].y);
		double tmp = l.square_distance(test.x, test.y);
		if(tmp < sqDist)
			sqDist = tmp;
	}
	return sqDist;
}

bool keyComp(cv::KeyPoint k1, cv::KeyPoint k2) {
	return k1.response > k2.response;
}



void userMessage(const char *mess, const char *title)
{
	// ::MessageBox(NULL, charConvert(mess), charConvert(title), MB_OK | MB_ICONWARNING | MB_APPLMODAL | MB_SETFOREGROUND );
  std::cerr << mess << title << std::endl;
}


// wchar_t* charConvert(const char *s)
// {
// 	static wchar_t wcstring[1000];

//     // Convert to a wchar_t*
//     size_t origsize = strlen(s) + 1;
//     size_t convertedChars = 0;
//     mbstowcs_s(&convertedChars, wcstring, origsize, s, _TRUNCATE);
//     return wcstring;
// }



/***

	The code below is used to match descriptors following the OpenCV example code. It is kept, just in case.
	However as descriptors do not work with subaquatic photos it is not used anymore.
	Now, we only use keypoints and their sizes.
	JdH, 31-5-2013
***/

void symmetryTest(const std::vector<cv::DMatch>& matches1, int size1, const std::vector<cv::DMatch>& matches2, int size2, std::vector<cv::DMatch>& symMatches);
void ransacTest(double distance, double confidence, const std::vector<cv::DMatch>& matches,
				const std::vector<cv::Point2f>& keypoints1, const std::vector<cv::Point2f>& keypoints2,
				std::vector<cv::DMatch>& outMatches);



double match(const std::vector<cv::Point2f>& keypoints1, const std::vector<cv::Point2f>& keypoints2,
	         const cv::Mat& descriptors1, const cv::Mat& descriptors2, std::vector<cv::DMatch>& matches)
{
	if(keypoints1.size() == 0 || keypoints2.size() == 0)
		return 1000000000.0;

	// Construction of the matcher
	cv::BruteForceMatcher<cv::L2<float> > matcher;
	//cv::BruteForceMatcher<cv::Hamming> matcher;		// possible alternative

	// Match the two images, 1->2 and 2->1
	std::vector<cv::DMatch> matches1(2000);
	std::vector<cv::DMatch> matches2(2000);

	matcher.match(descriptors1,descriptors2, matches1);
	matcher.match(descriptors2,descriptors1, matches2);

	// Remove non-symmetrical matches
	// Watch out, size of matches1 and matches2 is provided separately by size of keypoints lists. This should not be necessary but MSVC/STL is behaving peculiar in the dll version
	// and not in the standalone executable testversion????? JdH, 2012-08-22

    std::vector<cv::DMatch> symMatches;
	symmetryTest(matches1, keypoints1.size(), matches2, keypoints2.size(), symMatches);
	if(symMatches.size() == 0)
		return 1000000000.0;

	// Validate matches using RANSAC
	ransacTest(1, 0.95, symMatches, keypoints1, keypoints2, matches);

	if(matches.size() == 0)
		return 1000000000.0;

	double score = 0;
	for(unsigned int i=0; i<matches.size(); i++)
		score += (double) matches[i].distance;
	score = 10000.0 * score / (matches.size() * matches.size());

	return score;
}



// Identify good matches using RANSAC
void ransacTest(double distance, double confidence, const std::vector<cv::DMatch>& matches,
				const std::vector<cv::Point2f>& keypoints1, const std::vector<cv::Point2f>& keypoints2,
				std::vector<cv::DMatch>& outMatches)
{
	// Convert keypoints into Point2f
	std::vector<cv::Point2f> points1, points2;
	for(std::vector<cv::DMatch>::const_iterator it= matches.begin(); it!= matches.end(); ++it) {
		points1.push_back(keypoints1[it->queryIdx]);
		points2.push_back(keypoints2[it->trainIdx]);
	}

	// Compute F matrix using RANSAC
	std::vector<uchar> inliers(points1.size(), 0);
	cv::Mat fundemental= cv::findFundamentalMat(cv::Mat(points1), cv::Mat(points2), // matching points
												inliers,                            // match status (inlier or outlier)
												CV_FM_RANSAC,                       // RANSAC method
												distance,                           // max distance to epipolar line
												confidence);                        // confidence probability

	// extract the surviving (inliers) matches
	std::vector<uchar>::const_iterator itIn= inliers.begin();
	std::vector<cv::DMatch>::const_iterator itM= matches.begin();

	for( ;itIn!= inliers.end(); ++itIn, ++itM) {
		if (*itIn)    // it is a valid match
			outMatches.push_back(*itM);
	}
}


// Only keep symmetrical matches
void symmetryTest(const std::vector<cv::DMatch>& m1, int size1, const std::vector<cv::DMatch>& m2, int size2, std::vector<cv::DMatch>& symMatches)
{
	// for all matches image 1 -> image 2
	for (int i=0; i<size1; i++)
	{
		// for all matches image 2 -> image 1
		for (int j=0; j<size2; j++)
		{
			// Match symmetry test
			if (m1[i].queryIdx == m2[j].trainIdx  && m2[j].queryIdx == m1[i].trainIdx)
			{
				// add symmetrical match
				symMatches.push_back(cv::DMatch(m1[i].queryIdx, m1[i].trainIdx, m1[i].distance));
				break; // next match in image 1 -> image 2
			}
		}
	}
}
