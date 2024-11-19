// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// Archivo: lec_esc.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// de lector-escritor. Sin escribir en ningun buffer.
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include <chrono>
#include "scd.h"

using namespace std ;
using namespace scd ;

const unsigned //ambas deben ser >0 y divisores de num_items y el tamaño del buffer
    nl=5,    //número de hebras lectoras
    ne=10;    //número de hebras escritoras

mutex mtx ;                 // mutex de escritura en pantalla

constexpr int
    min_ms    = 5,     // tiempo minimo de espera en sleep_for
    max_ms    = 20 ;   // tiempo máximo de espera en sleep_for

/**
 * @brief Muestra en pantalla que un lector esta leyendo
 * @param ih indice de la hebra lectora
 */
void leer( int ih )
{
   assert( ih < nl && ih >= 0 );

    this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
    mtx.lock();
    cout << "Lector "<<ih<<" leyendo... " << endl << flush ;
    mtx.unlock();
}
//----------------------------------------------------------------------

/**
 * @brief Muestra en pantalla que un escritor esta escribiendo
 * @param ih indice de la hebra escritora
 */
void escribir( int ih )
{
    assert( ih < ne && ih >= 0 );
    this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
    mtx.lock();
    cout << "                    Escritor " << ih << " escribiendo... "<<endl ;
    mtx.unlock();
}

//----------------------------------------------------------------------
/**
 * @brief Monitor para el problema de los lectores y escritores
 * 
 */
class Lec_Esc : public HoareMonitor
{
private:
    int  n_lec;                      //numero de lectores leyendo
    bool escrib;                     //true si hay algun escritor escribiendo 
    CondVar
        lectura,                    //  no hay escrit. escribiendo, lectura posible 
        escritura ;                 //  no hay lect. ni escrit., escritura posible 

 public:
    Lec_Esc(){
        n_lec = escrib = 0 ;
        lectura          = newCondVar();
        escritura        = newCondVar();
    }

    /**
     * @brief Comprueba que pueda leer y registra un lector más 
     * además de desbloquear a otros lectores
     * Usado solo por lectores
     */
    void ini_lectura(void){
        if (escrib) lectura.wait();// si hay escritor, espera

        n_lec++;// registrar un lector más 

        lectura.signal(); // desbloqueo en cadena de posibles lectores bloqueados 
    }

    /**
     * @brief Registra un lector menos y desbloquea un escritor si no hay más lectores
     * Usado solo por lectores
     */
    void fin_lectura(void){
        n_lec--;// registrar un lector menos

        if ( n_lec == 0 ) escritura.signal(); // si es el ultimo lector: desbloquear un escritor 
    }

    /**
     * @brief Comprueba que pueda escribir y registra que hay un escritor
     * Usado solo por escritores
     */
    void ini_escritura(void){
        if ( n_lec > 0 || escrib ) escritura.wait();    // si hay otros, esperar

        escrib = true; //registrar que hay un escritor
    }

    /**
     * @brief Registra que ya no hay escritor y desbloquea lectores si hay en la cola
     * y si no hay, desbloquea un escritor.
     * Usado solo por escritores.
     */
    void fin_escritura(void){
        escrib = false; // registrar que ya no hay escritor 
        // si hay lectores, despertar uno si no hay, despertar un escritor
        if (! lectura.empty()) lectura.signal();
        else escritura.signal() ;
    }

} ;
// -----------------------------------------------------------------------------




// *****************************************************************************
// funciones de hebras

void funcion_hebra_lectora( MRef<Lec_Esc> biblioteca , int ih)
{
    while (true){
        biblioteca->ini_lectura() ; // intenta leer
        leer(ih);                   // lee
        biblioteca->fin_lectura() ; // termina de leer
    }
}
// -----------------------------------------------------------------------------

void funcion_hebra_escritora( MRef<Lec_Esc>  biblioteca, int ih )
{
    while (true)
    {
        biblioteca->ini_escritura() ; // intenta escribir
        escribir(ih);                 // escribe
        biblioteca->fin_escritura() ; // termina de escribir
    }
}
// -----------------------------------------------------------------------------





int main()
{
    cout << "--------------------------------------------------------------------" << endl
         << "Problema de lectores-escritores (Monitor SU). " << endl
         << "--------------------------------------------------------------------" << endl
         << flush ;

    // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
    MRef<Lec_Esc> biblioteca = Create<Lec_Esc>() ;

    //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
    thread hebras_lec[nl], hebras_esc[ne];
    for (int i=0; i<ne; i++) hebras_esc[i] = thread(funcion_hebra_escritora, biblioteca, i);
    for (int i=0; i<nl; i++) hebras_lec[i] = thread(funcion_hebra_lectora, biblioteca, i);
    

    //Espera a que cada hebra termine antes de salir de la funcion.
    for (int i=0; i<ne; i++)  hebras_esc[i].join();
    for (int i=0; i<nl; i++)  hebras_lec[i].join();


}
