CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -g
LDFLAGS =

COMMONFILES = utils.cpp

.PHONY: bf_interp
bf_interp: $(COMMONFILES) bf_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@
