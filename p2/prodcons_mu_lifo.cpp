// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Seminario 2. Introducción a los monitores en C++11.
//
// Archivo: prodcons1_su.cpp
//
// Ejemplo de un monitor en C++11 con semántica SU, para el problema
// del productor/consumidor, con productor y consumidor únicos.
// Opcion FIFO
// -----------------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include "scd.h"

using namespace std ;
using namespace scd ;

const unsigned //ambas deben ser >0 y divisores de num_items y el tamaño del buffer
   np=10,    //número de hebras productoras
   nc=5;    //número de hebras consumidoras

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

void consumir_dato( unsigned valor_consumir )
{
   if ( num_items <= valor_consumir )
   {
      cout << " valor a consumir === " << valor_consumir << ", num_items == " << num_items << endl ;
      assert( valor_consumir < num_items );
   }
   cont_cons[valor_consumir] ++ ;
   this_thread::sleep_for( chrono::milliseconds( 0 ));
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
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class ProdConsMu : public HoareMonitor
{
 private:
 static const int           // constantes ('static' ya que no dependen de la instancia)
   num_celdas_total = 10;   //   núm. de entradas del buffer
 int                        // variables permanentes
   buffer[num_celdas_total],//   buffer de tamaño fijo, con los datos
   primera_libre ;          //   indice de celda de la próxima inserción

 CondVar                    // colas condicion:
   ocupadas,                //  cola donde espera el consumidor (n>0)
   libres ;                 //  cola donde espera el productor  (n<num_celdas_total)

 public:                    // constructor y métodos públicos
   ProdConsMu() ;             // constructor
   int  leer();                // extraer un valor (sentencia L) (consumidor)
   void escribir( int valor ); // insertar un valor (sentencia E) (productor)
} ;
// -----------------------------------------------------------------------------

ProdConsMu::ProdConsMu(  )
{
   primera_libre = 0 ;
   ocupadas      = newCondVar();
   libres        = newCondVar();
}
// -----------------------------------------------------------------------------
// función llamada por el consumidor para extraer un dato

int ProdConsMu::leer(  )
{
   // esperar bloqueado hasta que 0 < n
   if ( primera_libre == 0 )
      ocupadas.wait();

   assert( 0 < primera_libre  );


   // hacer la operación de lectura, actualizando estado del monitor
   primera_libre --; // actualiza el numero de elementosç
   const int valor = buffer[primera_libre] ; // leer valor 

   // señalar al productor que hay un hueco libre, por si está esperando
   libres.signal();

   // devolver valor
   return valor ;
}
// -----------------------------------------------------------------------------

void ProdConsMu::escribir( int valor )
{
   // esperar bloqueado hasta que primera_libre < num_celdas_total
   if ( primera_libre == num_celdas_total )
      libres.wait();

   assert( primera_libre < num_celdas_total );

   // hacer la operación de inserción, actualizando estado del monitor
   buffer[primera_libre] = valor ; // escribir valor
   primera_libre ++; // actualiza número de productos

   //escribiendo.unlock(); //señala que ya ha terminado de escribir
   // señalar al consumidor que ya hay una celda ocupada (por si esta esperando)
   ocupadas.signal();
}
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
// -----------------------------------------------------------------------------

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
        << "Problema del productor-consumidor únicos (Monitor SU, buffer LIFO). " << endl
        << "--------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<ProdConsMu> monitor = Create<ProdConsMu>() ;

   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread hebras_prod[np], hebras_cons[nc];
   for (int i=0; i<np; i++)  hebras_prod[i] = thread(funcion_hebra_productora, monitor, i);
   for (int i=0; i<nc; i++)  hebras_cons[i] = thread(funcion_hebra_consumidora, monitor, i);

   //Espera a que cada hebra termine antes de salir de la funcion.
   for (int i=0; i<np; i++)  hebras_prod[i].join();
   for (int i=0; i<nc; i++)  hebras_cons[i].join();

   test_contadores() ;
}
