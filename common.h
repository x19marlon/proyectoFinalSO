/*********************
*
*	Objetivos:
*	- reutilización
*	- Coherencia
*	- Organización
*  - Lo que comparte el agente con el monitor
*
**********************/
//Proteccíón del archivo
#ifndef COMMON_H
#define COMMON_H
//Librerias basicas
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Constantes

/*RANGOS*/
#define MAX_ESTACION 10 //Tamaño máximo nombre de la estación (EU,ET,EK)
#define MAX_HORA 10 //Tamaño máximo para la hora
#define MAX_LINEA 256 //Tamaño máximo en la linea del archivo o pipe

/*PARAMETROS METEOROLÓGICOS*/
//Humedad (ínimos y máximos)
#define HUM_MIN 77
#define HUM_MAX 100
//Rocio (Mínimos y máximos)
#define ROCIO_MIN 3
#define ROCIO_MAX 12
//Presion
#define PRES_MIN 740
#define PRES_MAX 760


//estructura
typedef struct Medicion {
	char estacion [10];
	int humedad;
	int rocio;
	int presion;
	char hora [10];
} Medicion;

//Funciones
//(Ni verga de idea que funciones van ahi)
