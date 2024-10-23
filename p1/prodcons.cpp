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
   puede_consumir(0),/*inicializado a 0 porque aun no se ha producido nada. 
   Cuando se produzca, se debe emitir un signal y wait cuando se lea */
   puede_producir(tam_vec);/*inicializado a tam_vec porque es el máximo de 
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
      puede_producir.sem_wait() ;//espera hasta que haya huecos en el array

      productos[primera_libre] = dato ;//introduce el nuevo producto
      primera_libre = ( primera_libre + 1 )%tam_vec;

      puede_consumir.sem_signal() ;//notifica que hay un nuevo producto listo para ser leido
   }
}

//----------------------------------------------------------------------

void funcion_hebra_consumidora(  )
{
   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      puede_consumir.sem_wait() ;//espera hasta que haya datos que puedan ser leidos

      int dato = productos[primera_ocupada] ;//extrae el producto
      primera_ocupada = ( primera_ocupada +1 )%tam_vec ;

      puede_producir.sem_signal() ;//notifica que hay un nuevo hueco en el array

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




/*
                     DOCUMENTACIÓN
Para la implementación de este problema se ha utilizado un vector de tamaño arbitrario
(aunque debe ser divisor del número de items a producir) que contendrá los datos producidos.
Para ello, se han creado dos semáforos para controlar el acceso a este vector:
   - puede_consumir: inicializado a 0 porque aun no se ha producido nada. Cuando se produzca, se debe emitir un signal y wait cuando se lea.
   - puede_producir: inicializado a tam_vec porque es el máximo de veces que puede escribir antes de que se lea. Cuando se lea se debe emitir un signal y wait cuando se escriba.
De esta forma, se garantiza que no se produzcan interferencias entre las hebras productoras y consumidoras, garantizando que cada dato se produzca y consuma una única vez.
   +En el caso de que el vector se llene, la hebra productora esperará a que se consuma un dato para poder seguir produciendo.
   +En el caso de que el vector esté vacío, la hebra consumidora esperará a que se produzca un dato para poder seguir consumiendo.

En cuanto al manejo de indices, al haber elegido la solución Fifo, se han creado dos variables globales:
   - primera_libre: índice de la primera posición libre del vector.
   - primera_ocupada: índice de la primera posición ocupada del vector.
De esta forma, cada vez que se produzca un dato, se introducirá en la posición primera_libre y se incrementará en 1, y cada vez que se consuma un dato, se extraerá de la posición primera_ocupada y se incrementará en 1.
Sin embargo, se ha tenido en cuenta que el vector es circular, por lo que si se llega al final del vector, se volverá al principio y viceversa.


*/