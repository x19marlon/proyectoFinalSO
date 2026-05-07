/***********************************
*	 Agente de mediciones
*
*	- Descripción:
*	- Lee datos del archivo
*	- Convertir cada linea en medicion
*	- Enviar datos por la pipe
*
*************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// libreria para manejo de archivos
#include <unistd.h>
#include <fcntl.h>

// Creación de pipes
#include <sys/types.h>
#include <sys/stat.h> // mkfifo

// Manejo de errores
#include <errno.h>

// Hilo recolector y consumidor
#include <pthread.h> // Poxis

// Semaforos
#include <semaphore.h>

// Uso del headers
#include "common.h"

//Realizar funcion para lectura de archivos

int main (){

    // declarar archivo
/*FILE es el tipo de dato que representa el archivo*/
	FILE *archivo;
    // abrir archivo
/*fopen es para abrir el archivo*/
	archivo = fopen ( ruta del archivo donde estan los datos xd, "r");
    // validar apertura
	if (archivo == NULL){
	printf("Error al abrir el archivo\n"
	return 1;
	}
    // leer línea por línea
/*fgets lectura de el contenido por linea del archivo */

	while(fgets (linea, sizeof(linea), archivo) !=NULL){
	printf("%s", linea);


/*Detectar el fin del archivo*/

//quitar el salto de linea
	linea [strcspn(linea, "\n")] = 0;

//comparación con epunto final del archivo
	if(strcmp(linea, ".") == 0){
	break;
	}

	printf("%\n", linea);

}

    // imprimir líneas
	fclose(archivo);

    //cerrar archivo

	return 0;
}




