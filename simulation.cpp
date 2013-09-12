#include <cmath>
#include <cstdio>
#include <ctime>

#include <vector>

#include <err.h>

#include <gaussian_gen.h>
#include <point.h>
#include <cluster.h>
#include <luautils.h>

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
	ld epsilon = 1;
	ld mu = 2.5;
	ld D_E = 0.05;
	ld D_v = 0;
	ld D_phi = 0.24;
	ld sqrt2_D_E = sqrt(2 * D_E);
	ld sqrt2_D_v = sqrt(2 * D_v);
	ld sqrt2_D_phi = sqrt(2 * D_phi);

	ld h = 0.005;
	ld sqrt_h = sqrt(h);

	ld rh = h;

	/* @returns 0 if ok, -1 otherwise */
	int load_params(const char *file_name) {
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
		if (lua_numberexpr(L, "integration.time_step", &h) == 0)
			return -1;
		printf("h: %lf\n", h);
		rh = h;
		sqrt_h = sqrt(h);
		if (lua_intexpr(L, "model.number_of_particles", &N) == 0)
			return -1;
		printf("N: %d\n", N);
		if (lua_numberexpr(L, "model.rectangle_size", &L_size) == 0)
			return -1;
		printf("L_size: %lf\n", L_size);
		local_visibility = lua_boolexpr(L, "model.local_visibility");
		if (local_visibility) {
			if (lua_numberexpr(L, "model.epsilon", &epsilon) == 0)
				return -1;
			printf("local visibility with epsilon: %lf\n", epsilon);
		} else {
			printf("global visibility\n");
		}
		if (lua_numberexpr(L, "model.mu", &mu) == 0)
			return -1;
		printf("mu: %lf\n", mu);
		if (lua_numberexpr(L, "model.noise_intensities.passive_noise",
					&D_E) == 0)
			return -1;
		printf("D_E: %lf\n", D_E);
		sqrt2_D_E = sqrt(2 * D_E);
		if (lua_numberexpr(L, "model.noise_intensities.speed_noise",
					&D_v) == 0)
			return -1;
		printf("D_v: %lf\n", D_v);
		sqrt2_D_v = sqrt(2 * D_v);
		if (lua_numberexpr(L, "model.noise_intensities.angular_noise",
					&D_phi) == 0)
			return -1;
		printf("D_phi: %lf\n", D_phi);
		sqrt2_D_phi = sqrt(2 * D_phi);
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

void generate_output_name(char *name)
{
	char timestamp_buf[64];
	time_t t1 = time(NULL);
	struct tm *t2 = localtime(&t1);
	strftime(timestamp_buf, sizeof(timestamp_buf), "%b%d-%H%M", t2);
	sprintf(name, "cluster-%s.log", timestamp_buf);
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
	Cluster cluster(params::N, params::L_size,
			params::local_visibility, params::epsilon);
	char output_name[128];
	generate_output_name(output_name);
	printf("log will be put to '%s'\n", output_name);
	cluster.init_log(output_name);
	cluster.seed_randomly(-0.5, 0.5);
	const int progressbar_len = 64;
	const int update_mask = 255;
	for (int i = 0; i < progressbar_len; ++i)
		putc('-', stdout);
	puts("");

	puts("relaxation");
	int progress = 0;

	for (int it = 0; it < params::relaxation_iterations; ++it) {
		cluster.evolve(heun_speed, heun_position);
		if ((it & update_mask) == update_mask) {
			while (it * progressbar_len / params::relaxation_iterations > progress) {
				putc('#', stdout);
				fflush(stdout);
				++progress;
			}
		}
	}
	while (progressbar_len > progress) {
		putc('#', stdout);
		++progress;
	}
	puts("");

	puts("tracing");
	progress = 0;

	for (int it = 0; it < params::iterations; ++it) {
		cluster.evolve(heun_speed, heun_position);
		if ((it & update_mask) == update_mask) {
			cluster.log_positions();
			bool flag_to_flush = false;
			while (it * progressbar_len / params::iterations > progress) {
				putc('#', stdout);
				++progress;
				flag_to_flush = true;
			}
			if (flag_to_flush)
				fflush(stdout);
		}
	}
	while (progressbar_len > progress) {
		putc('#', stdout);
		++progress;
	}
	puts("");
	cluster.exit_log();
	return 0;
}
