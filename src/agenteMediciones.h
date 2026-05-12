#ifndef AGENTE_MEDICIONES_H
#define AGENTE_MEDICIONES_H

#include <stdbool.h>
#include "estacion.h"

#define MAX_LINEA 1024
#define MAX_RUTA_PIPE 256
#define MAX_NOMBRE_ARCHIVO 256

#ifndef CARPETA_DOCS
#define CARPETA_DOCS "docs"
#endif

#define NOMBRE_SEMAFORO "/sem_pipe_monitor"

void construirRutaArchivo(
    const char *archivoEntrada,
    char *rutaArchivo,
    int tamRuta
);

bool verificarArchivo(const char *archivo);

int leerCSV(
    const char *nombreArchivo,
    Estacion estaciones[]
);

int enviarLecturaPorPipe(
    const char *nombrePipe,
    Estacion estaciones[],
    int cantidad,
    int tiempoSegundos
);

#endif