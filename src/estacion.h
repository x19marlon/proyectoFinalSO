// Evita que este archivo .h se incluya más de una vez durante la compilación.
// Si ESTACION_H no está definido, entra al contenido del archivo.
#ifndef ESTACION_H
#define ESTACION_H

// Tamaño máximo permitido para el nombre de una estación.
// Ejemplo: "EK", "BOGOTA", "ESTACION_NORTE".
#define MAX_NOMBRE_ESTACION 50

// Tamaño máximo permitido para guardar la hora.
// Ejemplo: "08:00:00".
#define MAX_HORA 20

// Cantidad máxima de lecturas o estaciones que puede almacenar el agente.
#define MAX_ESTACIONES 100

// Estructura que representa una lectura meteorológica de una estación.
typedef struct {
    // Nombre o identificador de la estación.
    char nombreEstacion[MAX_NOMBRE_ESTACION];

    // Valor de humedad registrado.
    int humedad;

    // Valor de rocío registrado.
    int rocio;

    // Valor de presión registrado.
    int presion;

    // Hora en la que se tomó la medición.
    char hora[MAX_HORA];
} Estacion;

// Cierra la protección contra inclusiones múltiples.
#endif
