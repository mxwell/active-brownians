/* NOTE: separate flows of RNG ain't working,
 * maybe because of peculiarities of 'gasdev.h'
 **/

#include <cmath>
#include <gasdev.h>
#include <gaussian_gen.h>
#include <cstdio>

const int sample_init[] = {10, 531, -42, 897, 733, -6, -78, -371, 320};

GaussianGen& GaussianGen::Instance()
{
	static GaussianGen theSingleInstance;
	return theSingleInstance;
}

GaussianGen::GaussianGen()
{
	for (size_t i = 0; i < NUMBER_OF_SOURCES; ++i)
		idumms[i] = sample_init[i];
}

float GaussianGen::value(source_id id)
{
	return gasdev(&idumms[id]);
}

float GaussianGen::ran3_value()
{
	return ran3(&idumms[ran3_id]);
}
