#ifndef __SSU_KMY_POINT_H_
#define __SSU_KMY_POINT_H_

#include <types.h>
#include <cmath>
#include <cstdio>

class Point
{
	friend class Grid;
	friend class Cluster;
public:
	Point() {}
	Point(ld x, ld y) : _x(x), _y(y) {}
	friend const Point operator+(const Point&, const Point&);
	friend const Point operator-(const Point&, const Point&);
	friend const Point operator*(const ld&, const Point&);
	friend const Point operator*(const Point&, const ld&);
	friend const Point operator/(const Point&, const ld&);
	/* Hadamard product */
	const Point operator*(const Point& other);
	ld length() const;
	const Point get_unit() const;
	const Point rotate(const ld& angle);
	/* rotate by PI / 2 */
	const Point get_normal() const;
	/* @vertical means line with x = @vertical */
	ld distance_to_vertical(const ld& vertical) const;
	/* @horizontal means line with y = @horizontal */
	ld distance_to_horizontal(const ld& horizontal) const;
	void to_string(char *, const char, const char) const;
	void normalize_to_rect(const ld&, const ld&, const ld&, const ld&);
private:
	ld _x;
	ld _y;
};

#endif /* __SSU_KMY_POINT_H_ */
