// Librería estándar de entrada y salida.
// Se usa para printf, perror, fopen, fgets, fclose, etc.
#include <stdio.h>

// Librería estándar de utilidades.
// Se usa para atoi, que convierte cadenas a enteros.
#include <stdlib.h>

// Librería POSIX.
// Se usa para getopt, sleep, write y close.
#include <unistd.h>

// Librería para manejo de cadenas.
// Se usa para strlen, strcmp, strcpy, strtok, strchr, snprintf, etc.
#include <string.h>

// Librería para usar el tipo bool, junto con true y false.
#include <stdbool.h>

// Librería para abrir archivos o pipes con open.
// También permite usar banderas como O_WRONLY.
#include <fcntl.h>

// Librería para usar semáforos POSIX nombrados.
#include <semaphore.h>

// Archivo de cabecera propio del agente.
// Allí deberían estar las constantes, la estructura Estacion y prototipos.
#include "agenteMediciones.h"

// Define la carpeta por defecto donde se buscan los archivos .cvs.
// Si el usuario solo pasa "lluvioso.cvs", el programa buscará "docs/lluvioso.cvs".
#define CARPETA_DOCS "docs"

// Prototipo de la función que construye la ruta completa del archivo.
void construirRutaArchivo(const char *archivoEntrada, char *rutaArchivo, int tamRuta);

int main(int argc, char *argv[]) {

    // Arreglo donde se almacenan las mediciones leídas del archivo .cvs.
    Estacion estaciones[MAX_ESTACIONES];

    // Punteros para guardar los valores recibidos por consola.
    // archivo corresponde a -f.
    // tiempo corresponde a -t.
    // pipe_nombre corresponde a -p.
    char *archivo = NULL;
    char *tiempo = NULL;
    char *pipe_nombre = NULL;

    // Variable donde getopt guardará la opción leída.
    int opcion;

    // getopt procesa las banderas -f, -t y -p.
    // Los dos puntos indican que cada bandera espera un argumento.
    while ((opcion = getopt(argc, argv, "f:t:p:")) != -1) {

        switch (opcion) {

            // Bandera -f: nombre del archivo .cvs.
            case 'f':
                archivo = optarg;
                break;

            // Bandera -t: tiempo de espera entre lecturas.
            case 't':
                tiempo = optarg;
                break;

            // Bandera -p: nombre del pipe nominal.
            case 'p':
                pipe_nombre = optarg;
                break;

            // Si llega una bandera no reconocida o mal usada, se muestra el uso correcto.
            default:
                printf("Uso: %s -f archivo.cvs -t tiempo -p nombre_pipe\n", argv[0]);
                return 1;
        }
    }

    // Valida que el usuario haya enviado la bandera -f.
    if (archivo == NULL) {
        printf("Error: falta la bandera -f con el archivo.\n");
        return 1;
    }

    // Verifica que el archivo termine en .cvs.
    bool cvs = verificarArchivo(archivo);

    // Si el archivo no tiene extensión válida, termina el programa.
    if (!cvs) {
        printf("Error: Nombre archivo invalido. Debe terminar en .cvs\n");
        return 1;
    }

    // Valida que el usuario haya enviado la bandera -t.
    if (tiempo == NULL) {
        printf("Error: falta la bandera -t con el tiempo.\n");
        return 1;
    }

    // Convierte el tiempo recibido como texto a entero.
    int tiempoEntero = atoi(tiempo);

    // Valida que el tiempo no sea negativo.
    if (tiempoEntero < 0) {
        printf("Error: Tiempo inválido.\n");
        return 1;
    }

    // Valida que el usuario haya enviado la bandera -p.
    if (pipe_nombre == NULL) {
        printf("Error: falta la bandera -p con el nombre del pipe\n");
        return 1;
    }

    // Valida que el nombre del pipe no supere el tamaño máximo permitido.
    if (strlen(pipe_nombre) >= MAX_RUTA_PIPE) {
        printf("Error: Nombre pipe invalido\n");
        return 1;
    }

    // Valida que el nombre del archivo no supere el tamaño máximo permitido.
    if (strlen(archivo) >= MAX_NOMBRE_ARCHIVO) {
        printf("Error: Nombre archivo invalido\n");
        return 1;
    }

    // Arreglo donde se guardará la ruta final del archivo.
    // Puede ser una ruta recibida directamente o una ruta construida con docs/.
    char rutaArchivo[MAX_NOMBRE_ARCHIVO];

    // Construye la ruta del archivo a leer.
    construirRutaArchivo(archivo, rutaArchivo, sizeof(rutaArchivo));

    // Imprime información de configuración para verificar la ejecución.
    printf("Archivo recibido: %s\n", archivo);
    printf("Ruta usada: %s\n", rutaArchivo);
    printf("Tiempo: %s\n", tiempo);
    printf("Pipe: %s\n", pipe_nombre);

    // Lee el archivo .cvs y guarda las mediciones en el arreglo estaciones.
    int cantidad = leerCSV(rutaArchivo, estaciones);

    // Si leerCSV retorna -1, hubo error al abrir o leer el archivo.
    if (cantidad == -1) {
        printf("Error leyendo el archivo CSV.\n");
        return 1;
    }

    // Envía las lecturas almacenadas al Monitor usando el pipe nominal.
    // También se pasa el tiempo para esperar entre lectura y lectura.
    if (enviarLecturaPorPipe(pipe_nombre, estaciones, cantidad, tiempoEntero) == -1) {
        printf("Error enviando lecturas por el pipe.\n");
        return 1;
    }

    // Mensaje final si todo salió correctamente.
    printf("Lecturas enviadas correctamente.\n");

    return 0;
}

