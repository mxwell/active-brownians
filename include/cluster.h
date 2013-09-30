#ifndef __SSU_KMY_CLUSTER_H_
#define __SSU_KMY_CLUSTER_H_

#include <cstdio>
#include <cassert>
#include <vector>
#include "point.h"
#include "grid.h"
#include "gaussian_gen.h"

typedef Point (*speed_integrator)(Point, Point);
typedef Point (*position_integrator)(Point, Point);

/**
 * Cluster of @N active Brownian particles
 * on the rectangle area LxL 
 */

class Cluster
{
public:
	Cluster(const int &N, const ld &L,
		bool local_visibility,
		const ld &epsilon = 0, bool to_use_grid = false);
	~Cluster();

	void reinit(const int &N, const ld &L,
			bool local_visibility,
			const ld &epsilon = 0);
	void seed_randomly(const ld& speed_lowest,
			const ld& speed_highest);
	void seed_uniformly(const ld &speed_lowest,
			const ld &speed_highest);
	void evolve(speed_integrator, position_integrator);
	void init_log(const char *log_file);
	void exit_log();
	void log_positions();

	void start_speed_measurement();
	ld get_measurement() const;

	ld get_avg_speed_val() const;
	void use_grid(bool yes);
private:
	int N;
	ld L;
	bool local_visibility;
	ld epsilon;
	bool seeded;
	std::vector<Point> rs[2];
	std::vector<Point> vs[2];
	int cur_id;
	int next_id;
	FILE *log;
	char buffer[128];
	bool measurement;
	ld avg_speed;
	int avg_denominator;

	Point get_mean_field_speed(int particleId);
	Point get_disc_speed_with_grid(int particleId);
	Point get_avg_speed() const;

	std::vector<Point> &get_cur_coordinates();
	std::vector<Point> &get_next_coordinates();
	std::vector<Point> &get_cur_velocities();
	std::vector<Point> &get_next_velocities();
	void swap_states();
	void update_grid();

	Grid *grid;
	bool grid_updated;
	std::vector<Particle *> particles;

	int particles_found_naive;
	bool calculate_with_grid;
};

#endif /* __SSU_KMY_CLUSTER_H_ */
