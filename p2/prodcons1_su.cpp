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

constexpr int
   num_items = 15 ;   // número de items a producir/consumir
int
   siguiente_dato = 0 ; // siguiente valor a devolver en 'producir_dato'

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

int producir_dato(  )
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   const int valor_producido = siguiente_dato ;
   siguiente_dato ++ ;
   mtx.lock();
   cout << "hebra productora, produce " << valor_producido << endl << flush ;
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
// clase para monitor buffer, version FIFO, semántica SC, multiples prod/cons

class ProdConsSu1 : public HoareMonitor
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
   ProdConsSu1(){
      primera_libre = primera_ocupada = n = 0 ;
      ocupadas      = newCondVar();
      libres        = newCondVar();
   }

   /**
    * @brief Lee un valor del buffer
    * @return valor leído
    */
   int  leer(){
      // esperar bloqueado hasta que 0 < productos
      if ( n == 0 ) ocupadas.wait();

      assert( 0 < n  );

      // hacer la operación de lectura, actualizando estado del monitor
      const int valor = buffer[primera_ocupada] ; // leer valor 
      primera_ocupada = ( primera_ocupada + 1 )%num_celdas_total; // actualiza la primera ocupada

      n --; // decrementa el número de productos

      // señalar al productor que hay un hueco libre, por si está esperando
      libres.signal();

      // devolver valor
      return valor ;
   }

   /**
    * @brief Escribe un valor en el buffer
    * @param valor valor a escribir
    */
   void escribir( int valor ){
      // esperar bloqueado hasta que primera_libre < num_celdas_total
      if ( n == num_celdas_total )
         libres.wait();

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

void funcion_hebra_productora( MRef<ProdConsSu1> monitor )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int valor = producir_dato(  ) ;
      monitor->escribir( valor );
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_consumidora( MRef<ProdConsSu1>  monitor )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
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
   MRef<ProdConsSu1> monitor = Create<ProdConsSu1>() ;

   // crear y lanzar las hebras
   thread hebra_prod( funcion_hebra_productora, monitor ),
          hebra_cons( funcion_hebra_consumidora, monitor );

   // esperar a que terminen las hebras
   hebra_prod.join();
   hebra_cons.join();

   test_contadores() ;
}
