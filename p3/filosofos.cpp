// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: filosofos.cpp
// Implementación del problema de los filósofos (sin camarero).
// -----------------------------------------------------------------------------



#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <iostream>
#include <iomanip>
#include <mpi.h>

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

const int
   num_filosofos = 5 ,              // número de filósofos 
   num_filo_ten  = 2*num_filosofos, // número de filósofos y tenedores 
   num_procesos  = num_filo_ten ;   // número de procesos total (por ahora solo hay filo y ten)

const int 
   etiq_coger = 0,
   etiq_soltar = 1;

#define DEBUG true
#ifdef DEBUG
#define DEBUG_DELAY milliseconds(300)
#endif

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
      id_ten_der = (id+num_filo_ten-1) % num_filo_ten; //id. tenedor der.

  for(int i=0; true ;i++)
  {
      if (id != 0){
         cout <<"Filósofo " <<id << " solicita ten. izq." <<id_ten_izq <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         //... solicitar el tenedor izquierdo

         cout <<"Filósofo " <<id <<" solicita ten. der." <<id_ten_der <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         //... solicitar el tenedor derecho
      }else{
         cout <<"Filósofo " <<id << " solicita ten. der." <<id_ten_izq <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_der, etiq_coger, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         //... solicitar el tenedor derecho

         cout <<"Filósofo " <<id <<" solicita ten. izq." <<id_ten_der <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_izq, etiq_coger, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         //... solicitar el tenedor izquierdo
      }

      cout<< setw(30)<< ' ' <<"Filósofo " <<id <<" comienza a comer" <<endl ;
      sleep_for( milliseconds( aleatorio<10,100>() ) );

      if (id != 0 ){
         cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         // ... soltar el tenedor izquierdo
         
         cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         // ... soltar el tenedor derecho
      }else{
         cout<< "Filósofo " <<id <<" suelta ten. der. " <<id_ten_der <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_der, etiq_soltar, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         // ... soltar el tenedor derecho

         cout <<"Filósofo " <<id <<" suelta ten. izq. " <<id_ten_izq <<endl;
         MPI_Ssend( &id, 1, MPI_INT, id_ten_izq, etiq_soltar, MPI_COMM_WORLD);
         if (DEBUG) sleep_for( DEBUG_DELAY );
         // ... soltar el tenedor izquierdo
      }
      cout<< setw(30)<< ' ' << "Filosofo " << id << " comienza a pensar" << endl;

      sleep_for( milliseconds( aleatorio<10,100>() ) );
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
     cout << setw(15)<< ' ' <<"Ten. " <<id <<" ha sido cogido por filo. " <<id_filosofo <<endl;

     MPI_Recv( &valor, 1, MPI_INT, id_filosofo, etiq_soltar, MPI_COMM_WORLD, &estado );
     cout << setw(15)<< ' ' <<"Ten. "<< id<< " ha sido liberado por filo. " <<id_filosofo <<endl ;
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
      if ( id_propio % 2 == 0 )          // si es par
         funcion_filosofos( id_propio ); //   es un filósofo
      else                               // si es impar
         funcion_tenedores( id_propio ); //   es un tenedor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }

   MPI_Finalize( );
   return 0;
}

// ---------------------------------------------------------------------
