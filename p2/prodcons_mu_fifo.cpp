// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Practica 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons_mu_fifo.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor múltiples.
// Opcion FIFO
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
   np=5,    //número de hebras productoras
   nc=10;    //número de hebras consumidoras

constexpr int
   num_items = 100 ;   // número de items a producir/consumir
int
   siguiente_dato[np] = {0} ; /*array que indica, en cada momento, 
   para cada hebra productora, cuantos items ha producido ya. 
   Se consulta y actualiza en producir_dato() y cada índice solo es accedido 
   por una sola hebra*/

constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for

mutex
   mtx ;                 // mutex de escritura en pantalla
unsigned
   cont_prod[num_items] = {0}, // contadores de verificación: producidos
   cont_cons[num_items] = {0}; // contadores de verificación: consumidos

//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

/**
 * @brief Simula un retardo aleatorio de la hebra productora y 
 * muestra en pantalla que ha producido un valor
 * @param ih indice de la hebra productora
 * @return int valor producido
 */
int producir_dato( int ih )
{
   assert( ih < np && ih >= 0 );

   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int valor_producido = ih*(num_items/np) + siguiente_dato[ih];
   siguiente_dato[ih]++ ;
   mtx.lock();
   cout << "hebra productora "<<ih<<", produce " << valor_producido << endl << flush ;
   mtx.unlock();
   cont_prod[valor_producido]++ ;
   return valor_producido ;
}
//----------------------------------------------------------------------

/**
 * @brief Simula un retardo aleatorio de la hebra y muestra en pantalla 
 * que ha consumido un valor
 * @param valor_consumir valor a consumir
 */
void consumir_dato( unsigned valor_consumir )
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora, consume: " << valor_consumir << endl ;
   mtx.unlock();
}
//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

// *****************************************************************************
/**
 * @brief Monitor para el problema del productor-consumidor multiple
 * con un buffer de tamaño fijo y semántica SU
 */
class ProdConsMu : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
   primera_libre ,          //   indice de celda de la próxima inserción
   primera_ocupada,         //   indice de celda de la próxima extracción
   n;                       //   Número de productos en el buffer ( == número de celdas ocupadas)

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsMu(){
      primera_libre = 0 ;
      ocupadas      = newCondVar();
      libres        = newCondVar();
   }

   /**
    * @brief espera a que haya un valor en el buffer y lo lee, además de indicar
    * que ha dejado un hueco libre en el buffer.
    * @return int valor leido
    */
   int  leer(){
      // esperar bloqueado hasta que 0 < n
      if ( n == 0 ) ocupadas.wait();
      assert( 0 < n  );

      // hacer la operación de lectura, actualizando estado del monitor
      const int valor = buffer[primera_ocupada] ; // leer valor 
      primera_ocupada = ( primera_ocupada + 1 )%num_celdas_total; // actualiza la primera ocupada

      n --; // decrementa el número de productos

      // señalar al productor que hay un hueco libre, por si está esperando
      libres.signal();

      return valor ;
   }
   /**
    * @brief espera a que haya un hueco libre en el buffer y escribe un 
    * valor en el buffer
    * @param valor valor a escribir
    */
   void escribir( int valor ){
      // esperar bloqueado hasta que primera_libre < num_celdas_total
      if ( n == num_celdas_total ) libres.wait();
      assert( n < num_celdas_total );

      // hacer la operación de inserción, actualizando estado del monitor
      buffer[primera_libre] = valor ; // escribir valor
      primera_libre = ( primera_libre + 1 )%num_celdas_total; // actualiza la primera libre

      n ++;  // incrementa el número de productos

      // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
      ocupadas.signal();
   }
} ;
// -----------------------------------------------------------------------------



// *****************************************************************************
// funciones de hebras

void funcion_hebra_productora( MRef<ProdConsMu> monitor , int ih)
{
   for( unsigned i = 0 ; i < num_items/np ; i++ )
   {
      int valor = producir_dato( ih ) ;
      monitor->escribir( valor );
   }
}

void funcion_hebra_consumidora( MRef<ProdConsMu>  monitor, int ih )
{
   for( unsigned i = 0 ; i < num_items/nc ; i++ )
   {
      int valor = monitor->leer();
      consumir_dato( valor ) ;
   }
}
// -----------------------------------------------------------------------------



int main()
{
   cout << "--------------------------------------------------------------------" << endl
        << "Problema del productor-consumidor únicos (Monitor SU, buffer FIFO). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<ProdConsMu> monitor = Create<ProdConsMu>() ;
   
   auto start = chrono::high_resolution_clock::now();
   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread hebras_prod[np], hebras_cons[nc];
   for (int i=0; i<np; i++)  hebras_prod[i] = thread(funcion_hebra_productora, monitor, i);
   for (int i=0; i<nc; i++)  hebras_cons[i] = thread(funcion_hebra_consumidora, monitor, i);

   //Espera a que cada hebra termine antes de salir de la funcion.
   for (int i=0; i<np; i++)  hebras_prod[i].join();
   for (int i=0; i<nc; i++)  hebras_cons[i].join();
   auto end = chrono::high_resolution_clock::now();
   test_contadores() ;

   cout << "Tiempo de ejecución: " << chrono::duration_cast<chrono::milliseconds>(end-start).count() << "ms" << endl;
}


