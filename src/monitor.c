#include "monitor.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

int main(int argc, char *argv[]) {
    int tamBuffer;
    char rutaPipe[MAX_RUTA_PIPE];

    // Lee los argumentos del programa: tamaño del buffer y nombre del pipe.
    if (leerArgumentosMonitor(argc, argv, &tamBuffer, rutaPipe) == -1) {
        printf("Uso: %s -b tamBuffer -p nombre_pipe\n", argv[0]);
        return 1;
    }

    // Reinicia el semáforo por si quedó creado en una ejecución anterior.
    sem_unlink(NOMBRE_SEMAFORO);

    // Crea un semáforo nombrado con valor inicial 1.
    sem_t *semaforo = sem_open(NOMBRE_SEMAFORO, O_CREAT, 0666, 1);

    if (semaforo == SEM_FAILED) {
        perror("Error creando semáforo");
        return 1;
    }

    // Crea el pipe nominal donde el monitor recibirá las lecturas.
    if (crearPipeNominal(rutaPipe) == -1) {
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);
        return 1;
    }

    // Crea el archivo CSV donde se consolidarán las lecturas procesadas.
    FILE *archivoConsolidado = fopen(ARCHIVO_CONSOLIDADO, "w");

    if (archivoConsolidado == NULL) {
        perror("Error creando archivo consolidado");
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);
        return 1;
    }

    // Encabezado del archivo CSV.
    fprintf(
        archivoConsolidado,
        "tipo,nombreEstacion,humedad,rocio,presion,hora\n"
    );

    // Contexto compartido entre los hilos recolector y procesador.
    MonitorContext contexto;

    strcpy(contexto.rutaPipe, rutaPipe);
    contexto.archivoConsolidado = archivoConsolidado;

    // Inicializa estructuras principales del monitor.
    inicializarBuffer(&contexto.buffer, tamBuffer);
    inicializarEstadisticas(&contexto.estadisticas);
    inicializarConteoCategorias(&contexto.conteoCategorias);

    pthread_t recolector;
    pthread_t procesador;

    printf("Monitor esperando datos en: %s\n", rutaPipe);
    printf("Tamaño del buffer: %d lecturas\n", tamBuffer);

    // Crea el hilo procesador, encargado de sacar datos del buffer y procesarlos.
    if (pthread_create(&procesador, NULL, hiloProcesador, &contexto) != 0) {
        printf("Error creando Hilo Procesador.\n");

        destruirBuffer(&contexto.buffer);
        fclose(archivoConsolidado);
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);

        return 1;
    }

    // Crea el hilo recolector, encargado de leer datos desde el pipe.
    if (pthread_create(&recolector, NULL, hiloRecolector, &contexto) != 0) {
        printf("Error creando Hilo Recolector.\n");

        finalizarBuffer(&contexto.buffer);
        pthread_join(procesador, NULL);

        destruirBuffer(&contexto.buffer);
        fclose(archivoConsolidado);
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);

        return 1;
    }

    // Espera a que ambos hilos terminen.
    pthread_join(recolector, NULL);
    pthread_join(procesador, NULL);

    fclose(archivoConsolidado);

    printf("\nArchivo consolidado creado: %s\n", ARCHIVO_CONSOLIDADO);

    // Muestra los resultados finales.
    imprimirResumen(contexto.estadisticas);
    imprimirCategorias(contexto.conteoCategorias);

    // Libera recursos utilizados por el programa.
    destruirBuffer(&contexto.buffer);

    sem_close(semaforo);
    sem_unlink(NOMBRE_SEMAFORO);

    unlink(rutaPipe);

    return 0;
}

int leerArgumentosMonitor(
    int argc,
    char *argv[],
    int *tamBuffer,
    char *rutaPipe
) {
    char *pipeNombre = NULL;
    *tamBuffer = 0;

    int opcion;

    // Procesa las opciones -b y -p recibidas por consola.
    while ((opcion = getopt(argc, argv, "b:p:")) != -1) {
        switch (opcion) {
            case 'b':
                *tamBuffer = atoi(optarg);
                break;

            case 'p':
                pipeNombre = optarg;
                break;

            default:
                return -1;
        }
    }

    // Valida que el tamaño del buffer y el nombre del pipe sean correctos.
    if (*tamBuffer <= 0 || pipeNombre == NULL) {
        return -1;
    }

    // Construye la ruta completa del pipe dentro de /tmp.
    int resultado = snprintf(rutaPipe, MAX_RUTA_PIPE, "/tmp/%s", pipeNombre);

    if (resultado < 0 || resultado >= MAX_RUTA_PIPE) {
        printf("Error: nombre de pipe demasiado largo.\n");
        return -1;
    }

    return 0;
}

