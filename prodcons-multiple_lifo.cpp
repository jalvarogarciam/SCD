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
   nc=10;    //número de hebras consumidoras

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

unsigned 
   productos[tam_vec],      //array continente de los productos 

   /*índices de la primera posición libre*/
   primera_libre=0;

Semaphore
   puede_leer(0),/*inicializado a 0 porque aun no se ha producido nada. 
   Cuando se produzca, se debe emitir un signal y wait cuando se lea */
   puede_escribir(tam_vec);/*inicializado a tam_vec porque es el máximo de 
   veces que puede escribir antes de que se lea. 
   Cuando se lea se debe emitir un signal y wait cuando se escriba*/
mutex produce, consume; /* cerrojos para controlar la exclusión mutua en la
zona de producción y consumo de productos respectivamente*/


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
   assert( dato < num_items);
   cont_cons[dato] ++ ;
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));

   cout << "                  consumido: " << dato << endl ;
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

      produce.lock(); //ninguna otra hebra puede producir en el array
      puede_escribir.sem_wait(); //espera hasta que haya huecos en el array

      productos[primera_libre]=dato;//introduce el nuevo producto
      primera_libre = ( primera_libre + 1 )%tam_vec; //actualiza el índice de la primera posición libre

      puede_leer.sem_signal();//notifica que hay un nuevo producto listo para ser leido
      produce.unlock(); //otra hebra puede producir
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(int i_hc)
{
   for( unsigned i = 0 ; i < num_items/nc ; i++ )
   {
      consume.lock();//ninguna otra hebra puede consumir en el vector
      puede_leer.sem_wait(); //espera hasta que haya productos que puedan ser leidos

      //extrae el producto
      int dato = primera_libre == 0? productos[tam_vec-1]:productos[primera_libre-1];
      primera_libre = primera_libre==0? tam_vec-1:( primera_libre - 1 ); //actualiza la primera posición ocupada

      puede_escribir.sem_signal();  //notifica que hay un nuevo hueco en el array
      consume.unlock();//otra hebra puede consumir


      consumir_dato(dato);/*Está fuera de la zona de exclusión porque es 
      ajeno al vector de productos y no hay interferencias, simplemente lo consume y tarda*/

   }
}
//----------------------------------------------------------------------


int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los productores-consumidores (solución LIFO)." << endl
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
