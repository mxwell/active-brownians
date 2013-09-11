#include "cluster.h"

Cluster::Cluster(const int& N, const ld& L, const ld& epsilon):
	N(N), L(L), epsilon(epsilon)
{
	r.resize(N);
	v.resize(N);	
	seeded = false;
}

Cluster::~Cluster()
{
	r.clear();
	v.clear();
}

void Cluster::seed_randomly(const ld& speed_lowest,
							const ld& speed_highest)
{
	const ld center = 0.5 * L;
	const ld magnitude = 0.1 * L;

	auto gauss = [] () {
		return GaussianGen::Instance().value(default_gen);
	};

	for (auto it = r.begin(); it != r.end(); ++it) {
		ld x = center + gauss() * magnitude;
		ld y = center + gauss() * magnitude;
		*it = Point(x, y);
	}


	const ld mean_speed = (speed_lowest + speed_highest) / 2;
	const ld speed_magnitude = (speed_highest - speed_lowest) / 10;
	for (auto it = v.begin(); it != v.end(); ++it) {
		ld vx = mean_speed + gauss() * speed_magnitude;
		ld vy = mean_speed + gauss() * speed_magnitude;
		*it = Point(vx, vy);
	}
	seeded = true;
}

void Cluster::evolve(speed_integrator speed_step,
		position_integrator position_step)
{
	Point u_A = get_avg_speed();
	for (int i = 0; i < N; ++i) {
		r[i] = position_step(r[i], v[i]);
		r[i].normalize_to_rect(0, L, 0, L);
		v[i] = speed_step(v[i], u_A);
	}
}

Point Cluster::get_mean_field_speed(const Point& particle)
{
	Point virtuals[8];
	int virtuals_count = 0;
	virtuals[virtuals_count++] = particle;
	/* through the edges */
	if (particle.distance_to_vertical(0) < epsilon)
		virtuals[virtuals_count++] = particle + Point(L, 0);
	if (particle.distance_to_vertical(L) < epsilon)
		virtuals[virtuals_count++] = particle + Point(-L, 0);
	if (particle.distance_to_horizontal(0) < epsilon)
		virtuals[virtuals_count++] = particle + Point(0, L);
	if (particle.distance_to_horizontal(L) < epsilon)
		virtuals[virtuals_count++] = particle + Point(0, -L);
	/* through the angles */
	if ((particle - Point(0, 0)).length() < epsilon)
		virtuals[virtuals_count++] = particle + Point(L, L);
	if ((particle - Point(0, L)).length() < epsilon)
		virtuals[virtuals_count++] = particle + Point(L, -L);
	if ((particle - Point(L, 0)).length() < epsilon)
		virtuals[virtuals_count++] = particle + Point(-L, L);
	if ((particle - Point(L, L)).length() < epsilon)
		virtuals[virtuals_count++] = particle + Point(-L, -L);
	/* epsilon shoulda be small enough */
	assert(virtuals_count < 5);

	Point field_speed(0, 0);
	int particles = 0;
	for (int i = 0; i < N; ++i) {
		bool in_field = false;
		Point& p = r[i];
		for (int j = 0; j < virtuals_count; ++j) {
			if ((p - virtuals[j]).length() < epsilon) {
				in_field = true;
				break;
			}
		}
		if (!in_field)
			continue;
		++particles;
		field_speed = field_speed + v[i];
	}
	return field_speed * (1. / particles);
}

Point Cluster::get_avg_speed()
{
	Point avg_speed(0,0);
	for (int i = 0; i < N; ++i) {
		avg_speed = avg_speed + v[i];
	}
	return avg_speed * (1. / N);
}

void Cluster::init_log(const char *log_file)
{
	log = fopen(log_file, "wt");
	assert(log != NULL);
}

void Cluster::exit_log()
{
	if (log != NULL)
		fclose(log);
	log = NULL;
}

void Cluster::log_positions()
{
	for (size_t i = 0; i < r.size(); ++i) {
		r[i].to_string(buffer, ' ', '\t');
		fprintf(log, "%s", buffer);
	}
	fprintf(log, "\n");
}
