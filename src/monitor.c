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

<<<<<<< HEAD
=======
    // Lee los argumentos del programa: tamaño del buffer y nombre del pipe.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (leerArgumentosMonitor(argc, argv, &tamBuffer, rutaPipe) == -1) {
        printf("Uso: %s -b tamBuffer -p nombre_pipe\n", argv[0]);
        return 1;
    }

<<<<<<< HEAD
    /*
        Reiniciamos el semáforo por si quedó creado de una ejecución anterior.
        Esto evita problemas si el programa se cerró de forma inesperada.
    */
    sem_unlink(NOMBRE_SEMAFORO);

=======
    // Reinicia el semáforo por si quedó creado en una ejecución anterior.
    sem_unlink(NOMBRE_SEMAFORO);

    // Crea un semáforo nombrado con valor inicial 1.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    sem_t *semaforo = sem_open(NOMBRE_SEMAFORO, O_CREAT, 0666, 1);

    if (semaforo == SEM_FAILED) {
        perror("Error creando semáforo");
        return 1;
    }

<<<<<<< HEAD
=======
    // Crea el pipe nominal donde el monitor recibirá las lecturas.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (crearPipeNominal(rutaPipe) == -1) {
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);
        return 1;
    }

<<<<<<< HEAD
=======
    // Crea el archivo CSV donde se consolidarán las lecturas procesadas.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    FILE *archivoConsolidado = fopen(ARCHIVO_CONSOLIDADO, "w");

    if (archivoConsolidado == NULL) {
        perror("Error creando archivo consolidado");
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);
        return 1;
    }

<<<<<<< HEAD
=======
    // Encabezado del archivo CSV.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    fprintf(
        archivoConsolidado,
        "tipo,nombreEstacion,humedad,rocio,presion,hora\n"
    );

<<<<<<< HEAD
=======
    // Contexto compartido entre los hilos recolector y procesador.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    MonitorContext contexto;

    strcpy(contexto.rutaPipe, rutaPipe);
    contexto.archivoConsolidado = archivoConsolidado;

<<<<<<< HEAD
=======
    // Inicializa estructuras principales del monitor.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    inicializarBuffer(&contexto.buffer, tamBuffer);
    inicializarEstadisticas(&contexto.estadisticas);
    inicializarConteoCategorias(&contexto.conteoCategorias);

    pthread_t recolector;
    pthread_t procesador;

    printf("Monitor esperando datos en: %s\n", rutaPipe);
    printf("Tamaño del buffer: %d lecturas\n", tamBuffer);

<<<<<<< HEAD
=======
    // Crea el hilo procesador, encargado de sacar datos del buffer y procesarlos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (pthread_create(&procesador, NULL, hiloProcesador, &contexto) != 0) {
        printf("Error creando Hilo Procesador.\n");

        destruirBuffer(&contexto.buffer);
        fclose(archivoConsolidado);
        sem_close(semaforo);
        sem_unlink(NOMBRE_SEMAFORO);

        return 1;
    }

<<<<<<< HEAD
=======
    // Crea el hilo recolector, encargado de leer datos desde el pipe.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
    // Espera a que ambos hilos terminen.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    pthread_join(recolector, NULL);
    pthread_join(procesador, NULL);

    fclose(archivoConsolidado);

    printf("\nArchivo consolidado creado: %s\n", ARCHIVO_CONSOLIDADO);

<<<<<<< HEAD
    imprimirResumen(contexto.estadisticas);
    imprimirCategorias(contexto.conteoCategorias);

=======
    // Muestra los resultados finales.
    imprimirResumen(contexto.estadisticas);
    imprimirCategorias(contexto.conteoCategorias);

    // Libera recursos utilizados por el programa.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
    // Procesa las opciones -b y -p recibidas por consola.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
    // Valida que el tamaño del buffer y el nombre del pipe sean correctos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (*tamBuffer <= 0 || pipeNombre == NULL) {
        return -1;
    }

<<<<<<< HEAD
=======
    // Construye la ruta completa del pipe dentro de /tmp.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    int resultado = snprintf(rutaPipe, MAX_RUTA_PIPE, "/tmp/%s", pipeNombre);

    if (resultado < 0 || resultado >= MAX_RUTA_PIPE) {
        printf("Error: nombre de pipe demasiado largo.\n");
        return -1;
    }

    return 0;
}

