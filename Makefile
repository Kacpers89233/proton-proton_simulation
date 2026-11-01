PYTHIA8 ?= $(HOME)/cern/pythia8318
CXXFLAGS := $(shell $(PYTHIA8)/bin/pythia8-config --cxxflags) $(shell root-config --cflags)
LIBS := $(shell $(PYTHIA8)/bin/pythia8-config --libs) $(shell root-config --libs)
%: %.cc
	g++ $< -o $@ $(CXXFLAGS) -std=c++17 -Wall -Wextra $(LIBS)