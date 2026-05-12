#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#include "monitor.h"

#define NOMBRE_SEMAFORO "/sem_pipe_monitor"
#define MAX_RUTA_PIPE 256
#define MAX_LINEA 1024

int main(int argc, char *argv[]) {
    int humedad=0, rocio=0, presion=0;

    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        printf("Uso: %s -p nombre_pipe\n", argv[0]);
        return 1;
    }

    char rutaPipe[MAX_RUTA_PIPE];

    snprintf(rutaPipe, sizeof(rutaPipe), "/tmp/%s", argv[2]);

    if (mkfifo(rutaPipe, 0666) == -1) {
        if (errno != EEXIST) {
            perror("Error creando pipe");
            return 1;
        }
    }

    printf("Monitor esperando datos en: %s\n", rutaPipe);
    
    sem_t *semaforo = sem_open(NOMBRE_SEMAFORO, O_CREAT, 0666, 1);

    if (semaforo == SEM_FAILED) {
        perror("Error creando semáforo");
        return 1;
    }

    int fdPipe = open(rutaPipe, O_RDONLY);

    if (fdPipe == -1) {
        perror("Error abriendo pipe");
        return 1;
    }

    FILE *pipe = fdopen(fdPipe, "r");

    if (pipe == NULL) {
        perror("Error convirtiendo pipe a FILE");
        close(fdPipe);
        return 1;
    }

    FILE *archivoConsolidado = fopen("archivo-consolidado.cvs", "w");

    if (archivoConsolidado == NULL) {
        perror("Error creando archivo consolidado");
        fclose(pipe);
        return 1;
    }

    char linea[MAX_LINEA];

    

    while (fgets(linea, sizeof(linea), pipe) != NULL) {
        linea[strcspn(linea, "\n")] = '\0';

        printf("Lectura recibida: %s\n", linea);

        fprintf(archivoConsolidado, "%s\n", linea);

        fflush(archivoConsolidado);
    }

    fclose(archivoConsolidado);
    fclose(pipe);

    printf("Archivo consolidado creado: archivo-consolidado.cvs\n");

    return 0;
}