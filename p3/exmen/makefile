bin		:= ./bin
flagsc     := -std=c++11 -g -Wall


all: filosofos


prodcons: prodcons.cpp
	mpicxx  $(flagsc) -o $(bin)/$@ $^
	mpirun -oversubscribe  -np 11 $(bin)/$@

filosofos-interb: filosofos-interb.cpp
	mpicxx $(flagsc) -o $(bin)/$@ $^
	mpirun -oversubscribe  -np 10 $(bin)/$@

filosofos: filosofos.cpp
	mpicxx $(flagsc) -o $(bin)/$@ $^
	mpirun -oversubscribe  -np 10 $(bin)/$@
	
camarero: camarero.cpp
	mpicxx $(flagsc) -o $(bin)/$@ $^
	mpirun -oversubscribe  -np 11 $(bin)/$@

clean:
	rm -r $(bin)/*
