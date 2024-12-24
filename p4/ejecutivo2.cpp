// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 4. Implementación de sistemas de tiempo real.
//
// Archivo: ejecutivo2.cpp
// Implementación del primer ejemplo de ejecutivo cíclico:
//
//   Datos de las tareas:
//   ------------
//   Ta.  T    C
//   ------------
//   A  400   100
//   B  800   120
//   C  200   40
//   D  200   30
//  -------------
//
// La duracion del ciclo principal es de 800 ms, ya que mcm(400,800,200,200)=800,
// Por tanto, Ts = 200 ()
//  Planificación (con Ts == 200 ms)
//  *---------*----------*---------*--------*
//  | C D A   | C D B    | C D A   | C D    |
//  *---------*----------*---------*--------*
//      170       190      170        70
//      30        10       30         130
/*
1.¿cual es el mínimo tiempo de espera que queda al final de las
iteraciones del ciclo secundario con tu solución ?
   Con esta planificacion, el minipo tiempo de espera que queda al final de
   las iteraciones del ciclo secundario es  10.

2.¿ sería planificable si la tarea D tuviese un tiempo cómputo de
250 ms ?
No, ya que excedería su propio periodo (200), y es imposible que una tarea 
que se ejecuta cada 200ms tarde 250ms sin solaparse consigo misma
*/
// -----------------------------------------------------------------------------

#include <string>
#include <iostream> // cout, cerr
#include <thread>
#include <chrono>   // utilidades de tiempo
#include <ratio>    // std::ratio_divide

using namespace std ;
using namespace std::chrono ;
using namespace std::this_thread ;

// tipo para duraciones en segundos y milisegundos, en coma flotante:
//typedef duration<float,ratio<1,1>>    seconds_f ;
typedef duration<float,ratio<1,1000>> milliseconds_f ;

// -----------------------------------------------------------------------------
// tarea genérica: duerme durante un intervalo de tiempo (de determinada duración)

void Tarea( const std::string & nombre, milliseconds tcomputo )
{
   cout << "   Comienza tarea " << nombre << " (C == " << tcomputo.count() << " ms.) ... " ;
   sleep_for( tcomputo );
   cout << "fin." << endl ;
}

// -----------------------------------------------------------------------------
// tareas concretas del problema:

void TareaA() { Tarea( "A", milliseconds(100) );  }
void TareaB() { Tarea( "B", milliseconds(120) );  }
void TareaC() { Tarea( "C", milliseconds(40) );  }
void TareaD() { Tarea( "D", milliseconds(30) );  }

// -----------------------------------------------------------------------------
// implementación del ejecutivo cíclico:

int main( int argc, char *argv[] )
{
   // Ts = duración del ciclo secundario (en unidades de milisegundos, enteros)
   const milliseconds Ts_ms( 200 );

   // ini_sec = instante de inicio de la iteración actual del ciclo secundario
   time_point<steady_clock> ini_sec = steady_clock::now();

   while( true ) // ciclo principal
   {
      cout << endl
           << "---------------------------------------" << endl
           << "Comienza iteración del ciclo principal." << endl ;

      for( int i = 1 ; i <= 4 ; i++ ) // ciclo secundario (4 iteraciones)
      {
         cout << endl << "Comienza iteración " << i << " del ciclo secundario." << endl ;

         switch( i )
         {
            case 1 : TareaC(); TareaD(); TareaA(); break ;
            case 2 : TareaC(); TareaD(); TareaB(); break ;
            case 3 : TareaC(); TareaD(); TareaA(); break ;
            case 4 : TareaC(); TareaD();           break ;
         }

         // calcular el siguiente instante de inicio del ciclo secundario
         ini_sec += Ts_ms ;

         // esperar hasta el inicio de la siguiente iteración del ciclo secundario
         sleep_until( ini_sec );

        time_point<steady_clock> fin_sec = steady_clock::now();
        steady_clock::duration duracion = fin_sec - ini_sec;
        cout << "El retraso es de " << milliseconds_f(duracion).count() << " milisegundos"<<endl;
      }
   }
}
