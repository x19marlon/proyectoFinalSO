#ifndef AGENTE_MEDICIONES_H
#define AGENTE_MEDICIONES_H

#include <stdbool.h>

#include "estacion.h"

#define MAX_LINEA 1024
#define MAX_RUTA_PIPE 256
#define MAX_NOMBRE_ARCHIVO 256

#define NOMBRE_SEMAFORO "/sem_pipe_monitor"


bool verificarArchivo(const char *archivo);

int leerCSV(const char *nombreArchivo, Estacion estaciones[]);

int enviarLecturaPorPipe(const char* nombrePipe, Estacion estaciones[], int cantidad, int tiempoSegundos);

#endif