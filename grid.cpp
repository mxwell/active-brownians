/**
 * Implementation of spatial partition,
 * see http://gameprogrammingpatterns.com/spatial-partition.html
 */

#include "grid.h"

template<typename T> static inline T square(const T& x)
{
	return (x) * (x);
}

/* 8 neighboring cells */
static const int neigh_number = 8;
static const int neigh_dx[neigh_number] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const int neigh_dy[neigh_number] = {-1, 0, 1, -1, 1, -1, 0, 1};

Grid::Grid(double xsize, double ysize,
		   int xcells, int ycells):
		   xsize(xsize), ysize(ysize),
		   xcells(xcells), ycells(ycells)
{
	cell_xsize = xsize / xcells;
	cell_ysize = ysize / ycells;
	cells = std::vector<std::vector<Particle*> >(xcells,
		std::vector<Particle*>(ycells, NULL));
	used = new int[xcells * ycells];
}

Grid::~Grid()
{
	delete[] used;
}

void Grid::add(Particle *particle)
{
	int cell_x = (int) (particle->get_x() / cell_xsize);
	int cell_y = (int) (particle->get_y() / cell_ysize);

	particle->prev = NULL;
	particle->next = cells[cell_x][cell_y];
	cells[cell_x][cell_y] = particle;

	if (particle->next != NULL)
		particle->next->prev = particle;
}

void Grid::update_for_search(std::vector<Point> *velocities_p)
{
	velocities = velocities_p;
	memset(used, -1, xcells * ycells * sizeof *used);
	time_cnt = 0;
}

void Grid::move(Particle *particle, double nx, double ny)
{
	int cell_x = (int) (particle->get_x() / cell_xsize);
	int cell_y = (int) (particle->get_y() / cell_ysize);

	int cell_nx = (int) (nx / cell_xsize);
	int cell_ny = (int) (ny / cell_ysize);

	if (cell_x == cell_nx || cell_y == cell_ny)
		return;

	if (particle->prev != NULL)
		particle->prev->next = particle->next;
	else if (cells[cell_x][cell_y] == particle)
		cells[cell_x][cell_y] = particle->next;
	else
		assert(0);

	if (particle->next != NULL)
		particle->next->prev = particle->prev;

	particle->move_to(nx, ny);

	/* the rest is the same as method ::add */
	particle->prev = NULL;
	particle->next = cells[cell_nx][cell_ny];
	cells[cell_nx][cell_ny] = particle;

	if (particle->next != NULL)
		particle->next->prev = particle;
}

bool Grid::cell_in_disc(int gx, int gy,
	double cx, double cy, double r2) const
{
	double x = gx * cell_xsize - cx,
		   y = gy * cell_ysize - cy;
	if (square(x) + square(y) < r2)
		return true;
	if (square(x + cell_xsize) + square(y) < r2)
		return true;
	if (square(x) + square(y + cell_ysize) < r2)
		return true;
	if (square(x + cell_xsize) + square(y + cell_ysize) < r2)
		return true;
	/* tricky cases: arc crosses side of cell twice, so that
	 * cell intersects with disc, but corners of cell are outside of disc */
	if (x < 0 && 0 < x + cell_xsize)
		return square(y) < r2 || square(y + cell_ysize) < r2;
	if (y < 0 && 0 < y + cell_ysize)
		return square(x) < r2 || square(x + cell_xsize) < r2;
	return false;
}

inline int Grid::get_used(int x, int y)
{
	return used[y * xcells + x];
}

inline void Grid::set_used(int x, int y, int val)
{
	used[y * xcells + x] = val;
}

const Point Grid::get_cell_speed(Particle *head,
	double cx, double cy, double r2)
{
	Point v(0, 0);
	while (head != NULL) {
		double x = head->get_x() - cx;
		double y = head->get_y() - cy;
		if (square(x) + square(y) < r2)
			v = v + get_particle_speed(head);
		head = head->next;
	}
	return v;
}

const Point &Grid::get_particle_speed(const Particle *particle) const
{
	return (*velocities)[particle->getId()];
}

Point Grid::get_disc_speed(const Particle &particle, double r2)
{
	double cx = particle.get_x();
	double cy = particle.get_y();

	int cellx = (int) (cx / cell_xsize);
	int celly = (int) (cy / cell_ysize);

	assert(q.empty());
	assert(centers.empty());
	q.push(std::make_pair(cellx, celly));
	centers.push(std::make_pair(cx, cy));

	Point v = get_cell_speed(cells[cellx][celly], cx, cy, r2);
	set_used(cellx, celly, time_cnt);
	while (!q.empty()) {
		std::pair<int, int> next = q.front();
		std::pair<double, double> cur_center = centers.front();
		q.pop();
		centers.pop();
		for (int i = 0; i < neigh_number; ++i) {
			int nx = next.first + neigh_dx[i];
			int ny = next.second + neigh_dy[i];
			double ncx = cur_center.first;
			double ncy = cur_center.second;
			if (nx < 0) {
				nx += xcells;
				ncx += xsize;
			} else if (nx >= xcells) {
				nx -= xcells;
				ncx -= xsize;
			}
			if (ny < 0) {
				ny += ycells;
				ncy += ysize;
			} else if (ny >= ycells) {
				ny -= ycells;
				ncy -= ysize;
			}
			if (get_used(nx, ny) == time_cnt)
				continue;
			if (!cell_in_disc(nx, ny, ncx, ncy, r2))
				continue;
			set_used(nx, ny, time_cnt);
			q.push(std::make_pair(nx, ny));
			centers.push(std::make_pair(ncx, ncy));
			/*printf("add speed of cell at (%d, %d)\n", nx, ny);
			printf("center at (%lf, %lf), r2 = %lf\n", ncx, ncy, r2);
			fflush(stdout);*/
			v = v + get_cell_speed(cells[nx][ny], ncx, ncy, r2);
		}
	}
	/*
	for (int i = 0; i < xcells; ++i) {
		bool printStarted = false;
		int rangeStarted = -1;
		for (int j = 0; j <= ycells; ++j) {
			bool isUsed = false;
			if (j < ycells) {
				isUsed = get_used(i, j) == time_cnt;
			}
			if (isUsed) {
				if (rangeStarted == -1)
					rangeStarted = j;
			} else {
				if (rangeStarted != -1) {
					if (!printStarted) {
						printf("for x = %d:", i);
						printStarted = true;
					}
					if (rangeStarted + 1 < j)
						printf(" [%d, %d]", rangeStarted, j - 1);
					else
						printf(" [%d]", rangeStarted);
					rangeStarted = -1;
				}
			}
		}
		if (printStarted)
			puts("");
	}
	fflush(stdout);
	*/
	++time_cnt;
	return v - get_particle_speed(&particle);
}