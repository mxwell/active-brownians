/**
 * Implementation of spatial partition,
 * see http://gameprogrammingpatterns.com/spatial-partition.html
 */

#include "particle.h"

Particle::Particle(int id, std::vector<Point> *velocities,
	double x, double y) :
	id(id), velocities(velocities), x(x), y(y)
{
	next = prev = NULL;
}

double Particle::get_x() const
{
	return x;
}

double Particle::get_y() const
{
	return y;
}

const Point &Particle::get_velocity() const
{
	return (*velocities)[id];
}

void Particle::move_to(const double &nx, const double &ny)
{
	x = nx;
	y = ny;
}
