CXX = clang++
CXXFLAGS = -std=c++11 -Wall -O2 -g
LDFLAGS =

COMMONFILES = utils.cpp bf_interp.cpp jit_utils.cpp

.PHONY: bf_simple bf_opt1 bf_opt2 bf_opt3

bf_simple: $(COMMONFILES) simple_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DSIMPLE $^ -o $@

bf_opt1: $(COMMONFILES) opt1_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DOPT1 $^ -o $@

bf_opt2: $(COMMONFILES) opt2_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DOPT2 $^ -o $@

bf_opt3: $(COMMONFILES) opt3_interp.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DOPT3 $^ -o $@

bf_simple_jit: $(COMMONFILES) simple_jit.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DSIMPLE_JIT $^ -o $@