int crearPipeNominal(const char *rutaPipe) {
<<<<<<< HEAD
    if (mkfifo(rutaPipe, 0666) == -1) {
=======
    // Crea un FIFO para comunicación entre procesos.
    if (mkfifo(rutaPipe, 0666) == -1) {
        // Si ya existe, no se considera error.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
        if (errno != EEXIST) {
            perror("Error creando pipe nominal");
            return -1;
        }
    }

    return 0;
}

void inicializarBuffer(BufferEstaciones *buffer, int capacidad) {
<<<<<<< HEAD
=======
    // Reserva memoria para almacenar las lecturas del buffer.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    buffer->datos = malloc(sizeof(Estacion) * capacidad);

    if (buffer->datos == NULL) {
        perror("Error reservando memoria para el buffer");
        exit(1);
    }

<<<<<<< HEAD
=======
    // Inicializa los valores del buffer circular.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    buffer->capacidad = capacidad;
    buffer->inicio = 0;
    buffer->fin = 0;
    buffer->cantidad = 0;
    buffer->terminado = 0;

<<<<<<< HEAD
=======
    // Inicializa herramientas de sincronización entre hilos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->noLleno, NULL);
    pthread_cond_init(&buffer->noVacio, NULL);
}

void destruirBuffer(BufferEstaciones *buffer) {
<<<<<<< HEAD
=======
    // Libera memoria y destruye mecanismos de sincronización.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    free(buffer->datos);

    pthread_mutex_destroy(&buffer->mutex);
    pthread_cond_destroy(&buffer->noLleno);
    pthread_cond_destroy(&buffer->noVacio);
}

void insertarBuffer(BufferEstaciones *buffer, Estacion estacion) {
    pthread_mutex_lock(&buffer->mutex);

<<<<<<< HEAD
=======
    // Si el buffer está lleno, el hilo espera hasta que haya espacio.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    while (buffer->cantidad == buffer->capacidad) {
        pthread_cond_wait(&buffer->noLleno, &buffer->mutex);
    }

<<<<<<< HEAD
=======
    // Inserta la lectura en el buffer circular.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    buffer->datos[buffer->fin] = estacion;
    buffer->fin = (buffer->fin + 1) % buffer->capacidad;
    buffer->cantidad++;

<<<<<<< HEAD
=======
    // Avisa al procesador que ya hay datos disponibles.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    pthread_cond_signal(&buffer->noVacio);
    pthread_mutex_unlock(&buffer->mutex);
}

bool sacarBuffer(BufferEstaciones *buffer, Estacion *estacion) {
    pthread_mutex_lock(&buffer->mutex);

<<<<<<< HEAD
=======
    // Si el buffer está vacío, espera a que lleguen datos o a que finalice.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    while (buffer->cantidad == 0 && !buffer->terminado) {
        pthread_cond_wait(&buffer->noVacio, &buffer->mutex);
    }

<<<<<<< HEAD
=======
    // Si ya no hay datos y el recolector terminó, se finaliza el procesamiento.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (buffer->cantidad == 0 && buffer->terminado) {
        pthread_mutex_unlock(&buffer->mutex);
        return false;
    }

<<<<<<< HEAD
=======
    // Extrae una lectura del buffer circular.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    *estacion = buffer->datos[buffer->inicio];
    buffer->inicio = (buffer->inicio + 1) % buffer->capacidad;
    buffer->cantidad--;

<<<<<<< HEAD
=======
    // Avisa al recolector que hay espacio disponible.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    pthread_cond_signal(&buffer->noLleno);
    pthread_mutex_unlock(&buffer->mutex);

    return true;
}

void finalizarBuffer(BufferEstaciones *buffer) {
    pthread_mutex_lock(&buffer->mutex);

<<<<<<< HEAD
    buffer->terminado = 1;

=======
    // Marca que ya no se insertarán más datos.
    buffer->terminado = 1;

    // Despierta al procesador si estaba esperando datos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    pthread_cond_broadcast(&buffer->noVacio);

    pthread_mutex_unlock(&buffer->mutex);
}

void *hiloRecolector(void *arg) {
    MonitorContext *contexto = (MonitorContext *) arg;

<<<<<<< HEAD
=======
    // Abre el pipe en modo lectura.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    int fdPipe = open(contexto->rutaPipe, O_RDONLY);

    if (fdPipe == -1) {
        perror("Error abriendo pipe en Hilo Recolector");
        finalizarBuffer(&contexto->buffer);
        pthread_exit(NULL);
    }

<<<<<<< HEAD
=======
    // Convierte el descriptor del pipe a FILE* para leer con fgets.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    FILE *pipe = fdopen(fdPipe, "r");

    if (pipe == NULL) {
        perror("Error convirtiendo pipe a FILE en Hilo Recolector");
        close(fdPipe);
        finalizarBuffer(&contexto->buffer);
        pthread_exit(NULL);
    }

    char linea[MAX_LINEA];

<<<<<<< HEAD
=======
    // Lee cada línea recibida desde el pipe.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    while (fgets(linea, sizeof(linea), pipe) != NULL) {
        linea[strcspn(linea, "\n")] = '\0';

        Estacion estacion;

<<<<<<< HEAD
=======
        // Convierte la línea CSV en una estructura Estacion.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
        // Envía la lectura al buffer para que el procesador la consuma.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
        insertarBuffer(&contexto->buffer, estacion);
    }

    fclose(pipe);

