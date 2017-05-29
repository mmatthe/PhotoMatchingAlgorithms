#include <math.h>

#define PI 3.14159265


class Line2D {
private:
    double lea, leb, lec;
    double x1, y1, x2, y2;
    double v[2];

public:
	Line2D(double _x1, double _y1, double _x2, double _y2);

    const bool intersect(const Line2D& l, double& x, double& y);
    const void project_on_line(double origx, double origy, double& x, double& y);
	const double square_distance(double x, double y);
	const bool point_on_line_piece(double, double);
	const double angle();
	const double length();
};

inline double const Line2D::angle() {
	return atan2 (y2-y1, x2-x1) * 180 / PI;
}

inline double const Line2D::length() {
	double y = y2-y1;
	double x = x2-x1;
	return sqrt (y*y + x*x);
}