void construirRutaArchivo(const char *archivoEntrada, char *rutaArchivo, int tamRuta) {

    /*
        Si archivoEntrada ya tiene una ruta, por ejemplo:
        docs/lluvioso.cvs
        /home/usuario/docs/lluvioso.cvs

        entonces se usa tal cual.

        Si solo viene el nombre:
        lluvioso.cvs

        entonces se convierte en:
        docs/lluvioso.cvs
    */

    // strchr busca si el texto contiene el carácter '/'.
    // Si lo contiene, se asume que el usuario ya pasó una ruta.
    if (strchr(archivoEntrada, '/') != NULL) {
        snprintf(rutaArchivo, tamRuta, "%s", archivoEntrada);
    } else {
        // Si no contiene '/', se construye la ruta dentro de la carpeta docs.
        snprintf(rutaArchivo, tamRuta, "%s/%s", CARPETA_DOCS, archivoEntrada);
    }
}

// Esta función verifica que el archivo termine en .cvs.
bool verificarArchivo(const char *archivo) {

    // Calcula la longitud del nombre del archivo.
    int longitud = strlen(archivo);

    // Si tiene menos de 4 caracteres, no puede terminar en ".cvs".
    if (longitud < 4) {
        return false;
    }

    // Compara los últimos 4 caracteres del nombre con ".cvs".
    return strcmp(archivo + longitud - 4, ".cvs") == 0;
}

