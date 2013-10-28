#include <cmath>
#include <cstdio>
#include <ctime>

#include <vector>

#include <err.h>

#include <gaussian_gen.h>
#include <point.h>
#include <cluster.h>
#include <luautils.h>
#include <progressbar.h>

using namespace std;

namespace params {
	int relaxation_iterations 	= 5;
	int iterations 				= 400 * 1000;
	int N = 10000;
	ld L_size = 100;
	/* @local_visibility: if true, then a particle can observe
	 * only particles in disk-like area with radius @epsilon,
	 * otherwise any particle can see any other particle
	 */
	bool local_visibility = true;
	/* single value...*/
	ld epsilon = 1;
	/* ...or range if one investigate dependency on epsilon */
	ld epsilon_start = 0.1;
	ld epsilon_end = 1.0;
	ld epsilon_step = 0.1;
	ld epsilon_log_step = -1;

	bool use_grid = false;
	ld mu = 2.5;
	ld mu_start = 1;
	ld mu_end = 9;
	ld mu_step = 4;

	ld D_E = 0.05;
	ld D_v = 0;
	ld D_phi = 0.24;
	ld sqrt2_D_E = sqrt(2 * D_E);
	ld sqrt2_D_v = sqrt(2 * D_v);
	ld sqrt2_D_phi = sqrt(2 * D_phi);

	ld h = 0.005;
	ld sqrt_h = sqrt(h);

	ld rh = h;
	ld D_phi_start 	= 0.00;
	ld D_phi_end 	= 0.30;
	ld D_phi_step	= 0.01;
	ld D_phi_log_step = -1;
	ld speed_lowest 	= -1.0;
	ld speed_highest	= 1.0;

	void set_D_E(const ld& nval)
	{
		D_E = nval;
		sqrt2_D_E = sqrt(2 * D_E);
	}

	void set_D_v(const ld &nval)
	{
		D_v = nval;
		sqrt2_D_v = sqrt(2 * D_v);
	}

	void set_D_phi(const ld &nval)
	{
		D_phi = nval;
		sqrt2_D_phi = sqrt(2 * D_phi);
	}

	void set_h(const ld &nval)
	{
		h = nval;
		rh = h;
		sqrt_h = sqrt(h);
	}

	void set_mu(const ld &nval)
	{
		mu = nval;
	}

	void set_initspeed_range(ld lowest, ld highest)
	{
		speed_lowest = lowest;
		speed_highest = highest;
	}

