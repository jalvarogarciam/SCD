// -----------------------------------------------------------------------------
//
// Sistemas concurrentes y Distribuidos.
// Práctica 3. Implementación de algoritmos distribuidos con MPI
//
// Archivo: prodcons.cpp
// Implementación del problema del productor-consumidor con
// un proceso intermedio que recibe mensajes síncronos de forma alterna.
// (versión con múltiples productores (np) múltiples consumidores (nc))
// -----------------------------------------------------------------------------


#include <iostream>
#include <iomanip>
#include <thread> // this_thread::sleep_for
#include <random> // dispositivos, generadores y distribuciones aleatorias
#include <chrono> // duraciones (duration), unidades de tiempo
#include <mpi.h>  // includes de MPI

using namespace std;
using namespace std::this_thread ;
using namespace std::chrono ;

// ---------------------------------------------------------------------
// constantes que determinan la asignación de identificadores a roles:
const int np = 4, nc = 5; // número de productores y consumidores
int contador = 0; // contador de items producidos para cada productor 

const int
   etiq_productor          = 0 , // identificador del proceso productor
   etiq_buffer             = 1 , // identificador del proceso buffer
   etiq_consumidor         = 2 , // identificador del proceso consumidor
   id_buffer               = 4 , // identificador del proceso buffer
   num_procesos_esperado = 10 , // número total de procesos esperado
   num_items             = 40 , // numero de items producidos o consumidos
   tam_vector            = 20 ; // tamaño del vector de items


void test_contadores(int cont_prod[], int cont_cons[])
{
   bool ok = true ;
   cout << "comprobando contadores ...." << endl ;

   for( unsigned i = 0 ; i < num_items ; i++ )
   {
      if ( cont_prod[i] != 1 )
      {
         cout << "error: valor " << i << " producido " << cont_prod[i] << " veces." << endl ;
         ok = false ;
      }
      if ( cont_cons[i] != 1 )
      {
         cout << "error: valor " << i << " consumido " << cont_cons[i] << " veces" << endl ;
         ok = false ;
      }
   }
   if (ok)
      cout << endl << flush << "solución (aparentemente) correcta." << endl << flush ;
}


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

/**
 * @brief Simula un retardo aleatorio de la hebra productora y 
 * muestra en pantalla que ha producido un valor
 * @param ih indice de la hebra productora
 * @return int valor producido
 */
int producir(int id)
{
   // contador de items producidos para cada productor
   static int contador = id * (num_items/np) ; // Variable estática, solo se inicializa la primera vez
   

   sleep_for( milliseconds( aleatorio<10,200>()) );
   cout << "Poductor "<<id<<" ha producido valor " << contador << endl << flush ;
   return contador ++; // primero la devuelve y luego la incrementa
}

/**
 * @brief Simula un retardo aleatorio de la hebra y muestra en pantalla 
 * que ha consumido un valor
 * @param valor_consumir valor a consumir
 */
void consumir( int valor_cons, int id)
{
   // espera bloqueada
   sleep_for( milliseconds( aleatorio<10,200>()) );
   cout << "Consumidor "<<id<<" ha consumido valor " << valor_cons << endl << flush ;
}
// ---------------------------------------------------------------------



// ---------------------------------------------------------------------

void funcion_productor(int id)
{
   for ( unsigned int i= 0 ; i < num_items/np ; i++ )
   {
      // producir valor
      int valor_prod = producir(id);
      // enviar valor
      cout << "Productor "<<id<<" va a enviar valor " << valor_prod << endl << flush;
      MPI_Ssend( &valor_prod, 1, MPI_INT, id_buffer, etiq_productor, MPI_COMM_WORLD );
   }
}
// ---------------------------------------------------------------------



void funcion_consumidor(int id)
{
   id -= np+1; // ajuste de id para que empiece en 0

   int         peticion,
               valor_rec = 1 ;
   MPI_Status  estado ;

   for( unsigned int i=0 ; i < num_items/nc; i++ )
   {
      MPI_Ssend( &peticion,  1, MPI_INT, id_buffer, etiq_consumidor, MPI_COMM_WORLD);
      MPI_Recv ( &valor_rec, 1, MPI_INT, id_buffer, etiq_buffer, MPI_COMM_WORLD,&estado );
      cout << "Consumidor "<<id<<" ha recibido valor " << valor_rec << endl << flush ;
      consumir( valor_rec, id);
   }
}
// ---------------------------------------------------------------------

