#ifndef MONITOR_H
#define MONITOR_H

#include "estacion.h"

#define MAX_LINEA 1024
#define MAX_RUTA_PIPE 256
#define NOMBRE_ARCHIVO_CONSOLIDADO "archivo-consolidado.cvs"

int crearPipe(const char *nombrePipe, char *rutaPipe);

#endif