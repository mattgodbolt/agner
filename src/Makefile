DRIVER_SRC := driver
CXX := g++
CXXFLAGS := -O2 -m64
INCLUDES := -I$(DRIVER_SRC)

out/a64.o: PMCTestA.cpp *.h $(DRIVER_SRC)/*.h
	mkdir -p out
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

# Counter definitions (shared by test harness and list-counters)
out/CounterDefinitions.o: CounterDefinitions.cpp *.h
	mkdir -p out
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

# CPU detection (shared by test harness and list-counters)
out/CPUDetection.o: CPUDetection.cpp *.h $(DRIVER_SRC)/*.h
	mkdir -p out
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

# Standalone counter listing tool
out/list-counters: list_counters_main.cpp out/CounterDefinitions.o out/CPUDetection.o *.h $(DRIVER_SRC)/*.h
	mkdir -p out
	$(CXX) $(CXXFLAGS) -o $@ $< out/CounterDefinitions.o out/CPUDetection.o $(INCLUDES)

.PHONY: clean
clean:
	rm -f out/*.o out/*.lst out/test out/*.inc out/list-counters
