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
Semaphore ingr_disp[num_fumadores] = {0, 0, 0}; /*vector de semáforos asociado a la disponibilidad de cada ingrediente,
Cada ingrediente es representado por la posicion que ocupa en el vector, valiendo 1 cuando está disponible
y 0 cuando no. Inicialmente valen 0 porque no se ha producido ningún ingrediente
(se inicializa en el main)*/
Semaphore mostr_vacio(1); /*inicializado a 1 porque el mostrador está vacío inicialmente,
y valdrá 1 cuando el estanquero haya producido un ingrediente.
Se hará wait cuando el estanquero produzca un ingrediente y signal cuando
un fumador haya cogido el ingrediente*/ 
Semaphore trabaja_e1(0),trabaja_e2(1);
constexpr int
   min_ms    = 20,     // tiempo minimo de espera en sleep_for
   max_ms    = 100 ;   // tiempo máximo de espera en sleep_for
   
   

bool ingredientes_e2[]={false, false, false};
int ingredientes_e1=0;


//-------------------------------------------------------------------------
// Función que simula la acción de producir un ingrediente, como un retardo
// aleatorio de la hebra (devuelve número de ingrediente producido)

int producir_ingrediente( int i)
{
   // calcular milisegundos aleatorios de duración de la acción de fumar)
   chrono::milliseconds duracion_produ( aleatorio<min_ms,max_ms>() );

   // informa de que comienza a producir
   cout << "Estanquero"<<i<<" : empieza a producir ingrediente (" << duracion_produ.count() << " milisegundos)" << endl;

   // espera bloqueada un tiempo igual a ''duracion_produ' milisegundos
   this_thread::sleep_for( duracion_produ );

   const int num_ingrediente = aleatorio<0,num_fumadores-1>() ;

   // informa de que ha terminado de producir
   cout << "Estanquero"<<i<<" : termina de producir ingrediente " << num_ingrediente << endl;

   return num_ingrediente ;
}

//----------------------------------------------------------------------
// función que ejecuta la hebra del estanquero

void funcion_hebra_estanquero1( )
{
   while (true)
   {
   	trabaja_e1.sem_wait();//espera
   	
      int ingrediente = producir_ingrediente(1); 
      ingredientes_e1++;//registra que ha producido un ingrediente
      // primero produce un ingrediente, aunque el mostrador esté lleno

      mostr_vacio.sem_wait(); //espera a que el mostrador esté vacío para colocarlo
      cout<<"Puesto ingrediente  " + to_string(ingrediente)<< endl; //pone el ingrediente en el mostrador
      ingr_disp[ingrediente].sem_signal(); //notifica a los fumadores que el ingrediente está disponible
     
     //comprueba que pueda trabajar
      if (ingredientes_e1%6!=0)trabaja_e1.sem_signal(); //sigue
      else trabaja_e2.sem_signal();	//cede el paso
		
	
	
   } 

}
void funcion_hebra_estanquero2( )
{
   while (true)
   {
   	//comprueba que pueda seguir trabajando
   	trabaja_e2.sem_wait();
   	
      int ingrediente = producir_ingrediente(2); 
      // primero produce un ingrediente, aunque el mostrador esté lleno
      ingredientes_e2[ingrediente] = true; //registra que lo ha producido

      mostr_vacio.sem_wait(); //espera a que el mostrador esté vacío para colocarlo
      cout<<"Puesto ingrediente  " + to_string(ingrediente)<< endl; //pone el ingrediente en el mostrador
      ingr_disp[ingrediente].sem_signal(); //notifica a los fumadores que el ingrediente está disponible
      
      //si falta alguno por producir, sigue trabajando
      	if (!ingredientes_e2[0] || !ingredientes_e2[1] || !ingredientes_e2[2])   trabaja_e2.sem_signal();
	else{
		trabaja_e1.sem_signal();	//da paso al otro estanquero
      		ingredientes_e2[0] = ingredientes_e2[1] = ingredientes_e2[2] =0; //restablece los contadores
      	}

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

   //Crea e inicializa el array de np y nc hebras con su funcion e indice para cada hebra.
   thread fumadores[num_fumadores], estanquero2( funcion_hebra_estanquero2 ), estanquero1( funcion_hebra_estanquero1 );
   for (int i=0; i<num_fumadores; i++)  fumadores[i] = thread(funcion_hebra_fumador, i);


   //Espera a que cada hebra termine antes de salir de la funcion.
   for (int i=0; i<num_fumadores; i++)  fumadores[i].join();
   estanquero1.join(); estanquero2.join();
}
