#include <progressbar.h>

ProgressBar::ProgressBar(int length, int update_mask):
	length(length), update_mask(update_mask)
{
	active = false;
}

void ProgressBar::start(int process_length_arg)
{
	process_length = process_length_arg;
	puts("");
	for (int i = 0; i < length; ++i)
		putc('-', stdout);
	puts("");
	fflush(stdout);
	active = true;
	cur = 0;
}

void ProgressBar::finish(bool result)
{
	assert(active);
	if (result) {
		while (cur < length) {
			putc('#', stdout);
			++cur;
		}
	} else {
		putc('!', stdout);
	}
	puts("");
	fflush(stdout);
	active = false;
	cur = 0;
}

void ProgressBar::moveTo(int val)
{
	assert(active);
	val = val * length / process_length;
	while (cur < val && cur < length) {
		putc('#', stdout);
		++cur;
	}
	fflush(stdout);
}

void ProgressBar::check_and_move(int pos)
{
	if ((pos & update_mask) == update_mask)
		moveTo(pos);
}
