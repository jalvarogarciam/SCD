from threading import Lock, Semaphore, Thread
from time import sleep
import random
import time

np = 5 # número de productores
nc = 10 # número de consumidores

num_items = 100     # número de items a producir/consumir
siguiente_dato = [0]*np 
'''array que indica, en cada momento, para cada hebra productora, cuantos items ha producido ya. 
   Se consulta y actualiza en producir_dato() y cada índice solo es accedido por una sola hebra'''

min_s    = 0.005    # tiempo minimo de espera en sleep_for
max_s    = 0.020    # tiempo máximo de espera en sleep_for

mtx = Lock()                 # mutex de escritura en pantalla

cont_prod = [0]*num_items # contadores de verificación: producidos
cont_cons = [0]*num_items # contadores de verificación: consumidos

tam_buffer = 10
buffer = [-1]*tam_buffer     # array continente de los datos
primera_libre=0     # índice de la primera posición libre
primera_ocupada=0   # índice de la primera posición ocupada

puede_consumir = Semaphore(0) 
'''inicializado a 0 porque aun no se ha producido nada. 
   Cuando se produzca, se debe emitir un signal y wait cuando se lea'''

puede_producir = Semaphore(tam_buffer) 
'''inicializado a tam_vec porque es el máximo de veces que puede escribir antes de que se lea. 
   Cuando se lea se debe emitir un signal y wait cuando se escriba'''

escribiendo = Lock() #mutex para escribir en el buffer
leyendo = Lock() #mutex para leer del buffer
#**********************************************************************
# funciones comunes a las dos soluciones (fifo y lifo)
#----------------------------------------------------------------------

def producir_dato( ih:int ):
    #sleep( random.uniform(min_s,max_s))
    dato_producido = int (ih*(num_items//np) + siguiente_dato[ih])
    siguiente_dato[ih]+=1
    with mtx:
        print(f"hebra productora {ih}, produce {dato_producido}")
    cont_prod[dato_producido]+=1
    return dato_producido

#----------------------------------------------------------------------

def consumir_dato( valor_consumir ):
    if num_items <= valor_consumir :
        raise Exception(f" valor a consumir === {valor_consumir}, num_items == {num_items} ")
    cont_cons[valor_consumir] +=1

    with mtx:
        print(f"                  hebra consumidora, consume: {valor_consumir}")
    sleep( 0.5)

#----------------------------------------------------------------------

def test_contadores():
    ok = True
    print ("comprobando contadores ....")

    for i in range(num_items):

        if ( cont_prod[i] != 1 ):
            print(f"error: valor {i} producido {cont_prod[i]} veces.")
            ok = False
        if ( cont_cons[i] != 1 ):
            print(f"error: valor {i} consumido {cont_cons[i]} veces.")
            ok = False
    if (ok): print("\nsolución (aparentemente) correcta.")


# *****************************************************************************

def  funcion_hebra_productora( ih:int ):
    global primera_libre
    for _ in range(num_items//np):

        dato = producir_dato( ih) 
        #Está fuera de la zona de exclusión porque es ajeno al vector de datos y no hay interferencias'''
        puede_producir.acquire() #espera hasta que haya huecos en el array

        with escribiendo:
            buffer[primera_libre] = dato #introduce el nuevo producto
            primera_libre = ( primera_libre + 1 )%tam_buffer

        puede_consumir.release() #notifica que hay un nuevo producto listo para ser leido

#----------------------------------------------------------------------

def funcion_hebra_consumidora(  ):
    global primera_ocupada
    for _ in range(num_items//nc):

        puede_consumir.acquire() #espera hasta que haya datos que puedan ser leidos

        with leyendo:
            dato = buffer[primera_ocupada] #extrae el producto
            primera_ocupada = ( primera_ocupada +1 )%tam_buffer

        puede_producir.release() #notifica que hay un nuevo hueco en el array

        consumir_dato( dato ) 
        '''Está fuera de la zona de exclusión porque es ajeno al vector de productos y 
        no hay interferencias, simplemente lo consume y tarda'''

#----------------------------------------------------------------------

def main():
    print( "--------------------------------------------------------------------")
    print("Problema del productor-consumidor únicos (semaforos). ")
    print( "--------------------------------------------------------------------" )


    hebra_productora = [Thread( target = funcion_hebra_productora , args=(ih,) ) for ih in range(np)]
    hebra_consumidora = [Thread( target = funcion_hebra_consumidora ) for _ in range(nc)]

    for i in hebra_consumidora: i.start()
    for i in hebra_productora: i.start()

    for i in hebra_productora: i.join()
    for i in hebra_consumidora: i.join()

    test_contadores()


start = time.time()
main()
end = time.time()
print(f"Tiempo de ejecución: {end-start}")