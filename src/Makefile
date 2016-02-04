DRIVER_SRC := driver
out/a64.o: PMCTestA.cpp *.h $(DRIVER_SRC)/*.h
	mkdir -p out
	g++ -O2 -c -m64 -o out/a64.o PMCTestA.cpp -I$(DRIVER_SRC)
