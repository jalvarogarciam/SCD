compilador:=g++

HOME:= .
BIN:= $(HOME)/bin

all: $(BIN)/prodcons \
	 $(BIN)/prodcons_mu_fifo $(BIN)/prodcons_mu_lifo \
	 $(BIN)/fumadores

$(BIN)/prodcons: prodcons.cpp scd.cpp
	g++ -o $@ $^ -g -std=c++11 -pthread
$(BIN)/prodcons_mu_fifo: prodcons_mu_fifo.cpp scd.cpp
	g++ -o $@ $^ -g -std=c++11 -pthread
$(BIN)/prodcons_mu_lifo: prodcons_mu_lifo.cpp scd.cpp
	g++ -o $@ $^ -g -std=c++11 -pthread
$(BIN)/fumadores: fumadores.cpp scd.cpp
	g++ -o $@ $^ -g -std=c++11 -pthread

clean:
	rm -f $(BIN)/prodcons
	rm -f $(BIN)/prodcons_mu_fifo
	rm -f $(BIN)/prodcons_mu_lifo
	rm -f  $(BIN)/fumadores
