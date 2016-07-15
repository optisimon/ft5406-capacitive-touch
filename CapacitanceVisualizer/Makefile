DESTDIR?=""

INCLUDE= `sdl-config --cflags`
LIBS= `sdl-config --libs` -lSDL_gfx -lpthread

CapacitanceVisualizer_OBJS= main.o
CapacitanceVisualizer_LIBS= $(LIBS)

EXECS= CapacitanceVisualizer
EXEC_installed= CapacitanceVisualizer

COMPILER_FLAGS+= -Wall -O3 -std=c++0x -ggdb

CapacitanceVisualizer: $(CapacitanceVisualizer_OBJS) $(wildcard *.h) $(wildcard *.hpp) Makefile
	$(CXX) $(COMPILER_FLAGS) -o $@ $($@_OBJS) $($@_LIBS)

%.o:	%.cpp
	$(CXX) -c $(COMPILER_FLAGS) -o $@ $< $(INCLUDE)

all : CapacitanceVisualizer

.PHONY: install
install: $(EXEC_installed)
	install $(EXEC_installed) $(DESTDIR)/usr/local/bin

.PHONY: uninstall
uninstall:
	rm $(DESTDIR)/usr/local/bin/$(EXEC_installed)

.PHONY: prepare
prepare:
	apt-get install libsdl1.2-dev libsdl-gfx1.2-dev libi2c-dev

.PHONY: clean
clean:
	rm -f $(EXECS) $(CapacitanceVisualizer_OBJS)
