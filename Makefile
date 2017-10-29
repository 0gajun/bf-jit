CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -g
LDFLAGS =

COMMONFILES = utils.cpp

EXECUTORS = simple_interp.cpp opt1_interp.cpp

.PHONY: bf_interp
bf_interp: $(COMMONFILES) $(EXECUTORS) bf_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
