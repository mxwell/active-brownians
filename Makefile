PROG		= simulation
LDFLAGS 	+= -lm -lstdc++
CPPFLAGS	+= -std=c++0x -Wall -Werror -Iinclude
OBJFILES 	= simulation.o gaussian_gen.o point.o cluster.o

all: $(PROG)


$(PROG): $(OBJFILES)
	$(CC) $(OBJFILES) $(LDFLAGS) -o $@

simulation.o: simulation.cpp


gaussian_gen.o: gaussian_gen.cpp include/gaussian_gen.h


point.o: point.cpp include/point.h


cluster.o: cluster.cpp include/cluster.h


clean:
	rm -f $(PROG) *.o

clena: clean

plot_1: plot1.gnuplot cluster.log
	gnuplot $<

run: $(PROG)
	./$<

pack: cluster.log
	tar cjf cluster.tar.bz2 cluster.log simulation.cpp

