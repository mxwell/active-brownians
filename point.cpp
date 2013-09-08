#include <point.h>
#include <cassert>

template<typename T> static inline T sqr(const T& x)
{
	return (x) * (x);
}

const Point operator+(const Point& p, const Point& q)
{
	return Point(p._x + q._x, p._y + q._y);
}

const Point operator-(const Point& p, const Point& q)
{
	return Point(p._x - q._x, p._y - q._y);
}

const Point operator*(const ld& mult, const Point& p)
{
	return Point(p._x * mult, p._y * mult);
}

const Point operator*(const Point& p, const ld& mult)
{
	return Point(p._x * mult, p._y * mult);
}

const Point Point::operator*(const Point& other)
{
	return Point(_x * other._x, _y * other._y);
}

const ld Point::length() const
{
	return sqrt(sqr(_x) + sqr(_y));
}

const Point Point::get_unit() const
{
	ld len = length();
	assert(fabs(len) > 1e-7);
	return Point(_x / len, _y / len);
}

const Point Point::rotate(const ld& angle)
{
	ld cs = cos(angle),
	   sn = sin(angle);
	ld nx = _x * cs - _y * sn,
	   ny = _x * sn + _y * cs;
	return Point(nx, ny);
}

const Point Point::get_normal() const
{
	return Point(-_y, _x);
}

const ld Point::distance_to_vertical(const ld& vertical) const
{
	return fabs(_x - vertical);
}

const ld Point::distance_to_horizontal(const ld& horizontal) const
{
	return fabs(_y - horizontal);
}

void Point::to_string(char *string, const char delim, const char tail)
{
	sprintf(string, "%lf%c%lf%c", _x, delim, _y, tail);
}

void Point::normalize_to_rect(const ld& left, const ld& right,
	const ld& bottom, const ld& top)
{
	ld width = right - left;
	if (_x < left)
		_x += width;
	else if (_x > right)
		_x -= width;
	ld height = top - bottom;
	if (_y < bottom)
		_y += height;
	else if (_y > top)
		_y -= height;
}