int crearPipeNominal(const char *rutaPipe) {
    // Crea un FIFO para comunicación entre procesos.
    if (mkfifo(rutaPipe, 0666) == -1) {
        // Si ya existe, no se considera error.
        if (errno != EEXIST) {
            perror("Error creando pipe nominal");
            return -1;
        }
    }

    return 0;
}

void inicializarBuffer(BufferEstaciones *buffer, int capacidad) {
    // Reserva memoria para almacenar las lecturas del buffer.
    buffer->datos = malloc(sizeof(Estacion) * capacidad);

    if (buffer->datos == NULL) {
        perror("Error reservando memoria para el buffer");
        exit(1);
    }

    // Inicializa los valores del buffer circular.
    buffer->capacidad = capacidad;
    buffer->inicio = 0;
    buffer->fin = 0;
    buffer->cantidad = 0;
    buffer->terminado = 0;

    // Inicializa herramientas de sincronización entre hilos.
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->noLleno, NULL);
    pthread_cond_init(&buffer->noVacio, NULL);
}

void destruirBuffer(BufferEstaciones *buffer) {
    // Libera memoria y destruye mecanismos de sincronización.
    free(buffer->datos);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->noLleno);
    pthread_cond_destroy(&buffer->noVacio);
}

void insertarBuffer(BufferEstaciones *buffer, Estacion estacion) {
    pthread_mutex_lock(&buffer->mutex);

    // Si el buffer está lleno, el hilo espera hasta que haya espacio.
    while (buffer->cantidad == buffer->capacidad) {
        pthread_cond_wait(&buffer->noLleno, &buffer->mutex);
    }

    // Inserta la lectura en el buffer circular.
    buffer->datos[buffer->fin] = estacion;
    buffer->fin = (buffer->fin + 1) % buffer->capacidad;
    buffer->cantidad++;

    // Avisa al procesador que ya hay datos disponibles.
    pthread_cond_signal(&buffer->noVacio);
    pthread_mutex_unlock(&buffer->mutex);
}

bool sacarBuffer(BufferEstaciones *buffer, Estacion *estacion) {
    pthread_mutex_lock(&buffer->mutex);

    // Si el buffer está vacío, espera a que lleguen datos o a que finalice.
    while (buffer->cantidad == 0 && !buffer->terminado) {
        pthread_cond_wait(&buffer->noVacio, &buffer->mutex);
    }

    // Si ya no hay datos y el recolector terminó, se finaliza el procesamiento.
    if (buffer->cantidad == 0 && buffer->terminado) {
        pthread_mutex_unlock(&buffer->mutex);
        return false;
    }

    // Extrae una lectura del buffer circular.
    *estacion = buffer->datos[buffer->inicio];
    buffer->inicio = (buffer->inicio + 1) % buffer->capacidad;
    buffer->cantidad--;

    // Avisa al recolector que hay espacio disponible.
    pthread_cond_signal(&buffer->noLleno);
    pthread_mutex_unlock(&buffer->mutex);

    return true;
}

void finalizarBuffer(BufferEstaciones *buffer) {
    pthread_mutex_lock(&buffer->mutex);

    // Marca que ya no se insertarán más datos.
    buffer->terminado = 1;

    // Despierta al procesador si estaba esperando datos.
    pthread_cond_broadcast(&buffer->noVacio);

    pthread_mutex_unlock(&buffer->mutex);
}

void *hiloRecolector(void *arg) {
    MonitorContext *contexto = (MonitorContext *) arg;

    // Abre el pipe en modo lectura.
    int fdPipe = open(contexto->rutaPipe, O_RDONLY);

    if (fdPipe == -1) {
        perror("Error abriendo pipe en Hilo Recolector");
        finalizarBuffer(&contexto->buffer);
        pthread_exit(NULL);
    }

    // Convierte el descriptor del pipe a FILE* para leer con fgets.
    FILE *pipe = fdopen(fdPipe, "r");

    if (pipe == NULL) {
        perror("Error convirtiendo pipe a FILE en Hilo Recolector");
        close(fdPipe);
        finalizarBuffer(&contexto->buffer);
        pthread_exit(NULL);
    }

    char linea[MAX_LINEA];

    // Lee cada línea recibida desde el pipe.
    while (fgets(linea, sizeof(linea), pipe) != NULL) {
        linea[strcspn(linea, "\n")] = '\0';

        Estacion estacion;

        // Convierte la línea CSV en una estructura Estacion.
        if (!convertirLineaAEstacion(linea, &estacion)) {
            printf("Línea inválida recibida: %s\n", linea);
            continue;
        }

        printf(
            "[Recolector] Lectura recibida: %s,%d,%d,%d,%s\n",
            estacion.nombreEstacion,
            estacion.humedad,
            estacion.rocio,
            estacion.presion,
            estacion.hora
        );

        // Envía la lectura al buffer para que el procesador la consuma.
        insertarBuffer(&contexto->buffer, estacion);
    }

    fclose(pipe);

    // Indica que ya no llegarán más lecturas.
    finalizarBuffer(&contexto->buffer);

    pthread_exit(NULL);
}

