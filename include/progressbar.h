#ifndef __SSU_KMY_PROGRESSBAR_H
#define __SSU_KMY_PROGRESSBAR_H

#include <cstdio>
#include <cassert>

class ProgressBar {
public:
	ProgressBar(int length = 64, int update_mask = 255);
	void start(int process_length);
	void finish_successfully() { finish(true); }
	void finish_badly() { finish(false); }
	void moveTo(int val);
	void check_and_move(int pos); 
private:
	int length;
	int update_mask;
	int process_length;
	bool active;
	int cur;
	void finish(bool result);
};

#endif /* __SSU_KMY_PROGRESSBAR_H */

