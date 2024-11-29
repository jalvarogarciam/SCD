.SUFFIXES:
.PHONY:    pc,filo,clean

bin		:= ./bin
compilador := mpicxx 
flagsc     := -std=c++11 -g -Wall


all: pc


pc: $(bin)/prodcons
	mpirun -oversubscribe -np  10 ./$<


filo: $(bin)/filosofos
	mpirun -oversubscribe  -np 10 ./$<



$(bin)/%: %.cpp
	$(compilador) $(flagsc) -o $@ $<

clean:
	rm -rf *_exe *.dSYM