<<<<<<< HEAD
=======
    // Indica que ya no llegarán más lecturas.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    finalizarBuffer(&contexto->buffer);

    pthread_exit(NULL);
}

void *hiloProcesador(void *arg) {
    MonitorContext *contexto = (MonitorContext *) arg;

    Estacion estacion;

<<<<<<< HEAD
=======
    // Procesa lecturas mientras existan datos en el buffer.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    while (sacarBuffer(&contexto->buffer, &estacion)) {
        printf(
            "[Procesador] Procesando: %s,%d,%d,%d,%s\n",
            estacion.nombreEstacion,
            estacion.humedad,
            estacion.rocio,
            estacion.presion,
            estacion.hora
        );

<<<<<<< HEAD
=======
        // Guarda la lectura en el archivo consolidado.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
        // Actualiza estadísticas y categorías meteorológicas.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
        actualizarEstadisticas(&contexto->estadisticas, estacion);
        clasificarLectura(estacion, &contexto->conteoCategorias);
    }

    pthread_exit(NULL);
}

int convertirLineaAEstacion(const char *linea, Estacion *estacion) {
    char copia[MAX_LINEA];

<<<<<<< HEAD
    strncpy(copia, linea, sizeof(copia));
    copia[sizeof(copia) - 1] = '\0';

=======
    // Se copia la línea porque strtok modifica el texto original.
    strncpy(copia, linea, sizeof(copia));
    copia[sizeof(copia) - 1] = '\0';

    // Se separa la línea usando comas.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    char *nombre = strtok(copia, ",");
    char *humedad = strtok(NULL, ",");
    char *rocio = strtok(NULL, ",");
    char *presion = strtok(NULL, ",");
    char *hora = strtok(NULL, ",");

<<<<<<< HEAD
=======
    // Valida que la línea tenga todos los campos esperados.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (nombre == NULL ||
        humedad == NULL ||
        rocio == NULL ||
        presion == NULL ||
        hora == NULL) {

        return 0;
    }

<<<<<<< HEAD
=======
    // Copia los datos de texto y convierte los valores numéricos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    strncpy(estacion->nombreEstacion, nombre, sizeof(estacion->nombreEstacion));
    estacion->nombreEstacion[sizeof(estacion->nombreEstacion) - 1] = '\0';

    estacion->humedad = atoi(humedad);
    estacion->rocio = atoi(rocio);
    estacion->presion = atoi(presion);
<<<<<<< HEAD
    
    bool valido = (estacion->humedad >= 0 && estacion->humedad <= 100) &&
                   (estacion->rocio >= 0 && estacion->rocio <= 100) &&
                   (estacion->presion > 0);
    if (valido) {
         strncpy(estacion->hora, hora, sizeof(estacion->hora));
    estacion->hora[sizeof(estacion->hora) - 1] = '\0';
    }
    else {
        printf("Valores fuera de rango en línea: %s\n", linea);
        return 0;
    }          
   
=======

    strncpy(estacion->hora, hora, sizeof(estacion->hora));
    estacion->hora[sizeof(estacion->hora) - 1] = '\0';
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856

    return 1;
}

void inicializarEstadisticas(Estadisticas *estadisticas) {
<<<<<<< HEAD
=======
    // Inicializa acumuladores y valores de control.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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
<<<<<<< HEAD
=======
    // En la primera lectura, se inicializan mínimos y máximos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (estadisticas->cantidad == 0) {
        estadisticas->minHumedad = estacion.humedad;
        estadisticas->maxHumedad = estacion.humedad;

        estadisticas->minRocio = estacion.rocio;
        estadisticas->maxRocio = estacion.rocio;

        estadisticas->minPresion = estacion.presion;
        estadisticas->maxPresion = estacion.presion;
    }

<<<<<<< HEAD
=======
    // Acumula valores para calcular promedios.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    estadisticas->sumaHumedad += estacion.humedad;
    estadisticas->sumaRocio += estacion.rocio;
    estadisticas->sumaPresion += estacion.presion;

<<<<<<< HEAD
=======
    // Actualiza mínimos y máximos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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
<<<<<<< HEAD
=======
    // Evita dividir entre cero si no se recibieron datos.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    if (estadisticas.cantidad == 0) {
        printf("No se recibieron datos para calcular estadísticas.\n");
        return;
    }

<<<<<<< HEAD
=======
    // Calcula promedios.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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
<<<<<<< HEAD
=======
    // Inicializa el contador de cada categoría.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
    conteo->lluvioso = 0;
    conteo->nublado = 0;
    conteo->fresco = 0;
    conteo->sinCategoria = 0;
}

void clasificarLectura(Estacion estacion, ConteoCategorias *conteo) {
<<<<<<< HEAD
=======
    // Clasifica la lectura según humedad, rocío y presión.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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

<<<<<<< HEAD
=======
    // Determina la categoría más frecuente entre lluvioso, nublado y fresco.
>>>>>>> 10e3b3a45640932494809e5e424359ad9bd4f856
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