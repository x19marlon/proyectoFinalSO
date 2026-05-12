#ifndef MONITOR_H
#define MONITOR_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#include "estacion.h"

#define MAX_RUTA_PIPE 256
#define MAX_LINEA 1024
#define ARCHIVO_CONSOLIDADO "archivo-consolidado.cvs"
#define NOMBRE_SEMAFORO "/sem_pipe_monitor"

typedef struct {
    int cantidad;

    int sumaHumedad;
    int sumaRocio;
    int sumaPresion;

    int minHumedad;
    int maxHumedad;

    int minRocio;
    int maxRocio;

    int minPresion;
    int maxPresion;
} Estadisticas;

typedef struct {
    int lluvioso;
    int nublado;
    int fresco;
    int sinCategoria;
} ConteoCategorias;

typedef struct {
    Estacion *datos;
    int capacidad;
    int inicio;
    int fin;
    int cantidad;
    int terminado;

    pthread_mutex_t mutex;
    pthread_cond_t noLleno;
    pthread_cond_t noVacio;
} BufferEstaciones;

typedef struct {
    char rutaPipe[MAX_RUTA_PIPE];

    BufferEstaciones buffer;
    Estadisticas estadisticas;
    ConteoCategorias conteoCategorias;

    FILE *archivoConsolidado;
} MonitorContext;

int leerArgumentosMonitor(
    int argc,
    char *argv[],
    int *tamBuffer,
    char *rutaPipe
);

int crearPipeNominal(const char *rutaPipe);

void inicializarBuffer(BufferEstaciones *buffer, int capacidad);
void destruirBuffer(BufferEstaciones *buffer);
void insertarBuffer(BufferEstaciones *buffer, Estacion estacion);
bool sacarBuffer(BufferEstaciones *buffer, Estacion *estacion);
void finalizarBuffer(BufferEstaciones *buffer);

void *hiloRecolector(void *arg);
void *hiloProcesador(void *arg);

void inicializarEstadisticas(Estadisticas *estadisticas);
void actualizarEstadisticas(Estadisticas *estadisticas, Estacion estacion);
void imprimirResumen(Estadisticas estadisticas);

void inicializarConteoCategorias(ConteoCategorias *conteo);
void clasificarLectura(Estacion estacion, ConteoCategorias *conteo);
void imprimirCategorias(ConteoCategorias conteo);

int convertirLineaAEstacion(const char *linea, Estacion *estacion);

#endif