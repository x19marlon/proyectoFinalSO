/*********************************************************************
*	Proyecto #1 Sistema Operativos
*	FELIPENE,MARLONVERGA,MALEJA
*
*	-Creación de named pipe para la comunicación
*	de los agentes.
*	-Uso de semaforos productor-consumidor (Control de buffer)
*********************************************************************/
#include "monitor.h"
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

int crearPipe(const char *nombrePipe, char *rutaPipe, int tamRuta) {
    if (nombrePipe == NULL || rutaPipe == NULL || tamRuta <= 0) {
        return -1;
    }

    int resultado = snprintf(rutaPipe, tamRuta, "/tmp/%s", nombrePipe);

    if (resultado < 0 || resultado >= tamRuta) {
        printf("Error: la ruta del pipe es demasiado larga.\n");
        return -1;
    }

    if (mkfifo(rutaPipe, 0666) == -1) {
        if (errno == EEXIST) {
            return 0;
        }

        perror("Error creando el pipe");
        return -1;
    }

    return 0;
}


//Constantes de tamaño
//Maximo de lineas leidas en la tuberia
//Maximo del nombre de la estación (EK,ET,EU)
//Maximo de hora


