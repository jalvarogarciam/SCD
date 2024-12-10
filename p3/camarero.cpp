// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos.cpp
// Implementación del problema de los filósofos (sin camarero).
// -----------------------------------------------------------------------------


#include <mpi.h>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>
#include <iomanip>

#define DEBUG true
#ifdef DEBUG
#define DEBUG_DELAY milliseconds(200)
#endif

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores 
   num_procesos  = num_filo_ten + 1 ,  // número de procesos total (+1 por el camarero)
   id_camarero = 10;
const int
   etiq_coger = 0,
   etiq_soltar = 1,
   etiq_sentarse = 2,
   etiq_levantarse = 3;

//**********************************************************************
// plantilla de función para generar un entero aleatorio uniformemente
// distribuido entre dos valores enteros, ambos incluidos
// (ambos tienen que ser dos constantes, conocidas en tiempo de compilación)
//----------------------------------------------------------------------

template< int min, int max > int aleatorio()
{
  static default_random_engine generador( (random_device())() );
  static uniform_int_distribution<int> distribucion_uniforme( min, max ) ;
  return distribucion_uniforme( generador );
}

// ---------------------------------------------------------------------

void funcion_filosofos( int id )
{
  int id_ten_izq = (id+1)              % num_filo_ten, //id. tenedor izq.
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten, //id. tenedor der.
      peticion;

  for(int i=0; true ;i++)
  {
      cout <<"Filósofo " <<id << " solicita sentarse." <<endl;
      if (DEBUG) sleep_for( DEBUG_DELAY );
      MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_sentarse, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      //...solicitar sentarse

      cout << setw(10)<< ' ' <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
      if (DEBUG) sleep_for( DEBUG_DELAY );
      MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      //... solicitar el tenedor izquierdo

      cout << setw(10)<< ' ' <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
      if (DEBUG) sleep_for( DEBUG_DELAY );
      MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      //... solicitar el tenedor derecho


      cout<< setw(20)<< ' ' <<"Filósofo " <<id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,1000>() ) );


      cout << setw(10)<< ' ' <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
      MPI_Ssend( &peticion, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      // ... soltar el tenedor izquierdo
      
      cout << setw(10)<< ' ' << "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
      MPI_Ssend( &peticion, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      // ... soltar el tenedor derecho

      cout <<"Filósofo " <<id << " solicita levantarse." <<endl;
      MPI_Ssend( &peticion, 1, MPI_INT, id_camarero, etiq_levantarse, MPI_COMM_WORLD);
      if (DEBUG) sleep_for( DEBUG_DELAY );
      //... solicitar levantarse

      cout<< setw(20)<< ' ' << "Filosofo " << id << " comienza a pensar" << endl;
      sleep_for( milliseconds( aleatorio<10,1000>() ) );
 }
}

// ---------------------------------------------------------------------
void funcion_camarero()
{
   const int max_sentados = num_filosofos-1; // 4
   int peticion ;  // valor recibido, identificador del filósofo
   int sentados = 0;
   int etiq_aceptable;
   MPI_Status estado ;

   for(;true;){

      // Si no hay sitios libres, solo podrán levantarse
      if (sentados == max_sentados) etiq_aceptable =  etiq_levantarse;
      // De lo contrario, podrán sentarse o levantarse
      else                          etiq_aceptable = MPI_ANY_TAG;

      // Espera peticiones...
      MPI_Recv( &peticion, 1, MPI_INT, MPI_ANY_SOURCE, etiq_aceptable, MPI_COMM_WORLD, &estado);

      if (estado.MPI_TAG == etiq_levantarse){
         sentados--;
         cout<< "Filosofo " << estado.MPI_SOURCE << " se ha levantado. "<<endl;

      }else{
         cout<< "Filosofo " << estado.MPI_SOURCE << " se ha sentado. "<<endl;
         sentados++;
      }
   }
}



// ---------------------------------------------------------------------
void funcion_tenedores( int id )
{
  int valor, id_filosofo ;  // valor recibido, identificador del filósofo
  MPI_Status estado ;       // metadatos de las dos recepciones

  for (int i=0; true ; i++)
  {
     MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_coger, MPI_COMM_WORLD, &estado );
     id_filosofo = estado.MPI_SOURCE ;
     cout << setw(10)<< ' ' <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, etiq_soltar, MPI_COMM_WORLD, &estado );
     cout << setw(10)<< ' ' <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
  }
}



// ---------------------------------------------------------------------
int main( int argc, char** argv )
{
   int id_propio, num_procesos_actual ;

   MPI_Init( &argc, &argv );
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio );
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual );


   if ( num_procesos == num_procesos_actual )
   {
      // ejecutar la función correspondiente a 'id_propio'
      if (id_propio == id_camarero)
         funcion_camarero();
      else if ( id_propio % 2 == 0 ) 
         funcion_filosofos( id_propio ); //los pares son filósofos
      else                               
         funcion_tenedores( id_propio ); //los impares son tenedores
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cerr << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
