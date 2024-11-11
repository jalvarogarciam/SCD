#include <iostream>
#include <iomanip>
#include <cassert>
#include <random>
#include <thread>
#include <chrono>
#include "scd.h"

using namespace std ;
using namespace scd ;



constexpr int
   min_ms    = 5,     // tiempo minimo de espera en sleep_for
   max_ms    = 20 ;   // tiempo máximo de espera en sleep_for
static const int num_fumadores = 3;   //   núm. de fumadores


/** @brief Función que simula la acción de producir un ingrediente, como un retardo
 * aleatorio de la hebra
 * @return indice de ingrediente aleatorio
 */
int ProducirIngrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<min_ms,max_ms>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0, num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

/** @brief simula la acción de fumar, como un retardo aleatoria de la hebra */
void Fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<min_ms,max_ms>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}


class Estanco : public HoareMonitor
{/*Los ingredientes son tabaco, papel y cerillas, que se representan con los enteros 0, 1, 2*/
 private:

   int mostrador; /*entero que representa el mostrador, tendrá valor -1 cuando 
   esté vacio y el indice del ingrediente que lo ocupa cuando esté lleno*/

 CondVar                    // colas condicion:
   ingr_disp[3], /*cola para la espera de los ingredientes de los fumadores*/
   mostr_vacio; /*cola solo para el estanquero*/

 public:
   Estanco() ; // constructor

   /** @brief el fumador espera bloqueado a que su ingrediente esté disponible, 
    * y luego lo retira del mostrador.
    * @param i es el número de fumador (o el número del ingrediente que esperan)
    */
   void  obtenerIngrediente( int i);

   /** @brief pone el ingrediente en el mostrador
    * @param ingre indice del ingrediente a poner en el mostrador
    */
   void ponerIngrediente( int ingre );

   /** @brief espera bloqueado hasta que el mostrador está libre.
    */
   void esperarRecogidaIngrediente();

} ;
// -----------------------------------------------------------------------------

Estanco::Estanco(  )
{
   mostrador = -1 ;
   for (int i=0; i<num_fumadores; i++) ingr_disp[i] = newCondVar();
   mostr_vacio        = newCondVar();
}

void Estanco::obtenerIngrediente( int ih )
{
   if (mostrador != ih) ingr_disp[ih].wait();
   mostrador = -1;
   mostr_vacio.signal();
}
// -----------------------------------------------------------------------------

void Estanco::ponerIngrediente( int ingre )
{
   mostrador = ingre;
   ingr_disp[ingre].signal();
}

void Estanco::esperarRecogidaIngrediente()
{
   if (mostrador != -1) mostr_vacio.wait();
}


// *****************************************************************************
// funciones de hebras

void funcion_hebra_estanquero( MRef<Estanco> estanco)
{
   while(true){
      int ingre = ProducirIngrediente();
      estanco->ponerIngrediente( ingre );
      estanco->esperarRecogidaIngrediente();
   }
}
// -----------------------------------------------------------------------------

void funcion_hebra_fumador( MRef<Estanco> estanco, int ih )
{
   while(true){
      estanco->obtenerIngrediente(ih);
      Fumar( ih ) ;
   }
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;

   // crear monitor  ('monitor' es una referencia al mismo, de tipo MRef<...>)
   MRef<Estanco> estanco = Create<Estanco>() ;

   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread estanquero = thread(funcion_hebra_estanquero, estanco);
   thread fumadores[num_fumadores];
   for (int i=0; i<num_fumadores; i++)  
      fumadores[i] = thread(funcion_hebra_fumador, estanco, i);

   for (int i=0; i<num_fumadores; i++)  fumadores[i].join();
   estanquero.join();

}
