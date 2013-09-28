/**
 * Implementation of spatial partition,
 * see http://gameprogrammingpatterns.com/spatial-partition.html
 */

#include "particle.h"

Particle::Particle(int id, double x, double y) :
	id(id), x(x), y(y)
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

const int Particle::getId() const
{
	return id;
}

void Particle::move_to(const double &nx, const double &ny)
{
	x = nx;
	y = ny;
}