void funcion_buffer()
{
   int        buffer[tam_vector],      // buffer con celdas ocupadas y vacías
              valor,                   // valor recibido o enviado
              primera_libre       = 0, // índice de primera celda libre
              primera_ocupada     = 0, // índice de primera celda ocupada
              num_celdas_ocupadas = 0, // número de celdas ocupadas
              etiq_emisor ;            // identificador de emisor aceptable
   MPI_Status estado ;                 // metadatos del mensaje recibido


   int cont_prod[num_items] = {0}, // contadores de verificación: producidos
       cont_cons[num_items] = {0}; // contadores de verificación: consumidos

   for( unsigned int i=0 ; i < num_items*2 ; i++ )
   {
      // 1. determinar si puede enviar solo prod., solo cons, o todos

      if ( num_celdas_ocupadas == 0 ){                // si buffer vacío
         etiq_emisor = etiq_productor ;               // solo prod.
      }
      else if ( num_celdas_ocupadas == tam_vector ){  // si buffer lleno
         etiq_emisor = etiq_consumidor ;              // solo cons.
      }
      else{                                           // si no vacío ni lleno
         etiq_emisor = MPI_ANY_TAG ;                  // cualquiera
      }

      // 2. recibir un mensaje del emisor o emisores aceptables
      MPI_Recv( &valor, 1, MPI_INT, MPI_ANY_SOURCE, etiq_emisor, MPI_COMM_WORLD, &estado );
      // filtra por etiqueta de emisor, no por fuente.
   

      // 3. procesar el mensaje recibido
   
      switch( estado.MPI_TAG ) // leer emisor del mensaje en metadatos
      {
         case etiq_productor: // si ha sido el productor: insertar en buffer
            // insertar valor en el buffer
            buffer[primera_libre] = valor ; // guarda valor en buffer
            primera_libre = (primera_libre+1) % tam_vector ; // actualiza la primera libre
            num_celdas_ocupadas++ ; // actualiza número de celdas ocupadas

            cout << setw(20)<< ' ' << "Buffer ha recibido valor " << valor << endl ;

            cont_prod[valor]++; // incrementa contador de verificación
            break;

         case etiq_consumidor: // si ha sido el consumidor: extraer y enviarle
            // extraer valor del buffer
            valor = buffer[primera_ocupada] ; // lee valor del buffer
            primera_ocupada = (primera_ocupada+1) % tam_vector ; // actualiza la primera ocupada
            num_celdas_ocupadas-- ; // actualiza número de celdas ocupadas

            cout << setw(20)<< ' ' << "Buffer va a enviar valor " << valor << endl ;
            MPI_Ssend( &valor, 1, MPI_INT, estado.MPI_SOURCE, etiq_buffer, MPI_COMM_WORLD);

            cont_cons[valor]++;  // incrementa contador de verificación
            break;
      }
      
   }
   cout << setw(20)<< ' ' <<"Buffer ha terminado de procesar" << endl << endl << flush ;
   sleep_for( milliseconds( 1000 ) ); // espera prudente a que los consumidores terminen de imprimir

   test_contadores(cont_prod, cont_cons); // comprobación final de contadores
}


// ---------------------------------------------------------------------

int main( int argc, char *argv[] )
{


   int id_propio, num_procesos_actual; // identificador y número de procesos

   MPI_Init( &argc, &argv ); // inicializar MPI
   MPI_Comm_rank( MPI_COMM_WORLD, &id_propio ); // obtener mi identificador
   MPI_Comm_size( MPI_COMM_WORLD, &num_procesos_actual ); // obtener número de procesos

   // comprobación de que el número de items y el tamaño del vector es múltiplo de np y de nc
   if (num_items % np*nc != 0 || tam_vector % np != 0)
   {
      if (id_propio == 0) {
         cerr << "El número de items y sebe ser múltiplo de np y de nc" << endl
               << "Y el tamaño del vector debe ser múltiplo de np" << endl; 
      }
      MPI_Finalize( );
      return 1;
   }
   if ( num_procesos_esperado == num_procesos_actual )
   {
      if ( id_propio < 4 )                 // si mi ident. es el del productor
         funcion_productor(id_propio);     //    ejecutar función del productor
      else if ( id_propio == 4 )           // si mi ident. es el del buffer
         funcion_buffer();                 //    ejecutar función buffer
      else                                 // en otro caso, mi ident es consumidor
         funcion_consumidor(id_propio);    //    ejecutar función consumidor
   }
   else
   {
      if ( id_propio == 0 ) // solo el primero escribe error, indep. del rol
      { cout << "el número de procesos esperados es:    " << num_procesos_esperado << endl
             << "el número de procesos en ejecución es: " << num_procesos_actual << endl
             << "(programa abortado)" << endl ;
      }
   }


   MPI_Finalize( );
   return 0;
}
// ---------------------------------------------------------------------
