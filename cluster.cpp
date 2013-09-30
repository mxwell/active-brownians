#include "cluster.h"

Cluster::Cluster(const int& N, const ld& L, bool local_visibility, const ld& epsilon,
	bool to_use_grid)
{
	grid = nullptr;
	calculate_with_grid = to_use_grid;
	reinit(N, L, local_visibility, epsilon);
}

Cluster::~Cluster()
{
	for (int i = 0; i < 2; ++i) {
		rs[i].clear();
		vs[i].clear();
	}
}

void Cluster::use_grid(bool yes)
{
	calculate_with_grid = yes;
}

void Cluster::reinit(const int &N_arg, const ld &L_arg,
		bool local_visibility_arg, const ld &epsilon_arg)
{
	N = N_arg;
	L = L_arg;
	local_visibility = local_visibility_arg;
	epsilon = epsilon_arg;
	for (int i = 0; i < 2; ++i) {
		rs[i].resize(N);
		vs[i].resize(N);
	}
	cur_id = 0;
	next_id = 1;
	seeded = false;
	measurement = false;
	if (!calculate_with_grid)
		return;
	int cells = 5 * L / epsilon;
	if (grid != nullptr)
		delete grid;
	grid = new Grid(L, L, cells, cells);
	grid_updated = false;
	particles.clear();
}

void Cluster::seed_randomly(const ld& speed_lowest,
			    const ld& speed_highest)
{
	const ld center = 0.5 * L;
	const ld magnitude = 0.1 * L;
	std::vector<Point> &r = get_next_coordinates();
	std::vector<Point> &v = get_next_velocities();

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
	swap_states();
}

void Cluster::seed_uniformly(const ld &speed_lowest,
			     const ld &speed_highest)
{
	std::vector<Point> &r = get_next_coordinates();
	std::vector<Point> &v = get_next_velocities();
	auto ran3 = [] () { return GaussianGen::Instance().ran3_value(); };
	for (auto it = r.begin(); it != r.end(); ++it) {
		ld x = 0.01 + ran3() * (L - 0.02);
		ld y = 0.01 + ran3() * (L - 0.02);
		*it = Point(x, y);
	}

	const ld speed_range = (speed_highest - speed_lowest);
	for (auto it = v.begin(); it != v.end(); ++it) {
		ld vx = ran3() * speed_range + speed_lowest;
		ld vy = ran3() * speed_range + speed_lowest;
		*it = Point(vx, vy);
	}
	seeded = true;
	swap_states();
}

void Cluster::update_grid()
{
	std::vector<Point> &r = get_cur_coordinates();
	std::vector<Point> &v = get_cur_velocities();
	size_t amount = r.size();
	if (particles.empty()) {
		particles.resize(amount);
		for (size_t i = 0; i < amount; ++i) {
			Particle *particle = new Particle(i, r[i]._x, r[i]._y);
			particles[i] = particle;
			grid->add(particle);
		}
	} else {
		for (size_t i = 0; i < amount; ++i)
			grid->move(particles[i], r[i]);
	}
	grid->update_for_search(&v);
	grid_updated = true;
}

std::vector<Point> &Cluster::get_cur_coordinates()
{
	return rs[cur_id];
}

std::vector<Point> &Cluster::get_next_coordinates()
{
	return rs[next_id];
}

std::vector<Point> &Cluster::get_cur_velocities()
{
	return vs[cur_id];
}

std::vector<Point> &Cluster::get_next_velocities()
{
	return vs[next_id];
}

void Cluster::swap_states()
{
	std::swap(cur_id, next_id);
	grid_updated = false;
}

void Cluster::evolve(speed_integrator speed_step,
		position_integrator position_step)
{
	std::vector<Point> &r = get_cur_coordinates();
	std::vector<Point> &rnext = get_next_coordinates();
	std::vector<Point> &v = get_cur_velocities();
	std::vector<Point> &vnext = get_next_velocities();
	if (local_visibility) {
		for (int i = 0; i < N; ++i) {
			Point u_A;
			if (calculate_with_grid)
				u_A = get_disc_speed_with_grid(i);
			else
				u_A = get_mean_field_speed(i);
			rnext[i] = position_step(r[i], v[i]);
			rnext[i].normalize_to_rect(0, L, 0, L);
			vnext[i] = speed_step(v[i], u_A);
		}
	} else {
		Point u_A = get_avg_speed();
		if (measurement) {
			avg_speed += u_A.length();
			++avg_denominator;
		}
		for (int i = 0; i < N; ++i) {
			rnext[i] = position_step(r[i], v[i]);
			rnext[i].normalize_to_rect(0, L, 0, L);
			vnext[i] = speed_step(v[i], u_A);
		}
	}
	swap_states();
}

Point Cluster::get_mean_field_speed(int particleId)
{
	Point particle = get_cur_coordinates()[particleId];
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
	/* epsilon should be small enough */
	assert(virtuals_count < 5);

	Point field_speed(0, 0);
	particles_found_naive = 0;
	std::vector<Point> &r = get_cur_coordinates();
	std::vector<Point> &v = get_cur_velocities();
	for (int i = 0; i < N; ++i) {
		if (i == particleId)
			continue;
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
		++particles_found_naive;
		field_speed = field_speed + v[i];
	}
	if (particles_found_naive == 0)
		return field_speed;
	return field_speed / particles_found_naive;
}

Point Cluster::get_disc_speed_with_grid(int particleId)
{
	if (!grid_updated)
		update_grid();

	return grid->get_disc_speed(*(particles[particleId]), epsilon);
}

Point Cluster::get_avg_speed() const
{
	const std::vector<Point> &v = vs[cur_id];
	Point avg_speed(0,0);
	for (int i = 0; i < N; ++i) {
		avg_speed = avg_speed + v[i];
	}
	return avg_speed * (1. / N);
}

ld Cluster::get_avg_speed_val() const
{
	return get_avg_speed().length();
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
	assert(log);
	std::vector<Point> &r = rs[cur_id];
	for (size_t i = 0; i < r.size(); ++i) {
		r[i].to_string(buffer, ' ', '\t');
		fprintf(log, "%s", buffer);
	}
	fprintf(log, "\n");
}

void Cluster::start_speed_measurement()
{
	measurement = true;
	avg_denominator = 0;
	avg_speed = 0.0;
}

ld Cluster::get_measurement() const
{
	if (avg_denominator == 0)
		return -1.0;
	return avg_speed / avg_denominator;
}