	/* @returns 0 if ok, -1 otherwise */
	int load_params(const char *file_name) {
		ld temp;
		lua_State *L = luaL_newstate();
		luaL_openlibs(L);
		if (luaL_dofile(L, file_name) == 1) {
			err(EXIT_FAILURE, "can't load params from %s\n",
					file_name);
		}
		if (lua_intexpr(L, "integration.iterations.relaxation",
					&relaxation_iterations) == 0)
			return -1;
		printf("relaxation_iterations: %d\n", relaxation_iterations);
		if (lua_intexpr(L, "integration.iterations.observation",
					&iterations) == 0)
			return -1;
		printf("iterations: %d\n", iterations);
		if (lua_numberexpr(L, "integration.time_step", &temp) == 0)
			return -1;
		set_h(temp);
		printf("h: %lf\n", h);
		if (lua_intexpr(L, "model.number_of_particles", &N) == 0)
			return -1;
		printf("N: %d\n", N);
		if (lua_numberexpr(L, "model.rectangle_size", &L_size) == 0)
			return -1;
		printf("L_size: %lf\n", L_size);
		local_visibility = lua_boolexpr(L, "model.local_visibility");
		if (local_visibility) {
			if (lua_numberexpr(L, "model.epsilon.start", &epsilon_start) != 0) {
				if (lua_numberexpr(L, "model.epsilon.finish", &epsilon_end) == 0)
					return -1;
				if (lua_numberexpr(L, "model.epsilon.log_step", &epsilon_log_step) == 0) {
					if (lua_numberexpr(L, "model.epsilon.step", &epsilon_step) == 0) {
						puts("nor epsilon.step neither epsilon.log_step not found");
						return -1;
					}
				} else {
					epsilon_step = -1.0;
				}
				if (epsilon_step > -EPS) {
					printf("local visibility with epsilon in [%lf, %lf] with linear step %lf\n",
						epsilon_start, epsilon_end, epsilon_step);
				} else {
					printf("local visibility with epsilon in [%lf, %lf] with logarithmic step %lf\n",
						epsilon_start, epsilon_end, epsilon_log_step);
				}
				assert(epsilon_start < epsilon_end + EPS);
			} else if (lua_numberexpr(L, "model.epsilon", &epsilon) != 0) {
				printf("local visibility with epsilon %lf\n", epsilon);
			} else {
				puts("can't find any epsilon-related values");
				return -1;
			}
			use_grid = lua_boolexpr(L, "integration.use_grid");
			if (use_grid)
				puts("speed in disc will be calculated with grid (spatial partition)");
			else
				puts("speed in disc will be calculated with straightforward approach");
		} else {
			printf("global visibility\n");
		}
		if (lua_numberexpr(L, "model.mu", &temp) == 0)
			return -1;
		set_mu(temp);
		printf("mu is %lf\n", mu);
		if (lua_numberexpr(L, "model.noise_intensities.passive_noise",
					&temp) == 0)
			return -1;
		set_D_E(temp);
		printf("D_E: %lf\n", D_E);
		if (lua_numberexpr(L, "model.noise_intensities.speed_noise",
					&temp) == 0)
			return -1;
		set_D_v(temp);
		printf("D_v: %lf\n", D_v);
		if (lua_numberexpr(L, "model.noise_intensities.angular_noise.start",
					&D_phi_start) == 0)
			return -1;
		if (lua_numberexpr(L, "model.noise_intensities.angular_noise.finish",
					&D_phi_end) == 0)
			return -1;
		if (lua_numberexpr(L, "model.noise_intensities.angular_noise.step", &D_phi_step) == 0) {
			if (lua_numberexpr(L, "model.noise_intensities.angular_noise.log_step", &D_phi_log_step) == 0)
				return -1;
			D_phi_step = -1.0;
			set_D_phi(D_phi_start);
			printf("D_phi in [%lf, %lf] w/ log.step %lf\n", D_phi_start, D_phi_end, D_phi_log_step);
		} else {
			set_D_phi(D_phi_start);
			printf("D_phi in [%lf, %lf] w/ step %lf\n", D_phi_start, D_phi_end, D_phi_step);
		}
		if (lua_numberexpr(L, "model.speed.lowest", &speed_lowest) == 0)
			return -1;
		if (lua_numberexpr(L, "model.speed.highest", &speed_highest) == 0)
			return -1;
		printf("initial speeds in [%lf, %lf]\n",
			speed_lowest, speed_highest);
		lua_close(L);
		return 0;
	}
};

inline Point f(Point v, Point u_A)
{
	Point e_iv = v.get_unit();;
	return e_iv - v + params::mu * (u_A - v);
}

inline Point g_E()
{
	return params::sqrt2_D_E * Point(1, 1);
}

inline Point g_v(Point v)
{
	return params::sqrt2_D_v * v.get_unit();
}

inline Point g_phi(Point v)
{
	return params::sqrt2_D_phi * v.get_unit().get_normal();
}

Point heun_speed(Point v0, Point u_A)
{
	ld xi_x = GaussianGen::Instance().value(i_x);
	ld xi_y = GaussianGen::Instance().value(i_y);
	Point xi_E(xi_x, xi_y);

	ld xi_v = GaussianGen::Instance().value(i_v);
	ld xi_phi = GaussianGen::Instance().value(i_phi);

	Point f0 = f(v0, u_A);
	Point g_E0 = g_E();
	Point g_v0 = g_v(v0);
	Point g_phi0 = g_phi(v0);

	Point v1 = v0 + f0 * params::h + params::sqrt_h *
  	  (g_E0 * xi_E + g_v0 * xi_v + g_phi0 * xi_phi);

  	Point f1 = f(v1, u_A);
  	Point g_E1 = g_E();
  	Point g_v1 = g_v(v1);
  	Point g_phi1 = g_phi(v1);

  	Point f_avg = (f0 + f1) * 0.5;
  	Point g_E_avg = (g_E0 + g_E1) * 0.5;
  	Point g_v_avg = (g_v0 + g_v1) * 0.5;
  	Point g_phi_avg = (g_phi0 + g_phi1) * 0.5;

  	Point v2 = v0 + f_avg * params::h +
  	  (g_E_avg * xi_E + g_v_avg * xi_v +
  	   g_phi_avg * xi_phi) * params::sqrt_h;

  	return v2;
}

Point heun_position(Point r, Point v)
{
	return r + v * params::rh;
}

