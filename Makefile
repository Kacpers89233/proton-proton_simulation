CXX ?= g++
CXXFLAGS ?= -O2
CXXFLAGS += -std=c++17 -Wall -Wextra

ROOT_CONFIG ?= root-config
PYTHIA8_CONFIG := $(if $(PYTHIA8),$(PYTHIA8)/bin/pythia8-config,pythia8-config)

.DEFAULT_GOAL := all

.PHONY: all check pipeline clean

all: generate analyze

check:
	@command -v $(ROOT_CONFIG) >/dev/null 2>&1 || { \
		echo "ROOT is not configured: root-config was not found."; \
		echo "Run: source /path/to/root/bin/thisroot.sh"; \
		exit 1; \
	}
	@if [ -n "$(PYTHIA8)" ]; then \
		test -x "$(PYTHIA8_CONFIG)" || { \
			echo "Pythia 8 is not configured: $(PYTHIA8_CONFIG) was not found."; \
			echo "Set PYTHIA8 to the Pythia 8 installation directory."; \
			exit 1; \
		}; \
	else \
		command -v $(PYTHIA8_CONFIG) >/dev/null 2>&1 || { \
			echo "Pythia 8 is not configured: pythia8-config was not found."; \
			echo "Set PYTHIA8 to the Pythia 8 installation directory."; \
			exit 1; \
		}; \
	fi
	@echo "Pythia 8 and ROOT are configured."

generate: generate.cc | check
	$(CXX) $(CPPFLAGS) \
		$$($(PYTHIA8_CONFIG) --cxxflags) $$($(ROOT_CONFIG) --cflags) $(CXXFLAGS) \
		$< -o $@ $(LDFLAGS) \
		$$($(PYTHIA8_CONFIG) --libs) $$($(ROOT_CONFIG) --libs) $(LDLIBS)

analyze: analyze.cc | check
	$(CXX) $(CPPFLAGS) $$($(ROOT_CONFIG) --cflags) $(CXXFLAGS) \
		$< -o $@ $(LDFLAGS) $$($(ROOT_CONFIG) --libs) $(LDLIBS)

pipeline: all
	./generate
	./analyze
	root -l -b -q draw_plots.C

clean:
	rm -f generate analyze
