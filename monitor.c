/*********************************************************************
*	Proyecto #1 Sistema Operativos
*	FELIPENE,MARLONVERGA,MALEJA
*
*	-Creación de named pipe para la comunicación
*	de los agentes.
*	-Uso de semaforos productor-consumidor (Control de buffer)
*********************************************************************/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
//libreria para manejo de archivos
#include<unistd.h>
#include<fcntl.h>
//Creación de pipes
#include<sys/types.h>
#include<sys/stat.h> //mkfifo
//Manejo de errores
#include<errno.h>
//Hilo recolector y consumidor
#include <pthreads.h>//Poxis
//Semaforos
#include<semaphore.h>
//Uso del headers
#include"common.h"




//Constantes de tamaño
//Maximo de lineas leidas en la tuberia
//Maximo del nombre de la estación (EK,ET,EU)
//Maximo de hora


