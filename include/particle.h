/**
 * Implementation of spatial partition,
 * see http://gameprogrammingpatterns.com/spatial-partition.html
 */

#ifndef __SSU_KMY_PARTICLE_H_
#define __SSU_KMY_PARTICLE_H_

#include <vector>
#include "point.h"

class Particle {

public:
	Particle(int id, double x, double y);

	/*int get_id() const;*/
	double get_x() const;
	double get_y() const;
	const int getId() const;

	void move_to(const double &nx,
				 const double &ny);

	/* public to be accessed by Grid methods */
	Particle *next;
	Particle *prev;

private:
	/* model information */
	int id;
	double x;
	double y;
};

#endif /* __SSU_KMY_PARTICLE_H_ */