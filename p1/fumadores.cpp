#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include "scd.h"

using namespace std ;
using namespace scd ;



const int num_fumadores = 3; // numero de fumadores 

/*Los ingredientes son tabaco, papel y cerillas, que se representan con los enteros 0, 1, 2*/
vector<Semaphore> ingr_disp; /*vector de semáforos asociado a la disponibilidad de cada ingrediente,
Cada ingrediente es representado por la posicion que ocupa en el vector, valiendo 1 cuando está disponible
y 0 cuando no. Inicialmente valen 0 porque no se ha producido ningún ingrediente
(se inicializa en el main)*/
Semaphore mostr_vacio(1); /*inicializado a 1 porque el mostrador está vacío inicialmente,
y valdrá 1 cuando el estanquero haya producido un ingrediente.
Se hará wait cuando el estanquero produzca un ingrediente y signal cuando
un fumador haya cogido el ingrediente*/ 

//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int ProducirIngrediente()
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<10,100>() );

   // informa de que comienza a producir
   cout << "Estanquero : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero(  )
{
   while (true)
   {
      int ingrediente = ProducirIngrediente(); 
      // primero produce un ingrediente, aunque el mostrador esté lleno

      mostr_vacio.sem_wait(); //espera a que el mostrador esté vacío para colocarlo
      cout<<"Puesto ingrediente  " + to_string(ingrediente)<< endl; //pone el ingrediente en el mostrador
      ingr_disp[ingrediente].sem_signal(); //notifica a los fumadores que el ingrediente está disponible

   } 

}

//-------------------------------------------------------------------------
// Función que simula la acción de fumar, como un retardo aleatoria de la hebra

void fumar( int num_fumador )
{

   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_fumar( aleatorio<20,200>() );

   // informa de que comienza a fumar

    cout << "Fumador " << num_fumador << "  :"
          << " empieza a fumar (" << duracion_fumar.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_fumar' milisegundos
   this_thread::sleep_for( duracion_fumar );

   // informa de que ha terminado de fumar

    cout << "Fumador " << num_fumador << "  : termina de fumar, comienza espera de ingrediente." << endl;

}

//----------------------------------------------------------------------
// función que ejecuta la hebra del fumador
void  funcion_hebra_fumador( int num_fumador )
{
   while( true )
   {
      int ingrediente = num_fumador; //el número de fumador coincide con el número del ingrediente

      ingr_disp[ingrediente].sem_wait(); //espera a que el ingrediente esté disponible
      cout<<"Retirado ingrediente  " + to_string(ingrediente)<< endl; //retira el ingrediente
      mostr_vacio.sem_signal(); //notifica al estanquero que el mostrador está vacío

      fumar(num_fumador);
   }
}

//----------------------------------------------------------------------

int main()
{
   cout << "-----------------------------------------------------------------" << endl
        << "Problema de los fumadores." << endl
        << "------------------------------------------------------------------" << endl
        << flush ;
   
   //inicializa el vector de semaforos a 0
   for (int i=0; i<num_fumadores; i++) ingr_disp.push_back(Semaphore(0));

   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread fumadores[num_fumadores], estanquero( funcion_hebra_estanquero );
   for (int i=0; i<num_fumadores; i++)  fumadores[i] = thread(funcion_hebra_fumador, i);


   //Espera a que cada hebra termine antes de salir de la funcion.
   for (int i=0; i<num_fumadores; i++)  fumadores[i].join();
   estanquero.join();
}




/*             Documentacion
Para solucionar este problema se ha utilizado un vector de semáforos asociado a la disponibilidad de cada ingrediente.
Cada ingrediente es representado por la posición que ocupa en el vector ingr_disp, valiendo 1 cuando está disponible y 0 cuando no.
Además, como solo existen 3 fumadores, y 3 ingredientes, por simplicidad, a cada fumador solo le falta el ingrediente que es referenciado por el número de fumador dentro del vector de semáforos.
Estos semáforos se inicializan a 0 porque inicialmente no se ha producido ningún ingrediente.
   -Cada vez que el ingrediente es producido, se emite una señal al semáforo correspondiente del vector ingr_disp.
   -Cada fumador espera (wait) a que el ingrediente esté disponible para retirarlo usando el semáforo correspondiente.
Además, se ha utilizado un semáforo mostr_vacio, inicializado a 1 porque el mostrador está vacío inicialmente.
   -Cuando el estanquero produce un ingrediente, espera a que el mostrador esté vacío para colocarlo y notifica a los fumadores que el ingrediente está disponible.
   -Cuando un fumador retira un ingrediente, notifica al estanquero que el mostrador está vacío.
*/