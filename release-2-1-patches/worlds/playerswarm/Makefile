CXX = g++
CXXFLAGS = -Wall `pkg-config --cflags playerc++`
LDFLAGS = `pkg-config --libs playerc++`

all: single multiple

single: single.cc args.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

multiple: multiple.cc args.h
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f multiple single