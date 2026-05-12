#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <semaphore.h>

#include "agenteMediciones.h"
int main(int argc, char *argv[]) {

	Estacion estaciones[MAX_ESTACIONES];

    char *archivo = NULL;
    char *tiempo = NULL;
    char *pipe_nombre = NULL;

    int opcion;

    while ((opcion = getopt(argc, argv, "f:t:p:")) != -1) {

        switch (opcion) {

            case 'f':
                archivo = optarg;
                break;

            case 't':
                tiempo = optarg;
                break;

            case 'p':
                pipe_nombre = optarg;
                break;

            default:
                printf("Uso: %s -f archivo.csv -t tiempo -p nombre_pipe\n", argv[0]);
                return 1;
        }
    }
	bool cvs=verificarArchivo(archivo);



    if (archivo == NULL || !cvs) {
        printf("Error: Nombre archivo invalido.\n");
        return 1;
    }

    int tiempoEntero = atoi(tiempo);
    if (tiempo == NULL || tiempoEntero < 0) {
        printf("Error: Tiempo inválido.\n");
        return 1;
    }

    if (pipe_nombre == NULL) {
        printf("Error: falta la bandera -p con el nombre del pipe\n");
        return 1;
    }

    if(strlen(pipe_nombre)> MAX_RUTA_PIPE) {
		printf("Error: Nombre pipe invalido");
		return 1;
	}

	if(strlen(archivo)> MAX_NOMBRE_ARCHIVO) {
		printf("Error: Nombre pipe invalido");
		return 1;
	}

    printf("Archivo CSV: %s\n", archivo);
    printf("Tiempo: %s\n", tiempo);
    printf("Pipe: %s\n", pipe_nombre);

    int cantidad = leerCSV(archivo, estaciones);

    if (cantidad == -1) {
        printf("Error leyendo el archivo CSV.\n");
        return 1;
    }

  if (enviarLecturaPorPipe(pipe_nombre, estaciones, cantidad, tiempoEntero) == -1) {
        printf("Error enviando lecturas por el pipe.\n");
        return 1;
    }

    printf("Lecturas enviadas correctamente.\n");

    return 0;
}

//  Esta funcion verifica que los ultimos caracteres de archivo sean de las extension cvs
bool verificarArchivo(const char *archivo){


	int s = strlen(archivo)-1;
	int v = strlen(archivo)-2;
	int c = strlen(archivo)-3;

	if(archivo[c] == 'c'&& archivo[v] == 'v'&& archivo[s] == 's') return true;
	else return false;


}
// Esta funcion lee cvs
int leerCSV(const char *nombreArchivo, Estacion estaciones[]) {
    FILE *archivo = fopen(nombreArchivo, "r");

    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return -1;
    }

    char linea[MAX_LINEA];
    int cantidad = 0;

    while (fgets(linea, sizeof(linea), archivo) != NULL) {

        // Quitar salto de línea '\n'
        linea[strcspn(linea, "\n")] = '\0';

        // Si la línea es ".", se detiene la lectura
        if (strcmp(linea, ".") == 0) {
            break;
        }

        // Separar la línea por comas
        char *nombre = strtok(linea, ",");
        char *humedad = strtok(NULL, ",");
        char *rocio = strtok(NULL, ",");
        char *presion = strtok(NULL, ",");
        char *hora = strtok(NULL, ",");

        // Validar que la línea tenga todos los datos
        if (nombre == NULL || humedad == NULL || rocio == NULL || presion == NULL || hora == NULL) {
            printf("Línea inválida, se ignora.\n");
            continue;
        }

        // Guardar los datos en el arreglo de structs
        strcpy(estaciones[cantidad].nombreEstacion, nombre);
        estaciones[cantidad].humedad = atoi(humedad);
        estaciones[cantidad].rocio = atoi(rocio);
        estaciones[cantidad].presion = atoi(presion);
        strcpy(estaciones[cantidad].hora, hora);

        cantidad++;

        if (cantidad >= MAX_ESTACIONES) {
            printf("Se alcanzó el máximo de estaciones.\n");
            break;
        }
    }

    fclose(archivo);

    return cantidad;
}

int enviarLecturaPorPipe(const char* nombrePipe, Estacion estaciones[], int cantidad, int tiempoSegundos) {
    char rutaPipe[MAX_RUTA_PIPE];

    snprintf(rutaPipe, sizeof(rutaPipe), "/tmp/%s", nombrePipe);

    int fdPipe = open(rutaPipe, O_WRONLY);

    if (fdPipe == -1) {
        perror("Error abriendo el pipe nominal");
        return -1;
    }

    sem_t *semaforo = sem_open(NOMBRE_SEMAFORO, 0);

    if (semaforo == SEM_FAILED) {
        perror("Error abriendo el semáforo");
        close(fdPipe);
        return -1;
    }

    for (int i = 0; i < cantidad; i++) {
        char mensaje[MAX_LINEA];

        snprintf(
            mensaje,
            sizeof(mensaje),
            "%s,%d,%d,%d,%s\n",
            estaciones[i].nombreEstacion,
            estaciones[i].humedad,
            estaciones[i].rocio,
            estaciones[i].presion,
            estaciones[i].hora
        );

        sem_wait(semaforo);

        write(fdPipe, mensaje, strlen(mensaje));

        sem_post(semaforo);

        printf("Lectura enviada: %s", mensaje);

        if (i < cantidad - 1) {
            sleep(tiempoSegundos);
        }
    }

    sem_close(semaforo);
    close(fdPipe);

    return 0;
}