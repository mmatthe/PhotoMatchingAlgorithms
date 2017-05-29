/*******************************************************************************
 *	I3S: Interactive Individual Identification System						   *
 *																			   *
 *	Copyright (C) 2004-2013  Jurgen den Hartog & Renate Reijns				   *
 *																			   *
 *	This program is free software; you can redistribute it and/or modify	   *
 *	it under the terms of the GNU General Public License as published by	   *
 *	the Free Software Foundation; either version 2 of the License, or		   *
 *	(at your option) any later version.										   *
 *																			   *
 *	This program is distributed in the hope that it will be useful,			   *
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of			   *
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			   *
 *	GNU General Public License for more details.							   *
 *																			   *
 *	You should have received a copy of the GNU General Public License		   *
 *	along with this program; see the file COPYING GPL v2.txt. If not,		   *
 *	write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,  *
 *	Boston, MA 02111-1307, USA.												   *
 *																			   *
 *******************************************************************************/


// compare.cpp : Defines the entry point for the DLL application.
//

#include <iostream>
#include "compare.hpp"
#include "affine.h"


// Global parameters to the comparison algorithm
double maxAllowedDistance  = 0.01;
double minRelativeDistance = 1.5;
double minRatioArea        = 0.5;		// maximum difference in ratio areas 0.5 means factor 2 (1 / 2)
double maxAngleDiff        = 40.0;		// maximum difference in angles between two ellipses in degrees
double minRatioRatio       = 0.666;		// ellipse ratios (short side / long side) may maximally differ
double maxFillDiff         = 0.25;		// The maximum difference in the fill ratio (nr of fg pixels divided by all pixels) of each keypoint
double maxRatioToCalcAngle = 0.666;		// before the angle of an ellipse can be calculated the long and short radius
										// must differ by a factor 1.5 minimally, otherwise there is no direction.
										// for the moment it is hard coded and not set by the user

int    I3S_type = 2;					// CLASSIC default (0), SPOT = 1, PATTERN = 2, PATTERN+ = 3

int isUserAccountControlOn();


#ifdef I3SSURF_EXPORTS
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	return TRUE;
}
#endif

static int userMessage (const char *title, const char *mess)
{
  #ifdef I3SSURF_EXPORTS
   return ::MessageBox(NULL, mess, title, MB_OK | MB_ICONWARNING | MB_APPLMODAL | MB_SETFOREGROUND );
   #else
   std::cerr << title << mess << std::endl;
   #endif
}


double compareTwo_Impl(double* uref, double* udata, int unr,
						double* kref, double* kdata, int knr,
						long* pairs, int nPairs)
{
  	I3S_type = 3;
	// std::cout << maxAllowedDistance << std::endl;
	// std::cout << minRelativeDistance << std::endl;
	// std::cout << minRatioArea << std::endl;
	// std::cout << maxAngleDiff << std::endl;
	// std::cout << minRatioRatio << std::endl;
	// std::cout << maxFillDiff << std::endl;


	// std::cout << "start... compare" << std::endl;
	// for (int i = 0; i < unr; i++)
	// {
	// 	for (int j = 0; j < 9; j++)
	// 		std::cout << ((double*)udata)[9 * i + j] << " ";
	// 	std::cout << std::endl;
	// }
	// std::cout << "end compare" << std::endl;

	FingerPrint unknown((double*)uref, (double*)udata, (int)unr);
	FingerPrint known((double*)kref, (double*)kdata, (int)knr);

	if (pairs)
	  {
	    for (int i = 0; i<nPairs; i++)
	      pairs[i] = -1;
	  }

	Pair *mypairs = new Pair[nPairs];
	int paircnt = 0;

	if (Compare::compareTwo(unknown, known, mypairs, &paircnt) == false)
		return false;

	if (pairs)
	  {
	    for (int i = 0; i<paircnt; i++)
	      pairs[i] = mypairs[i].getM1() * 1000 + mypairs[i].getM2();
	  }

	delete[] mypairs;
	// known.toArray(kdata);
	// unknown.toArray(udata);

	return known.getScore();
}
