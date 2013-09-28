#include <vector>

#include "grid.h"
#include "gtest/gtest.h"

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

	/* virtual void TearDown() {} */
	double side;
	int ncells;
	Grid *grid;
	std::vector<Point> velocities;
	std::vector<Particle *> particles;
	bool gridPrepared;
};

TEST_F(GridTest, AloneParticle) {
	addParticle(3, 4, 1, 0);

	ASSERT_EQ(velocities.size(), 1) << "expected size of velocities is 1";
	ASSERT_EQ(particles.size(), 1) << "expected size of particles is 1";

	Point ds = getDiscSpeed(0, 1);

	ASSERT_TRUE(ds.length() < 1e-9) << "expected near zero speed of disc";
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

	ASSERT_TRUE(fabs(ds.length() - sqrt(8.0)) < 1e-7) << "expected disc speed is sqrt(8)" <<
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

	ASSERT_TRUE(fabs(ds.length() - sqrt(2 * 0.5 * 0.5)) < 1e-7) << "expected disc speed is speed of 2nd particle";
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

 	ASSERT_TRUE(fabs(ds.length() - 5) < 1e-7) << "expected disc speed is speed of the 2nd particle" <<
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

 	ASSERT_TRUE(fabs(ds.length() - 5) < 1e-7) << "expected disc speed is speed of the 2nd particle" <<
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

	ASSERT_TRUE(fabs(ds.length() - 5) < 1e-7) << "expected disc speed is sum of 3rd and 4th particles velocities";
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

	ASSERT_TRUE(fabs(ds.length() - 5) < 1e-7) << "expected disc speed is speed of the 2nd particle" <<
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

	ASSERT_TRUE(fabs(ds.length() - 10) < 1e-7) << "expected disc speed is speed of the 2nd particle" << 
		", actual value is " << ds.length();
}
