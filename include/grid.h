/**
 * Implementation of spatial partition,
 * see http://gameprogrammingpatterns.com/spatial-partition.html
 */

#ifndef __SSU_KMY_GRID_H_
#define __SSU_KMY_GRID_H_

#include <vector>
#include <cassert>
#include "particle.h"

class Grid {
public:
	Grid(double xsize, double ysize,
		int xcells, int ycells);
	~Grid();
	void add(Particle *particle);	
	
	/**
	 * checks if @particle is out of its cell;
	 * if so, @particle is removed from prev.cell and
	 * put to cell at (@nx, @ny)
	 * @particle - pointer to Particle, that should exist
	 *				in grid already
	 */
	void move(Particle *particle, double nx, double ny);

	/* clears @used to -1 before serie of searches */
	void prepare_to_search();
	Point get_disc_speed(const Particle &particle, double r2);
private:
	/* spatial sizes of area under grid */
	double xsize;
	double ysize;
	/* dimensions of grid */
	int xcells;
	int ycells;
	/* spatial sizes of grid cell */
	double cell_xsize;
	double cell_ysize;
	/* access: cells[x][y], not cells[y][x] */
	std::vector<std::vector<Particle*> > cells;

	/**
	 * @q and @used are for BFS
	 */
	queue<std::pair<int, int> > q;
	int *used;
	/**
	 * @time_cnt used to fill @used,
	 * initially it equals to 0,
	 * after each BFS run it should be incremented
	 */
	int time_cnt;

	/**
	 * cell_in_disc - check if any of 4 angles of the cell
	 *			hits into the disc
	 * @gx, @gy are grid coordinates of the cell
	 * @cx, @cy describe center of disc
	 * @r is a disc radius
	 */
	bool cell_in_disc(int gx, int gy,
		double cx, double cy, double r2) const;
	inline int get_used(int x, int y);
	inline void set_used(int y, int y, int val);
	/**
	 * @get_cell_speed - sums up velocities of particles in list @head,
	 * if they are in disc with params @cx, @cy, @r2
	 */
	const Point get_cell_speed(Particle *head,
		double cx, double cy, double r2);
};

#endif /* __SSU_KMY_GRID_H_ */