void *hiloProcesador(void *arg) {
    MonitorContext *contexto = (MonitorContext *) arg;

    Estacion estacion;

    // Procesa lecturas mientras existan datos en el buffer.
    while (sacarBuffer(&contexto->buffer, &estacion)) {
        printf(
            "[Procesador] Procesando: %s,%d,%d,%d,%s\n",
            estacion.nombreEstacion,
            estacion.humedad,
            estacion.rocio,
            estacion.presion,
            estacion.hora
        );

        // Guarda la lectura en el archivo consolidado.
        fprintf(
            contexto->archivoConsolidado,
            "LECTURA,%s,%d,%d,%d,%s\n",
            estacion.nombreEstacion,
            estacion.humedad,
            estacion.rocio,
            estacion.presion,
            estacion.hora
        );

        fflush(contexto->archivoConsolidado);

        // Actualiza estadísticas y categorías meteorológicas.
        actualizarEstadisticas(&contexto->estadisticas, estacion);
        clasificarLectura(estacion, &contexto->conteoCategorias);
    }

    pthread_exit(NULL);
}

int convertirLineaAEstacion(const char *linea, Estacion *estacion) {
    char copia[MAX_LINEA];

    // Se copia la línea porque strtok modifica el texto original.
    strncpy(copia, linea, sizeof(copia));
    copia[sizeof(copia) - 1] = '\0';

    // Se separa la línea usando comas.
    char *nombre = strtok(copia, ",");
    char *humedad = strtok(NULL, ",");
    char *rocio = strtok(NULL, ",");
    char *presion = strtok(NULL, ",");
    char *hora = strtok(NULL, ",");

    // Valida que la línea tenga todos los campos esperados.
    if (nombre == NULL ||
        humedad == NULL ||
        rocio == NULL ||
        presion == NULL ||
        hora == NULL) {

        return 0;
    }

    // Copia los datos de texto y convierte los valores numéricos.
    strncpy(estacion->nombreEstacion, nombre, sizeof(estacion->nombreEstacion));
    estacion->nombreEstacion[sizeof(estacion->nombreEstacion) - 1] = '\0';

    estacion->humedad = atoi(humedad);
    estacion->rocio = atoi(rocio);
    estacion->presion = atoi(presion);

    strncpy(estacion->hora, hora, sizeof(estacion->hora));
    estacion->hora[sizeof(estacion->hora) - 1] = '\0';

    return 1;
}

void inicializarEstadisticas(Estadisticas *estadisticas) {
    // Inicializa acumuladores y valores de control.
    estadisticas->cantidad = 0;

    estadisticas->sumaHumedad = 0;
    estadisticas->sumaRocio = 0;
    estadisticas->sumaPresion = 0;

    estadisticas->minHumedad = 0;
    estadisticas->maxHumedad = 0;

    estadisticas->minRocio = 0;
    estadisticas->maxRocio = 0;

    estadisticas->minPresion = 0;
    estadisticas->maxPresion = 0;
}