// Esta función lee el archivo .cvs y guarda cada línea válida en el arreglo estaciones.
int leerCSV(const char *nombreArchivo, Estacion estaciones[]) {
    // Abre el archivo en modo lectura.
    FILE *archivo = fopen(nombreArchivo, "r");

    // Si fopen devuelve NULL, el archivo no pudo abrirse.
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return -1;
    }

    // Arreglo temporal para guardar cada línea leída del archivo.
    char linea[MAX_LINEA];

    // Contador de mediciones válidas leídas.
    int cantidad = 0;

    // Lee el archivo línea por línea.
    while (fgets(linea, sizeof(linea), archivo) != NULL) {

        // Elimina el salto de línea '\n' si existe.
        linea[strcspn(linea, "\n")] = '\0';

        // Si la línea es ".", se interpreta como fin de archivo lógico.
        if (strcmp(linea, ".") == 0) {
            break;
        }

        // Separa la línea usando coma como delimitador.
        char *nombre = strtok(linea, ",");
        char *humedad = strtok(NULL, ",");
        char *rocio = strtok(NULL, ",");
        char *presion = strtok(NULL, ",");
        char *hora = strtok(NULL, ",");

        // Valida que la línea tenga todos los campos esperados.
        if (nombre == NULL || humedad == NULL || rocio == NULL || presion == NULL || hora == NULL) {
            printf("Línea inválida, se ignora.\n");
            continue;
        }

        // Copia el nombre de la estación dentro de la estructura.
        strcpy(estaciones[cantidad].nombreEstacion, nombre);

        // Convierte los valores numéricos de texto a entero.
        estaciones[cantidad].humedad = atoi(humedad);
        estaciones[cantidad].rocio = atoi(rocio);
        estaciones[cantidad].presion = atoi(presion);

        // Copia la hora dentro de la estructura.
        strcpy(estaciones[cantidad].hora, hora);

        // Aumenta la cantidad de mediciones válidas.
        cantidad++;

        // Evita superar el tamaño máximo del arreglo de estaciones.
        if (cantidad >= MAX_ESTACIONES) {
            printf("Se alcanzó el máximo de estaciones.\n");
            break;
        }
    }

    // Cierra el archivo después de leerlo.
    fclose(archivo);

    // Retorna cuántas mediciones válidas se leyeron.
    return cantidad;
}

// Esta función envía las lecturas al Monitor por medio del pipe nominal.
int enviarLecturaPorPipe(const char* nombrePipe, Estacion estaciones[], int cantidad, int tiempoSegundos) {
    // Ruta completa del pipe nominal.
    char rutaPipe[MAX_RUTA_PIPE];

    // Construye la ruta del pipe en /tmp.
    snprintf(rutaPipe, sizeof(rutaPipe), "/tmp/%s", nombrePipe);

    // Abre el pipe nominal solo para escritura.
    int fdPipe = open(rutaPipe, O_WRONLY);

    // Si open retorna -1, no se pudo abrir el pipe.
    if (fdPipe == -1) {
        perror("Error abriendo el pipe nominal");
        return -1;
    }

    // Abre el semáforo nombrado creado por el Monitor.
    sem_t *semaforo = sem_open(NOMBRE_SEMAFORO, 0);

    // Si SEM_FAILED, el semáforo no existe o no pudo abrirse.
    if (semaforo == SEM_FAILED) {
        perror("Error abriendo el semáforo");
        close(fdPipe);
        return -1;
    }

    // Recorre todas las mediciones leídas del archivo.
    for (int i = 0; i < cantidad; i++) {
        // Mensaje que se enviará por el pipe.
        char mensaje[MAX_LINEA];

        // Convierte la estructura Estacion nuevamente a una línea tipo CSV.
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

        // Entrada a la sección crítica.
        // Solo un agente puede escribir al pipe a la vez.
        sem_wait(semaforo);

        // Envía la línea al pipe nominal.
        write(fdPipe, mensaje, strlen(mensaje));

        // Salida de la sección crítica.
        // Libera el semáforo para que otro agente pueda escribir.
        sem_post(semaforo);

        // Muestra en terminal la lectura enviada.
        printf("Lectura enviada: %s", mensaje);

        // Espera el número de segundos indicado por -t antes de enviar la siguiente lectura.
        if (i < cantidad - 1) {
            sleep(tiempoSegundos);
        }
    }

    // Cierra el semáforo en este proceso.
    sem_close(semaforo);

    // Cierra el descriptor del pipe.
    close(fdPipe);

    return 0;
}