bool fixed_output = false;
void generate_output_name(char *name)
{
	if (fixed_output) {
		strcpy(name, "results/fixed.log");
		return;
	}
	char timestamp_buf[64];
	time_t t1 = time(NULL);
	struct tm *t2 = localtime(&t1);
	strftime(timestamp_buf, sizeof(timestamp_buf), "%b%d-%H%M", t2);
	sprintf(name, "results/cluster-%s.log", timestamp_buf);
}

void get_uA_of_eps(FILE *out)
{
	Cluster cluster(params::N, params::L_size, params::local_visibility,
			params::epsilon, params::use_grid);
	ProgressBar progress;
	for (double eps = params::epsilon_start; eps < params::epsilon_end + EPS; ) {
		printf("current epsilon: %lf\n", eps);
		cluster.reinit(params::N, params::L_size, params::local_visibility, eps);
		cluster.seed_uniformly(params::speed_lowest, params::speed_highest);
		puts("  relaxation");
		progress.start(params::relaxation_iterations);
		for (int it = 0; it < params::relaxation_iterations; ++it) {
			cluster.evolve(heun_speed, heun_position);
			progress.check_and_move(it);
		}
		progress.finish_successfully();
		puts("  observation");
		progress.start(params::iterations);
		double speed_min = 1e9;
		double speed_max = -1.0;
		double speed_sum = 0;
		for (int it = 0; it < params::iterations; ++it) {
			cluster.evolve(heun_speed, heun_position);
			progress.check_and_move(it);
			double val = cluster.get_avg_speed_val();
			speed_min = min(speed_min, val);
			speed_max = max(speed_max, val);
			speed_sum += val;
		}
		progress.finish_successfully();
		double speed_avg = speed_sum / params::iterations;
		printf("D_phi %lf, epsilon %lf: speed: min %lf, max %lf, avg %lf\n", 
			params::D_phi, eps, speed_min, speed_max, speed_avg);
		fprintf(out, "%lf %lf   %lf %lf %lf\n",
			params::D_phi, eps, speed_min, speed_max, speed_avg);
		if (params::epsilon_step > -EPS)
			eps += params::epsilon_step;
		else
			eps *= params::epsilon_log_step;
	}
}

void get_uA_of_eps_and_Dphi()
{
	char output_name[128];
	generate_output_name(output_name);
	printf("results will be put to '%s'\n", output_name);
	FILE *out = fopen(output_name, "wt");
	assert(out != NULL);
	for (double dphi = params::D_phi_start; dphi < params::D_phi_end + EPS; ) {
		printf("== current D_phi %lf ==\n", dphi);
		params::set_D_phi(dphi);
		get_uA_of_eps(out);
		if (params::D_phi_step > -EPS)
			dphi += params::D_phi_step;
		else
			dphi *= params::D_phi_log_step;
	}
	fclose(out);
}

void get_uA_of_dphi(FILE *out)
{
	Cluster cluster(params::N, params::L_size, params::local_visibility,
			params::epsilon, params::use_grid);
	ProgressBar progress;
	for (double dphi = params::D_phi_start; dphi < params::D_phi_end + EPS; ) {
		params::set_D_phi(dphi);
		printf("current: mu %lf, D_phi %lf\n",
			params::mu, params::D_phi);
		cluster.reinit(params::N, params::L_size, params::local_visibility, params::epsilon);
		cluster.seed_uniformly(params::speed_lowest, params::speed_highest);
		puts("  relaxation");
		progress.start(params::relaxation_iterations);
		for (int it = 0; it < params::relaxation_iterations; ++it) {
			cluster.evolve(heun_speed, heun_position);
			progress.check_and_move(it);
		}
		progress.finish_successfully();
		puts("  observation");
		progress.start(params::iterations);
		double speed_min = 1e9;
		double speed_max = -1.0;
		double speed_sum = 0;
		for (int it = 0; it < params::iterations; ++it) {
			cluster.evolve(heun_speed, heun_position);
			progress.check_and_move(it);
			double val = cluster.get_avg_speed_val();
			speed_min = min(speed_min, val);
			speed_max = max(speed_max, val);
			speed_sum += val;
		}
		progress.finish_successfully();
		double speed_avg = speed_sum / params::iterations;
		printf("mu %lf, D_phi %lf: speed: min %lf, max %lf, avg %lf\n", 
			params::mu, params::D_phi, speed_min, speed_max, speed_avg);
		fprintf(out, "%lf %lf   %lf %lf %lf\n",
			params::mu, params::D_phi, speed_min, speed_max, speed_avg);
		if (params::D_phi_step > -EPS)
			dphi += params::D_phi_step;
		else
			dphi *= params::D_phi_log_step;
	}
}

