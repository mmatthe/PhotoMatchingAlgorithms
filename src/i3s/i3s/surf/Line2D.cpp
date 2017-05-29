#include <math.h>
#include "Line2D.hpp"

#define sqr(a) (a)*(a)

Line2D::Line2D(double _x1, double _y1, double _x2, double _y2)
{
    x1 = _x1;
    y1 = _y1;
    x2 = _x2;
    y2 = _y2;
    
    lea = y2 - y1;
    leb = x1 - x2;
    lec = -(lea * x1 + leb * y1);
     
    v[0] = x2 - x1;
    v[1] = y2 - y1;
}

const bool Line2D::intersect(const Line2D& l, double& x, double& y)
{
    /* first check for parallel lines */
    if(l.lea * leb == l.leb * lea)  /* parallel */
        return false;
       
    double a = l.lea;
    double b = l.leb;
    double c = l.lec;
      
    double lambda = -1.0 * (c + a * x1 + b * y1) / (a * v[0] + b * v[1]);
    x = (lambda * v[0] + x1);
    y = (lambda * v[1] + y1);
    
    return true;
}
const void Line2D::project_on_line(double origx, double origy, double& x, double& y)
{
   Line2D *l = new Line2D(origx, origy, origx + v[1], origy - v[0]);
   intersect(*l, x, y);
}
const double Line2D::square_distance(double x, double y) 
{
	double resx, resy;
	project_on_line(x, y, resx, resy);

	// only accept distance to projected point if it is on the line piece
	if(point_on_line_piece(resx, resy))
		return sqr(resx-x) + sqr(resy-y);

	// otherwise take the minimum distance to one of the end points
	double sqd1 = sqr(x1-x) + sqr(y1-y);
	double sqd2 = sqr(x2-x) + sqr(y2-y);

	if(sqd1 < sqd2)
		return sqd1;
	else
		return sqd2;
}
const bool Line2D::point_on_line_piece(double x, double y)
{
    double v1[2];
    double v2[2];
     
    v1[0] = x - x1;
    v1[1] = y - y1;
    v2[0] = x - x2;
    v2[1] = y - y2;
     
    if(v1[0]*v2[0] <= 0.000001 && v1[1]*v2[1] <= 0.000001)
       return true;         // point between p1 and p2
    else
       return false;
}
    

