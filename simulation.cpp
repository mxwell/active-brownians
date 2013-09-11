#include <cmath>
#include <cstdio>
#include <ctime>

#include <vector>

#include <gaussian_gen.h>
#include <point.h>
#include <cluster.h>

using namespace std;

namespace params {
	const int relaxation_iterations 	= 5;
	const int iterations 				= 400 * 1000;
	const int N = 10000;
	const ld L = 100;
	const ld epsilon = 1e9;
	const ld mu = 2.5;
	const ld D_E = 0.05;
	const ld D_v = 0;
	const ld D_phi = 0.24;
	const ld sqrt2_D_E = sqrt(2 * D_E);
	const ld sqrt2_D_v = sqrt(2 * D_v);
	const ld sqrt2_D_phi = sqrt(2 * D_phi);

	const ld h = 0.005;
	const ld sqrt_h = sqrt(h);

	const ld rh = h;
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

int main()
{
	Cluster cluster(params::N, params::L, params::epsilon);
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
	puts("");
	cluster.exit_log();
	return 0;
}
