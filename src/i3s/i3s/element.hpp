/*******************************************************************************
 *   I3S: Interactive Individual Identification System                         *
 *                                                                             *
 *   Copyright (C) 2004-2008  Jurgen den Hartog & Renate Reijns                *
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

#ifndef ELEMENT_HPP
#define ELEMENT_HPP 1

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include "point2D.hpp"

class Element    
{
friend class FingerPrint;

public:
   Element();
   Element(const Element&);
   ~Element() {};
   const Element& operator=(const Element&);

   void doAffine(double *matrix);

   double getCx()       const { return cx; };
   double getCy()       const { return cy; };
   double getArea()     const { return area; };
   double getRatio() const { return ratio; };
   void set(double, double);
   void set(double, double, double);
   void set(const double*);

   double sqrDist(const Element&) const;
   double getDist(const Element&) const;
   double calcSimilarityRate(const Element&) const;

   const Point2D& getData(int i) const { return data[i]; };
   bool matches(const Element&) const;

private:
   void calcShapeAndArea();
   bool isSingular() const;

private:
   Point2D data[4];
   double cx, cy;
   double area;
   double ratio;
   double angle;
   double fgratio;	// percentage of keypoint consisting of foreground pixels, only used by Pattern+
};

inline Element::Element(const Element& e)
{
   cx    = e.cx;
   cy    = e.cy;
   area  = e.area;
   ratio = e.ratio;
   fgratio = e.fgratio;
   angle = e.angle;
   memcpy(data, e.data, sizeof(Point2D)*4);
}

inline const Element& Element::operator=(const Element& e)
{
   cx    = e.cx;
   cy    = e.cy;
   area  = e.area;
   ratio = e.ratio;
   fgratio = e.fgratio;
   angle = e.angle;
   memcpy(data, e.data, sizeof(Point2D)*4);

   return *this;
}

inline Element::Element()
{
   cx    = DBL_INIT;
   cy    = DBL_INIT;
   area  = 0;
   ratio = -1;
   fgratio = -1;
   angle = -1;
}

inline void Element::set(double _x, double _y)
{
   cx    = _x;
   cy    = _y;
   area  = 0;
   ratio = 1;
   angle = -1;
   fgratio = -1;
   data[0].set(_x, _y);
   data[1].set(_x, _y);
   data[2].set(_x, _y);
   data[3].set(_x, _y);
}

inline void Element::set(const double *_d)
{
   data[0].set(_d[0], _d[1]);
   data[1].set(_d[2], _d[3]);
   data[2].set(_d[4], _d[5]);
   data[3].set(_d[6], _d[7]);
   fgratio = _d[8];	// if extra information is added, please also change fingerprint.toArray and the fingerprint constructor

   cx = (data[0].getX() + data[1].getX()) / 2;
   cy = (data[0].getY() + data[1].getY()) / 2;

   calcShapeAndArea();
}
inline bool withinRange(double x1, double x2, double range)
{
   double val = x1 - x2;
   if(val < 0)
	   val = -val;
   return (val <= range);
}
inline bool withinRatio(double x1, double x2, double ratio)
{
   if(x1 < x2) 
   {
      if(x1 / x2 >= ratio)
         return true;
   }
   else
   {
      if(x2 / x1 >= ratio)
         return true;
   }
   return false;
}

inline double Element::sqrDist(const Element& p) const
{
	return (p.cx-cx)*(p.cx-cx) + (p.cy-cy)*(p.cy-cy);
}

inline double Element::getDist(const Element& p) const {
    return sqrt(sqrDist(p));
}
inline bool Element::isSingular() const {
	return data[0].getX() == cx && data[0].getY() == cy;
}

#endif