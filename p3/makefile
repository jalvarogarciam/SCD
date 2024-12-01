bin		:= ./bin
compilador := mpicxx 
flagsc     := -std=c++11 -g -Wall


all: $(bin)/filosofos


$(bin)/prodcons: prodcons.cpp
	$(compilador) $(flagsc) -o $@ $<
	mpirun -oversubscribe  -np 10 $@


$(bin)/filosofos: filosofos.cpp
	$(compilador) $(flagsc) -o $@ $<
	mpirun -oversubscribe  -np 10 $@
	

clean:
	rm -r $(bin)/*
