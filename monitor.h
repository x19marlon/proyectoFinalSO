#ifndef MONITOR_H
#define MONITOR_H

#define MAX_ID 50
#define MAX_HORA 30
#define MAX_RUTA_PIPE 256

// typedef struct {
//     char id[MAX_ID];
//     int humedad;
//     int rocio;
//     int presion;
//     char hora[MAX_HORA];
// } Estacion;

/*
 * Crea un pipe nominal en /tmp usando el nombre recibido.
 *
 * nombrePipe: nombre simple del pipe, por ejemplo "pipeMonitor"
 * rutaPipe: arreglo donde se guardará la ruta completa, por ejemplo "/tmp/pipeMonitor"
 * tamRuta: tamaño del arreglo rutaPipe
 *
 * Retorna:
 *  0 si todo salió bien
 * -1 si hubo error
 */
int crearPipe(const char *nombrePipe, char *rutaPipe, int tamRuta);

#endif