void actualizarEstadisticas(Estadisticas *estadisticas, Estacion estacion) {
    // En la primera lectura, se inicializan mínimos y máximos.
    if (estadisticas->cantidad == 0) {
        estadisticas->minHumedad = estacion.humedad;
        estadisticas->maxHumedad = estacion.humedad;

        estadisticas->minRocio = estacion.rocio;
        estadisticas->maxRocio = estacion.rocio;

        estadisticas->minPresion = estacion.presion;
        estadisticas->maxPresion = estacion.presion;
    }

    // Acumula valores para calcular promedios.
    estadisticas->sumaHumedad += estacion.humedad;
    estadisticas->sumaRocio += estacion.rocio;
    estadisticas->sumaPresion += estacion.presion;

    // Actualiza mínimos y máximos.
    if (estacion.humedad < estadisticas->minHumedad) {
        estadisticas->minHumedad = estacion.humedad;
    }

    if (estacion.humedad > estadisticas->maxHumedad) {
        estadisticas->maxHumedad = estacion.humedad;
    }

    if (estacion.rocio < estadisticas->minRocio) {
        estadisticas->minRocio = estacion.rocio;
    }

    if (estacion.rocio > estadisticas->maxRocio) {
        estadisticas->maxRocio = estacion.rocio;
    }

    if (estacion.presion < estadisticas->minPresion) {
        estadisticas->minPresion = estacion.presion;
    }

    if (estacion.presion > estadisticas->maxPresion) {
        estadisticas->maxPresion = estacion.presion;
    }

    estadisticas->cantidad++;
}

void imprimirResumen(Estadisticas estadisticas) {
    // Evita dividir entre cero si no se recibieron datos.
    if (estadisticas.cantidad == 0) {
        printf("No se recibieron datos para calcular estadísticas.\n");
        return;
    }

    // Calcula promedios.
    double promedioHumedad = (double) estadisticas.sumaHumedad / estadisticas.cantidad;
    double promedioRocio = (double) estadisticas.sumaRocio / estadisticas.cantidad;
    double promedioPresion = (double) estadisticas.sumaPresion / estadisticas.cantidad;

    printf("\n===== RESUMEN DE MEDICIONES =====\n");

    printf("Cantidad de lecturas procesadas: %d\n", estadisticas.cantidad);

    printf("\nHumedad:\n");
    printf("  Promedio: %.2f\n", promedioHumedad);
    printf("  Mínimo: %d\n", estadisticas.minHumedad);
    printf("  Máximo: %d\n", estadisticas.maxHumedad);

    printf("\nRocío:\n");
    printf("  Promedio: %.2f\n", promedioRocio);
    printf("  Mínimo: %d\n", estadisticas.minRocio);
    printf("  Máximo: %d\n", estadisticas.maxRocio);

    printf("\nPresión:\n");
    printf("  Promedio: %.2f\n", promedioPresion);
    printf("  Mínimo: %d\n", estadisticas.minPresion);
    printf("  Máximo: %d\n", estadisticas.maxPresion);

    printf("=================================\n");
}

void inicializarConteoCategorias(ConteoCategorias *conteo) {
    // Inicializa el contador de cada categoría.
    conteo->lluvioso = 0;
    conteo->nublado = 0;
    conteo->fresco = 0;
    conteo->sinCategoria = 0;
}

void clasificarLectura(Estacion estacion, ConteoCategorias *conteo) {
    // Clasifica la lectura según humedad, rocío y presión.
    if (estacion.humedad > 90 &&
        estacion.rocio > 9 &&
        estacion.presion < 750) {

        conteo->lluvioso++;
        return;
    }

    if (estacion.humedad >= 80 &&
        estacion.humedad <= 95 &&
        estacion.rocio > 8 &&
        estacion.presion == 751) {

        conteo->nublado++;
        return;
    }

    if (estacion.humedad < 80 &&
        estacion.rocio >= 5 &&
        estacion.rocio <= 8 &&
        estacion.presion > 754) {

        conteo->fresco++;
        return;
    }

    conteo->sinCategoria++;
}

void imprimirCategorias(ConteoCategorias conteo) {
    printf("\n===== CATEGORIZACIÓN METEOROLÓGICA =====\n");

    printf("Lecturas lluviosas: %d\n", conteo.lluvioso);
    printf("Lecturas nubladas: %d\n", conteo.nublado);
    printf("Lecturas frescas: %d\n", conteo.fresco);
    printf("Lecturas sin categoría: %d\n", conteo.sinCategoria);

    printf("\nCategoría predominante: ");

    // Determina la categoría más frecuente entre lluvioso, nublado y fresco.
    if (conteo.lluvioso == 0 &&
        conteo.nublado == 0 &&
        conteo.fresco == 0) {

        printf("No categorizado\n");
    }
    else if (conteo.lluvioso >= conteo.nublado &&
             conteo.lluvioso >= conteo.fresco) {

        printf("Lluvioso\n");
    }
    else if (conteo.nublado >= conteo.lluvioso &&
             conteo.nublado >= conteo.fresco) {

        printf("Nublado\n");
    }
    else {
        printf("Fresco\n");
    }

    printf("========================================\n");
}