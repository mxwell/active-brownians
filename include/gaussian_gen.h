#ifndef __SSU_KMY_GAUSSIAN_GEN_H_
#define __SSU_KMY_GAUSSIAN_GEN_H_

#include <cstdlib>

enum source_id {
		i_x, i_y, i_v, i_phi, default_gen,
		ran3_id, NUMBER_OF_SOURCES
};

class GaussianGen
{
public:
	static GaussianGen& Instance();
	float value(source_id id);
	float ran3_value();

private:
	GaussianGen();
	GaussianGen(const GaussianGen& root);
	GaussianGen& operator= (const GaussianGen);

	int idumms[NUMBER_OF_SOURCES];
};

#endif /* __SSU_KMY_GAUSSIAN_GEN_H_ */