double get_uA_for_dphi(double dphi_val)
{
	Cluster cluster(params::N, params::L_size, params::local_visibility,
			params::epsilon, params::use_grid);
	ProgressBar progress;
	params::set_D_phi(dphi_val);
	printf("\tD_phi: %lf, init.speed in [%lf, %lf]\n",
			params::D_phi, params::speed_lowest, params::speed_highest);
	cluster.seed_uniformly(params::speed_lowest, params::speed_highest);
	puts("\trelaxation");
	progress.start(params::relaxation_iterations);
	for (int it = 0; it < params::relaxation_iterations; ++it) {
		cluster.evolve(heun_speed, heun_position);
		progress.check_and_move(it);
	}
	progress.finish_successfully();
	return cluster.get_avg_speed_val();
}

void get_uA_of_mu_and_Dphi()
{
	char output_name[128];
	generate_output_name(output_name);
	printf("results will be put to '%s'\n", output_name);
	FILE *out = fopen(output_name, "wt");
	assert(out != NULL);
	for (double mu = params::mu_start; mu < params::mu_end + EPS; mu += params::mu_step) {
		printf("== current mu %lf ==\n", mu);
		params::set_mu(mu);
		get_uA_of_dphi(out);
	}
	fclose(out);
}

void get_uA_of_time()
{
	char output_name[128];
	generate_output_name(output_name);
	printf("results will be put to %s\n", output_name);
	FILE *out = fopen(output_name, "wt");
	assert(out != NULL);
	printf("current params:\n"
		"mu %lf, eps %lf, D_phi %lf, D_E %lf, D_v %lf\n",
		params::mu, params::epsilon, params::D_phi, params::D_E,
		params::D_v);
	printf("relax during %d its, observe during %d its\n",
		params::relaxation_iterations, params::iterations);
	Cluster cluster(params::N, params::L_size, params::local_visibility,
			params::epsilon, params::use_grid);
	ProgressBar progress;
	cluster.seed_uniformly(params::speed_lowest, params::speed_highest);
	puts("\trelaxation");
	progress.start(params::relaxation_iterations);
	for (int it = 0; it < params::relaxation_iterations; ++it) {
		cluster.evolve(heun_speed, heun_position);
		progress.check_and_move(it);
	}
	progress.finish_successfully();
	puts("\tobservation");
	progress.start(params::iterations);
	for (int it = 0; it < params::iterations; ++it) {
		cluster.evolve(heun_speed, heun_position);
		progress.check_and_move(it);
		if ((it & 31) == 31) {
			double val = cluster.get_avg_speed_val();
			double cur_time = (params::relaxation_iterations + it) * params::h;
			fprintf(out, "%lf\t%lf\n", cur_time, val);
		}
	}
	progress.finish_successfully();
	puts("\tDone.");
	fclose(out);
}

void get_uA_of_dphi_for_bistability()
{
	char output_name[128];
	generate_output_name(output_name);
	printf("results will be put to %s\n", output_name);
	FILE *out = fopen(output_name, "wt");
	assert(out != NULL);
	for (double dphi = params::D_phi_start; dphi < params::D_phi_end; ) {
		printf("D_phi: %lf\n", dphi);
		/* ordered initial state */
		params::set_initspeed_range(0.5, 1.0);
		double from_ordered = get_uA_for_dphi(dphi);
		/* disordered */
		params::set_initspeed_range(-1.0, 1.0);
		double from_disordered = get_uA_for_dphi(dphi);
		printf("D_phi: %lf, uA: from ordered state %lf, from disordered state %lf\n",
				params::D_phi, from_ordered, from_disordered);
		fprintf(out, "%lf\t%lf %lf\n", params::D_phi, from_ordered, from_disordered);
		if (params::D_phi_step > -EPS)
			dphi += params::D_phi_step;
		else
			dphi *= params::D_phi_log_step;
	}
	fclose(out);
}

int main(int argc, char const *argv[])
{
	if (argc > 1) {
		if (params::load_params(argv[1]) != 0) {
			printf("problems occur while loading params "
				"from '%s'\n", argv[1]);
			return -1;
		}
	}
	fixed_output = false;
	if (argc > 2) {
		if (strcmp(argv[2], "--fixedoutput") == 0) {
			fixed_output = true;
		}
	}
	get_uA_of_dphi_for_bistability();
	return 0;
}
