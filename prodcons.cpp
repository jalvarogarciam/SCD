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

const unsigned 
   num_items = 40 ,   // número de items
	tam_vec   = 10 ;   // tamaño del buffer
unsigned  
   cont_prod[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha producido.
   cont_cons[num_items] = {0}, // contadores de verificación: para cada dato, número de veces que se ha consumido.
   siguiente_dato       = 0 ;  // siguiente dato a producir en 'producir_dato' (solo se usa ahí)
unsigned 
   productos[tam_vec],      //vector continente de los datos
   primera_libre=0,     //índice de la primera posición libre
   primera_ocupada=0;   //índice de la primera posición ocupada
Semaphore 
   puede_leer(0),/*inicializado a 0 porque aun no se ha producido nada. 
   Cuando se produzca, se debe emitir un signal y wait cuando se lea */
   puede_escribir(tam_vec);/*inicializado a tam_vec porque es el máximo de 
   veces que puede escribir antes de que se lea. 
   Cuando se lea se debe emitir un signal y wait cuando se escriba*/


//**********************************************************************
// funciones comunes a las dos soluciones (fifo y lifo)
//----------------------------------------------------------------------

unsigned producir_dato()
{
   this_thread::sleep_for( chrono::milliseconds( aleatorio<20,100>() ));
   const unsigned dato_producido = siguiente_dato ;
   siguiente_dato++ ;
   cont_prod[dato_producido] ++ ;
   cout << "producido: " << dato_producido << endl << flush ;
   return dato_producido ;
}
//----------------------------------------------------------------------

void consumir_dato( unsigned dato )
{
   assert( dato < num_items );
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

void  funcion_hebra_productora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      int dato = producir_dato() ;/*Está fuera de la zona de exclusión porque es 
      ajeno al vector de datos y no hay interferencias*/
      puede_escribir.sem_wait() ;//espera hasta que haya huecos en el array

      productos[primera_libre] = dato ;//introduce el nuevo producto
      primera_libre = ( primera_libre + 1 )%tam_vec;

      puede_leer.sem_signal() ;//notifica que hay un nuevo producto listo para ser leido
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      puede_leer.sem_wait() ;//espera hasta que haya datos que puedan ser leidos

      int dato = productos[primera_ocupada] ;//extrae el producto
      primera_ocupada = ( primera_ocupada +1 )%tam_vec ;

      puede_escribir.sem_signal() ;//notifica que hay un nuevo hueco en el array

      consumir_dato( dato ) ;/*Está fuera de la zona de exclusión porque es 
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

   thread hebra_productora ( funcion_hebra_productora ),
          hebra_consumidora( funcion_hebra_consumidora );

   hebra_productora.join() ;
   hebra_consumidora.join() ;

   test_contadores();
}
