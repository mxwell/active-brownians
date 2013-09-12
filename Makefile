PROG		= simulation
LDFLAGS 	+= -lm -lstdc++ -llua5.1
CPPFLAGS	+= -std=c++0x -Wall -Werror -Iinclude -I/usr/include -lm -lstdc++ -llua5.1

OBJFILES 	= simulation.o gaussian_gen.o point.o cluster.o luautils.o

all: $(PROG)


$(PROG): $(OBJFILES)
	$(CC) $(OBJFILES) $(LDFLAGS) -o $@

simulation.o: simulation.cpp


gaussian_gen.o: gaussian_gen.cpp include/gaussian_gen.h


point.o: point.cpp include/point.h


cluster.o: cluster.cpp include/cluster.h


luautils.o: luautils.cpp include/luautils.h


clean:
	rm -fv $(PROG) *.o

clena: clean

plot_1: plot1.gnuplot cluster.log
	gnuplot $<

run: $(PROG)
	time ./$<

pack: cluster-*.log run.log
	tar cvjf output/cluster-`date +%b%d-%H%M`.tar.bz2 cluster-*.log run.log
	rm -fv cluster-*.log run.log

clean_logs:
	rm -fv cluster-*.log

