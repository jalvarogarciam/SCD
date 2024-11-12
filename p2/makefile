compilador:=g++
opcionesc:= -g -std=c++11 -pthread -Wfatal-errors
HOME:= .
BIN:= $(HOME)/bin

all: $(BIN)/prodcons1_su \
	 $(BIN)/prodcons_mu_fifo $(BIN)/prodcons_mu_lifo \
	 $(BIN)/fumadores $(BIN)/lec_esc $(BIN)/lec_esc_arreglado


$(BIN)/prodcons1_su: prodcons1_su.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/prodcons_mu_fifo: prodcons_mu_fifo.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/prodcons_mu_lifo: prodcons_mu_lifo.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/fumadores: fumadores.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/lec_esc: lec_esc.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

$(BIN)/lec_esc_arreglado: lec_esc_arreglado.cpp scd.cpp
	$(compilador) -o $@ $^ $(opcionesc)

clean:
	rm -f $(BIN)/prodcons1_su
	rm -f $(BIN)/prodcons_mu_fifo
	rm -f $(BIN)/prodcons_mu_lifo
	rm -f $(BIN)/fumadores
	rm -f $(BIN)/lec_esc
	rm -f $(BIN)/lec_esc_arreglado