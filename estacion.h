#ifndef ESTACION_H
#define ESTACION_H

#define MAX_NOMBRE_ESTACION 50
#define MAX_HORA 20
#define MAX_ESTACIONES 100

typedef struct {
    char nombreEstacion[MAX_NOMBRE_ESTACION];
    int humedad;
    int rocio;
    int presion;
    char hora[MAX_HORA];
} Estacion;

#endif