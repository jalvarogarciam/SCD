.SUFFIXES:
.PHONY: x, pc1, clean

compilador:=g++
opcionesc:= -g -std=c++11 -pthread -Wfatal-errors
HOME:= .
BIN:= $(HOME)/bin

all: $(BIN)/prodcons1_su \
	 $(BIN)/prodcons_mu_fifo $(BIN)/prodcons_mu_lifo 


$(BIN)/prodcons1_su: prodcons1_su.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/prodcons_mu_fifo: prodcons_mu_fifo.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/prodcons_mu_lifo: prodcons_mu_lifo.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)



clean:
	rm -f $(BIN)/prodcons1_su
	rm -f $(BIN)/prodcons_mu_fifo
	rm -f $(BIN)/prodcons_mu_lifo