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
static const neigh_number = 8;
static const neigh_dx[neigh_number] = {-1, -1, -1, 0, 0, 1, 1, 1};
static const neigh_dy[neigh_number] = {-1, 0, 1, -1, 1, -1, 0, 1};

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

	/* the rest is the same as method ::add */
	particle->prev = NULL;
	particle->next = cells[cell_nx][cell_ny];
	cells[cell_nx][cell_ny] = particle;

	if (particle->next != NULL)
		particle->next->prev = particle;
}

void Grid::prepare_to_search()
{
	memset(used, -1, n * m * sizeof *used);
	time_cnt = 0;
}

bool Grid::cell_in_disc(int gx, int gy,
	double cx, double cy, double r2) const
{
	double x = gx * cell_xsize - cx,
		   y = gy * cell_ysize - cy;
	if (sqr(x) + sqr(y) < r2)
		return true;
	if (sqr(x + cell_xsize) + sqr(y) < r2)
		return true;
	if (sqr(x) + sqr(y + cell_ysize) < r2)
		return true;
	if (sqr(x + cell_xsize) + sqr(y + cell_ysize) < r2)
		return true;
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
		if (sqr(x) + sqr(y) < r2)
			v = v + head->get_velocity();
		head = head->next;
	}
}

Point Grid::get_disc_speed(const Particle &particle, double r2)
{
	double cx = particle.get_x();
	double cy = particle.get_y();

	int cellx = (int) (cx / cell_xsize);
	int celly = (int) (cy / cell_ysize);

	assert(q.empty(std::make_pair(cellx, celly)));
	q.push_back();

	Point v = get_cell_speed(cells[cellx][celly], cx, cy, r2);
	set_used(cellx, celly, time_cnt);
	while (!q.empty()) {
		std::pair<int, int> next = q.front();
		q.pop();
		for (int i = 0; i < neigh_number; ++i) {
			int nx = next.first + neigh_dx[i];
			int ny = next.second + neigh_dy[i];
			if (get_used(nx, ny) == time_cnt)
				continue;
			double ncx = cx;
			double ncy = cy;
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
			if (!cell_in_disc(nx, ny, ncx, ncy, r2))
				continue;
			v = v + get_cell_speed(cells[nx][ny], ncx, ncy, r2);
		}
	}
	++time_cnt;
	return v;
}