#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random>
#include "scd.h"

using namespace std ;
using namespace scd ;

//**********************************************************************
// Variables globales
const unsigned //ambas deben ser >0 y divisores de num_items y tam_vec
   np=10,    //número de hebras productoras
   nc=100;    //número de hebras consumidoras

const unsigned
   num_items = 100 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer

unsigned
   // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_prod[num_items] = {0},
   // contadores de verificación: para cada dato, número de veces que se ha consumido.
   cont_cons[num_items] = {0},
   // siguiente dato a producir para cada hebra en 'producir_dato' (solo se usa ahí)
   siguiente_dato[np] = {0} ;/*array que indica, en cada momento, 
   para cada hebra productora, cuantos items ha producido ya. 
   Se consulta y actualiza en producir_dato() y cada índice solo es accedido 
   por una sola hebra*/

constexpr int               
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for

mutex
   mtx ;                 // mutex de escritura en pantalla

unsigned 
   productos[tam_vec],      //array continente de los productos 

   /*índices de la primera posición libre y ocupada */
   primera_libre=0,
   primera_ocupada = 0;

Semaphore
   puede_consumir(0),/*inicializado a 0 porque aun no se ha producido nada. 
   Cuando se produzca, se debe emitir un signal y wait cuando se lea */
   puede_producir(tam_vec);/*inicializado a tam_vec porque es el máximo de 
   veces que puede escribir antes de que se lea. 
   Cuando se lea se debe emitir un signal y wait cuando se escriba*/
mutex escribe, lee;


//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato(int ih)
{
   assert( ih < np && ih >= 0 );
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = ih*(num_items/np) + siguiente_dato[ih];
   siguiente_dato[ih]++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato)
{
   if ( num_items <= dato )
   {
      cout << " valor a consumir === " << dato << ", num_items == " << num_items << endl ;
      assert( dato < num_items );
   }
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<min_ms,max_ms>() ));
   mtx.lock();
   cout << "                  hebra consumidora, consume: " << dato << endl ;
   mtx.unlock();
}


//----------------------------------------------------------------------

void test_contadores()
{
   bool ok = true ;
   cout << "comprobando contadores ...." ;
   for( unsigned i = 0 ; i < num_items ; i++ )
   {  if ( cont_prod[i] != 1 )
      {  cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {  cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}

//----------------------------------------------------------------------

void  funcion_hebra_productora(int i_hp)
{
   assert( i_hp < np && i_hp >= 0 );
   for( unsigned i = 0 ; i < num_items/np ; i++ )
   {
      int dato = producir_dato(i_hp);/*Está fuera de la zona de exclusión porque es 
      ajeno al vector de productos y no hay interferencias*/

      puede_producir.sem_wait(); //espera hasta que haya huecos en el array
      escribe.lock(); //ninguna otra hebra puede producir en el array

      productos[primera_libre]=dato;//introduce el nuevo producto
      primera_libre = ( primera_libre + 1 )%tam_vec; //actualiza la primera posición libre

      puede_consumir.sem_signal();//notifica que hay un nuevo producto listo para ser leido
      escribe.unlock(); //otra hebra puede producir
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int i_hc)
{
   for( unsigned i = 0 ; i < num_items/nc ; i++ )
   {
      puede_consumir.sem_wait(); //espera hasta que haya productos que puedan ser leidos
      lee.lock();//ninguna otra hebra puede consumir en el vector

      int dato = productos[primera_ocupada];//extrae el producto
      primera_ocupada = ( primera_ocupada + 1 )%tam_vec; //actualiza la primera posición ocupada

      puede_producir.sem_signal();  //notifica que hay un nuevo hueco en el array
      lee.unlock();//otra hebra puede consumir


      consumir_dato(dato);/*Está fuera de la zona de exclusión porque es 
      ajeno al vector de productos y no hay interferencias, simplemente lo consume y tarda*/

   }
}
//----------------------------------------------------------------------


int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución FIFO)." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;
   

   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread hebrasp[np], hebrasc[nc];
   for (int i=0; i<np; i++)  hebrasp[i] = thread(funcion_hebra_productora, i);
   for (int i=0; i<nc; i++)  hebrasc[i] = thread(funcion_hebra_consumidora, i);


   //Espera a que cada hebra termine antes de salir de la funcion.
   for (int i=0; i<np; i++)  hebrasp[i].join();
   for (int i=0; i<nc; i++)  hebrasc[i].join();

   test_contadores();
}






/*                   Documentación
En la mísma línea que el problema anterior, para garantizar que cada dato se produce y consume una única vez,
se han implementado los mismos semáforos puede_consumir y puede_producir.
Sin embargo, al tener varias hebras productoras y consumidoras, se han tenido que añadir dos mutex para garantizar la exclusión mútua:
- escribe: para que ninguna otra hebra pueda producir en el array mientras una lo hace (no podrá escoger el mismo índice para escribir que la otra).
- lee: para que ninguna otra hebra pueda consumir en el array mientras una lo hace (no podrá escoger el mismo índice para leer que la otra).
Además, como cada hebra productora tiene asignados un rango de datos a producir, se ha añadido un array siguiente_dato que indica cuantos datos ha producido cada hebra.

Al igual que en el problema anterior, se usan los dos indices primera_libre y primera_ocupada para indicar la primera posición libre y ocupada del array de productos.


*/