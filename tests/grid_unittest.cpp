#include <vector>
#include <algorithm>

#include "grid.h"
#include "gtest/gtest.h"

const static double EPS_FOR_FP_NUMBERS = EPS;

class GridTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		side = 10;
		ncells = 100;
		grid = new Grid(side, side, ncells, ncells);
		velocities.clear();
		gridPrepared = false;
	}

	void addParticle(double x, double y, double vx, double vy) {
		int id = (int) velocities.size();
		velocities.push_back(Point(vx, vy));
		Particle *particle = new Particle(id, x, y);
		particles.push_back(particle);
		grid->add(particle);
	}

	Point getDiscSpeed(int particleId, double radius) {
		if (!gridPrepared) {
			grid->update_for_search(&velocities);
			gridPrepared = true;
		}
		return grid->get_disc_speed(*(particles[particleId]), radius * radius);
	}

	Point getDiscSpeedNaively(int centerId, double radius) {
		Point speed(0, 0);
		found_particles = 0;
		found_list.clear();
		int n = (int) velocities.size();

		found_list.push_back(centerId);		
		Point center(particles[centerId]->get_x(), particles[centerId]->get_y());
		std::vector<Point> centers;
		const int cdx[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
		const int cdy[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
		for (int i = 0; i < 9; ++i)
			centers.push_back(center + Point(cdx[i] * side, cdy[i] * side));

		for (int i = 0; i < n; ++i) {
			if (i == centerId)
				continue;
			Point p(particles[i]->get_x(), particles[i]->get_y());
			bool inside = false;
			for (size_t j = 0; j < centers.size(); ++j) {
				if ((centers[j] - p).length() < radius) {
					inside = true;
					break;
				}
			}
			if (inside) {
				speed = speed + velocities[i];
				++found_particles;
				found_list.push_back(i);
			}
		}
		return speed / found_particles;
	}

	void evolve() {
		fflush(stdout);
		for (size_t i = 0; i < particles.size(); ++i) {
			Particle *p = particles[i];
			Point next_pos = velocities[i] + Point(p->get_x(), p->get_y());
			next_pos.normalize_to_rect(0, side, 0, side);
			grid->move(p, next_pos);
		}
	}

	bool speedEqual(const Point &speed, double num) {
		return fabs(speed.length() - num) < EPS_FOR_FP_NUMBERS;
	}

	double rnd_xy() {
		int mod = (int) (side - 1);
		return 0.5 + rand() % mod;
	}

	double rnd_v() {
		return 0.1 * (1 + rand() % 40);
	}

	/* virtual void TearDown() {} */
	double side;
	int ncells;
	Grid *grid;
	std::vector<Point> velocities;
	std::vector<Particle *> particles;
	bool gridPrepared;
	char buffer[1024];
	int found_particles;
	std::vector<int> found_list;
};

TEST_F(GridTest, AloneParticle) {
	addParticle(3, 4, 1, 0);

	ASSERT_EQ(velocities.size(), 1) << "expected size of velocities is 1";
	ASSERT_EQ(particles.size(), 1) << "expected size of particles is 1";

	Point ds = getDiscSpeed(0, 1);

	ASSERT_TRUE(speedEqual(ds, 0)) << "expected near zero speed of disc";
}

/* 3 particles: 2 of them in disc around the 3rd with radius 2 */
TEST_F(GridTest, TwoParticlesInDisc) {
	/* center of disc, pretty big speed */
	addParticle(3, 3, 3, 3);
	/* two particles with mediocre velocities */
	addParticle(2, 2, 1, 1);
	addParticle(4, 4, 1, 1);

	ASSERT_EQ(velocities.size(), 3) << "expected 3 elements in velocities";

	/* radius = 2: both particles are inside the disc */
	Point ds = getDiscSpeed(0, 2);
	
	ASSERT_TRUE(speedEqual(ds, sqrt(8.0) / 2)) << "expected disc speed is sqrt(8)" <<
		", actual value is " << ds.length();
}

/**
 * 3 particles: 1st is center, 2nd inside of disc,
 * 3rd outside of disc, radius 2
 */
TEST_F(GridTest, OneInAndOneOut) {
	/* center */
	addParticle(3, 3, 3, 3);
	/* particle inside, small speed */
	addParticle(2, 2, 0.5, 0.5);
	/* particle outside, huge speed */
	addParticle(5, 5, 10, 10);

	ASSERT_EQ(velocities.size(), 3) << "expected 3 elements in velocities";

	Point ds = getDiscSpeed(0, 2);

	ASSERT_TRUE(speedEqual(ds, sqrt(2 * 0.5 * 0.5))) << "expected disc speed is speed of 2nd particle";
}

/**
 * 2 particles: 1st is center, 2nd is in disc,
 * but disc crosses left border, radius 2
 *
 * 1st at (1, 3)
 * 2nd at (side - 0.5, 3)
 */
 TEST_F(GridTest, DiscCrossLeftBorder) {
 	/* center */
 	addParticle(1, 3, 3, 3);
 	/* particle at the right side */
 	addParticle(side - 0.5, 3, 4, 3);

 	Point ds = getDiscSpeed(0, 2);

 	ASSERT_TRUE(speedEqual(ds, 5)) << "expected disc speed is speed of the 2nd particle" <<
 		", actually it's " << ds.length();
 }

/**
 * 3 particles: 1st is center, 2nd is in disc,
 * 	3rd is outside
 * Disc crosses bottom line, radius 2
 * 2nd and 3rd particles under bottom line
 *** Disposition ***
 * 1st at (5, 1)
 * 2nd at (4, side - 0.1)
 * 3rd at (6, side - 1)
 */
 TEST_F(GridTest, DiscCrossBottomLine) {
 	/* center */
 	addParticle(5, 1, 3, 3);
 	/* particles under the bottom line */
 	addParticle(4, side - 0.1, 3, 4);
 	addParticle(6, side - 1, 10, 5);	/* huge speed */

 	Point ds = getDiscSpeed(0, 2);
 	ASSERT_TRUE(speedEqual(ds, 5)) << "expected disc speed is speed of the 2nd particle" <<
 		", actually it's " << ds.length();
 }

/**
 * 5 particles:
 * - 1st is center at (9, 9)
 * - 2nd is outside at (6, 6)
 * - 3rd is inside over upper border at (9, 11 - side)
 * - 4th is inside to the right of right border at (11 - side, 9)
 * - 5th is outside over upper and to the right of right borders at (12 - side, 12 - side)
 * So disc crosses two borders at the same time, radius 3
 */
TEST_F(GridTest, DiscContainsCorner) {
	/* center */
	addParticle(9, 9, 100, 100);
	/* other particles */
	addParticle(6, 6, 100, 200);
	addParticle(9, 11 - side, 3, 0);
	addParticle(11 - side, 9, 0, 4);
	addParticle(12 - side, 12 - side, 200, 100);

	Point ds = getDiscSpeed(0, 3);
	ASSERT_TRUE(speedEqual(ds, 5 / 2.0)) << "expected disc speed is sum of 3rd and 4th particles velocities";

	Point dsn = getDiscSpeedNaively(0, 3);
	ASSERT_TRUE(speedEqual(ds - dsn, 0)) << "expected the same result";
}

/**
 * Tricky case: radius is so small, that cell can be
 *  inside of disc, while corners of the cell are outside of disc
 * radius = (side / ncells) / 2;
 * Let's base_x = 5, base_y = 5
 * 1st is center at (base_x + radius * 1.5, base_y + radius)
 * 2nd is inside at (base_x + radius * (1.5 + 0.6), base_y + radius)
 */
TEST_F(GridTest, TrickySmallDisc) {
	double base_x = side / 2,
		   base_y = side / 2;
	double radius = (side / ncells) / 2;
	/* center */
	addParticle(base_x + radius * 1.5, base_y + radius, 3, 3);
	/* another particle */
	addParticle(base_x + radius * (1.5 + 0.6), base_y + radius, 3, 4);

	Point ds = getDiscSpeed(0, radius);

	ASSERT_TRUE(speedEqual(ds, 5)) << "expected disc speed is speed of the 2nd particle" <<
		", actual value is " << ds.length();
}

/**
 * Another tricky case, but disc crosses horizontal side of cell,
 *  unlike the previous case (there left side of cell is crossed)
 * radius = (side / ncells) / 2
 * Let's base_x = 5, base_y = 5
 * 1st is center at (base_x + radius, base_y + radius * 1.5)
 * 2nd is center at (base_x + radius, base_y + radius * (1.5 + 0.6))
 */
TEST_F(GridTest, TrickyVerticalCase) {
	double base_x = side / 2,
		   base_y = side / 2;
	double radius = (side / ncells) / 2;
	/* center */
	addParticle(base_x + radius, base_y + radius * 1.5, 3, 3);
	/* another particle */
	addParticle(base_x + radius, base_y + radius * (1.5 + 0.6), 6, 8);

	Point ds = getDiscSpeed(0, radius);

	ASSERT_TRUE(speedEqual(ds, 10)) << "expected disc speed is speed of the 2nd particle" << 
		", actual value is " << ds.length();
}

/**
 * 1st particle is motionless, while 2nd particle is in disc. In the next moment
 *  2nd particle leaves disc.
 */
TEST_F(GridTest, OneStepOut) {
	addParticle(2, 2, 0, 0);
	addParticle(2, 2.5, 0, 1);

	Point ds = getDiscSpeed(0, 1);

	ASSERT_TRUE(fabs(ds.length() - 1) < 1e-7) << "expected non-zero speed";

	evolve();

	ds = getDiscSpeed(0, 1);

	ASSERT_TRUE(speedEqual(ds, 0)) << "expected zero speed, but actually it's " <<
		ds.length();
}

/**
 * moment 0: 2nd inside, 3rd outside
 * moment 1: still 2nd inside, 3rd outside
 * moment 2: 2nd outside, 3rd still outside
 * moment 3: 2nd outside, 3rd inside
 */
TEST_F(GridTest, InAndOut) {
	addParticle(1, 1, 0, 0);
	addParticle(1.5, 1.45, -0.5, 0.5);
	addParticle(2, side - 1.05, -0.5, 0.5);

	double speedOfOne = Point(-0.5, 0.5).length();

	Point ds = getDiscSpeed(0, 1);
	ASSERT_TRUE(speedEqual(ds, speedOfOne)) <<
		"moment0: expected one particle in disc";

	evolve();
	ds = getDiscSpeed(0, 1);
	ASSERT_TRUE(speedEqual(ds, speedOfOne)) <<
		"moment1: expected one particle in disc";

	evolve();
	ds = getDiscSpeed(0, 1);
	ASSERT_TRUE(speedEqual(ds, 0)) <<
		"moment2: expected no particles in disc";

	evolve();
	ds = getDiscSpeed(0, 1);
	ASSERT_TRUE(speedEqual(ds, speedOfOne)) <<
		"moment3: expected one particle in disc" <<
		", actual speed of disc is " << ds.length();
}

/* Some static randomized test w/ small number of particles */
TEST_F(GridTest, SmallStaticRandom) {
	srand(41);
	int amount = 24;

	/* seeding */
	for (int i = 0; i < amount; ++i) {
		addParticle(rnd_xy(), rnd_xy(), rnd_v(), rnd_v());
	}

	for (int i = 0; i < amount; ++i) {
		Point s1 = getDiscSpeed(i, 3.3);
		Point s2 = getDiscSpeedNaively(i, 3.3);
		ASSERT_TRUE(speedEqual(s1 - s2, 0)) << "expected equality w/ dif.approaches " <<
			"for particle#" << i;
	}
}

/* Randomized test set w/ small amount of particles, but w/ some observation in time */
TEST_F(GridTest, SmallDynamicRandom) {
	srand(42);
	int amount = 25;

	/* seeding */
	for (int i = 0; i < amount; ++i) {
		addParticle(rnd_xy(), rnd_xy(), rnd_v(), rnd_v());
	}

	for (int it = 0; it < 30; ++it) {
		for (int i = 0; i < amount; ++i) {
			Point s1 = getDiscSpeed(i, 4.1);
			int found1 = grid->particlesInDisc();
			Point s2 = getDiscSpeedNaively(i, 4.1);
			int found2 = found_particles;
			buffer[0] = '(';
			s1.to_string(buffer + 1, ',', ')');
			std::string s1_print = std::string(buffer);
			s2.to_string(buffer + 1, ',', ')');
			std::string s2_print = std::string(buffer);
			if (!speedEqual(s1 - s2, 0)) {
				grid->dump_grid("grid-dump.txt");
				FILE *dump = fopen("dump.txt", "wt");
				for (int j = 0; j < amount; ++j) {
					fprintf(dump, "%lf %lf\n", particles[j]->get_x(), particles[j]->get_y());
				}
				fclose(dump);
				dump = fopen("grid-found.txt", "wt");
				std::vector<int> list1 = grid->getFoundParticles();
				sort(list1.begin(), list1.end());
				printf("grid-found: ");
				for (int j = 0; j < (int) list1.size(); ++j) {
					int id = list1[j];
					printf("%d ", id);
					fprintf(dump, "%lf %lf\n", particles[id]->get_x(), particles[id]->get_y());
				}
				puts("");
				fclose(dump);
				dump = fopen("naive-found.txt", "wt");
				sort(found_list.begin(), found_list.end());
				printf("naive-found: ");
				for (int j = 0; j < (int) found_list.size(); ++j) {
					int id = found_list[j];
					printf("%d ", id);
					fprintf(dump, "%lf %lf\n", particles[id]->get_x(), particles[id]->get_y());
				}
				puts("");
				fclose(dump);
				puts("dumped");
				printf("center at (%lf, %lf\n", particles[i]->get_x(), particles[i]->get_y());
			}
			ASSERT_TRUE(speedEqual(s1 - s2, 0)) << "expected equality" <<
				" at it = " << it << ", i = " << i << ", but s1 is " <<
				s1_print << ", s2 is " << s2_print << "; grid found " << found1 <<
				" particles and naive method found " << found2 << " particles";
		}
		evolve();
	}
}

/* Huge randomized test during proper time */
TEST_F(GridTest, LargeDynamicRandom) {
	srand(42);
	int amount = 10000;

	/* seeding */
	for (int i = 0; i < amount; ++i) {
		addParticle(rnd_xy(), rnd_xy(), rnd_v(), rnd_v());
	}

	for (int it = 0; it < 10; ++it) {
		if (it & 1) {
			printf("#");
			fflush(stdout);
		}
		for (int i = 0; i < amount; ++i) {
			Point s1 = getDiscSpeedNaively(i, 2.02);
			/*Point s2 = getDiscSpeed(i, 2.02);
			/*
			ASSERT_TRUE(speedEqual(s1 - s2, 0)) << "expected equality " <<
				" at it = " << it << ", i = " << i;
				*/
		}
		evolve();
	}
	puts("");
	fflush(stdout);
}