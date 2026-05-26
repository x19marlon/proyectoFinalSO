# Proyecto Final de Sistemas Operativos: Sistema de Monitoreo Meteorológico Concurrente

Este proyecto implementa una solución robusta y concurrente en lenguaje **C** bajo estándares **POSIX** para la recolección, buffering, procesamiento y consolidación de datos meteorológicos provenientes de múltiples estaciones de medición distribuidas.

El sistema ilustra conceptos fundamentales de **Sistemas Operativos (SO)** como:
*   **Comunicación Entre Procesos (IPC)** mediante Tuberías Nominales (FIFOs).
*   **Sincronización Concurrente Multi-Proceso** utilizando Semáforos Nombrados de POSIX.
*   **Programación Multi-Hilo** con la librería `pthread`.
*   **Problema del Productor-Consumidor** con Buffer Circular Compartido, Mutex (`pthread_mutex_t`) y Variables de Condición (`pthread_cond_t`).
*   **Mecanismos de Control de Flujo e Contrapresión (Backpressure)** entre procesos.

---

## 1. Arquitectura General del Sistema

El sistema sigue una arquitectura distribuida de tipo **Multi-Productor (Agentes) y Consumidor Único (Monitor)**, estructurada de la siguiente manera:

```mermaid
graph TD
    subgraph Procesos Agentes (Productores)
        A1[Agente 1: Lluvioso] -->|Escribe línea CSV| SEM[Semáforo: /sem_pipe_monitor]
        A2[Agente 2: Nublado] -->|Escribe línea CSV| SEM
        A3[Agente N: Fresco] -->|Escribe línea CSV| SEM
    end

    SEM -->|Sección Crítica de Escritura| FIFO[Pipe Nominal: /tmp/pipe_name]

    subgraph Proceso Monitor (Consumidor)
        FIFO -->|Lectura Bloqueante| HR[Hilo Recolector]
        HR -->|insertarBuffer| BC((Buffer Circular Compartido))
        BC -->|sacarBuffer| HP[Hilo Procesador]
        HP -->|Escribe| AC[docs/archivo-consolidado.cvs]
        HP -->|Calcula| ST[Estadísticas en Memoria]
    end

    style SEM fill:#f9f,stroke:#333,stroke-width:2px
    style FIFO fill:#bbf,stroke:#333,stroke-width:2px
    style BC fill:#bfb,stroke:#333,stroke-width:2px
```

### Componentes Principales:

1.  **Procesos Agentes (Productores):**
    Procesos independientes que se ejecutan en su propio espacio de memoria. Cada agente lee un archivo `.cvs` local que simula sensores meteorológicos, analiza línea por línea cada medición, y las envía a través del canal de comunicación a intervalos regulares parametrizables (`-t segundos`).

2.  **Canal IPC (Pipe Nominal / FIFO):**
    Un archivo especial en el sistema de archivos Linux (`/tmp/...`) que actúa como una cola FIFO (First-In, First-Out) de bytes administrada por el kernel. Permite que procesos no emparentados intercambien datos de forma segura.

3.  **Mecanismo de Mutua Exclusión (Semáforo Nombrado):**
    Un semáforo POSIX compartido (`/sem_pipe_monitor`) que garantiza que solo **un agente a la vez** pueda escribir en la tubería nominal. Esto evita que múltiples escrituras concurrentes intercalen sus bytes y corrompan el formato del stream.

4.  **Proceso Monitor (Consumidor):**
    El corazón del sistema. Es un proceso multi-hilo que inicializa el entorno (crea el pipe, crea el semáforo y lanza los hilos de control). Cuenta con:
    *   **Hilo Recolector:** Lee continuamente desde el pipe nominal, deserializa la cadena en una estructura C (`Estacion`) e introduce los datos en el buffer circular compartido.
    *   **Buffer Circular Compartido (Memoria Compartida del Proceso):** Un buffer en memoria dinámica con capacidad limitada que almacena temporalmente las lecturas listas para ser procesadas.
    *   **Hilo Procesador:** Consume datos del buffer circular, realiza cálculos de agregación estadística (promedios, mínimos, máximos), clasifica las lecturas según reglas meteorológicas preestablecidas y las escribe de manera ordenada en el archivo de salida consolidado.

---

## 2. Detalle de Archivos del Proyecto

El código está estructurado de manera modular y limpia en el directorio `src/`:

### `estacion.h`
Define la estructura de datos que representa una lectura meteorológica individual:
```c
typedef struct {
    char nombreEstacion[50]; // Identificador de la estación
    int humedad;             // Valor porcentual (0-100)
    int rocio;               // Temperatura de rocío
    int presion;             // Presión atmosférica en hPa
    char hora[20];           // Marca de tiempo en formato "HH:MM:SS"
} Estacion;
```

### `agenteMediciones.h` y `agenteMediciones.c`
Contiene la lógica del proceso Agente:
*   **`leerCSV`**: Abre el archivo de datos meteorológicos y analiza las líneas. Ignora líneas mal formateadas, detiene la lectura ante el carácter de fin lógico `.` o al alcanzar el límite físico `MAX_ESTACIONES`.
*   **`enviarLecturaPorPipe`**: Abre la tubería nominal para escritura (`O_WRONLY`), localiza y abre el semáforo compartido, y recorre las mediciones leídas. Para cada una:
    1.  Formatea la estructura en una cadena de texto estructurada en CSV.
    2.  Llama a `sem_wait()` para adquirir exclusión mutua.
    3.  Escribe el mensaje en el descriptor del pipe nominal (`write`).
    4.  Llama a `sem_post()` para liberar el semáforo.
    5.  Duerme (`sleep`) el tiempo indicado por el usuario antes de procesar la siguiente lectura.

### `monitor.h` y `monitor.c`
Contiene la lógica principal del Monitor:
*   **`inicializarBuffer` / `destruirBuffer`**: Reserva y libera memoria dinámica para la estructura circular y configura el mutex junto a las variables de condición.
*   **`insertarBuffer` / `sacarBuffer`**: Implementan el algoritmo del productor-consumidor clásico utilizando un mutex para exclusión mutua y variables de condición para evitar la espera activa.
*   **`hiloRecolector`**: Abre el pipe nominal en modo lectura (`O_RDONLY`), convierte el descriptor de archivo a un stream formateado (`fdopen`), lee líneas mediante `fgets`, las convierte a estructuras C con `convertirLineaAEstacion` y las inserta al buffer. Al terminar la tubería (EOF), invoca a `finalizarBuffer` para señalizar el cierre.
*   **`hiloProcesador`**: Extrae elementos del buffer circular. Por cada lectura extraída:
    1.  La escribe en `docs/archivo-consolidado.cvs` garantizando consistencia mediante `fflush`.
    2.  Llama a `actualizarEstadisticas` para actualizar la sumatoria, máximos y mínimos de humedad, rocío y presión.
    3.  Llama a `clasificarLectura` para categorizar la lectura dentro de las variables contadoras globales de meteorología.
*   **`imprimirResumen` y `imprimirCategorias`**: Muestran en consola el reporte consolidado agregando métricas acumuladas y detectando la tendencia climática predominante.

---

## 3. Mecanismos de Concurrencia y Sincronización (Detalle Técnico)

Esta sección detalla de forma exhaustiva los pilares de sistemas operativos implementados, ideal para la sustentación:

### A. Tuberías Nominales (Named Pipes / FIFOs)
Un **Pipe Nominal** es un canal de comunicación unidireccional permanente en el sistema de archivos (se crea con `mkfifo`).
*   **Bloqueo en la apertura:** Por defecto, abrir una FIFO para lectura (`O_RDONLY`) se bloquea en el kernel del sistema operativo hasta que otro proceso la abra para escritura (`O_WRONLY`), y viceversa. Esto permite sincronizar de forma natural el inicio del Monitor y los Agentes: el Hilo Recolector no avanzará hasta que el primer Agente inicie y abra la tubería.
*   **Garantías de Lectura/Escritura:** Linux provee una cola interna en el kernel de tamaño fijo (típicamente 64 KB). Si la cola se llena, las escrituras de los agentes se bloquean automáticamente. Si la cola está vacía, las lecturas en el monitor se bloquean. Esto genera un mecanismo automático de control de flujo.

### B. Semáforos Nombrados de POSIX
Dado que múltiples agentes independientes (procesos separados con distintos espacios de direccionamiento) escriben concurrentemente en el mismo Pipe, existe un riesgo crítico de **carrera de datos (race conditions)**: si dos agentes escriben simultáneamente, sus bytes se podrían mezclar en el stream del pipe, corrompiendo la medición.
*   Para solucionar esto, se implementa un **Semáforo Binario Nombrado** (`/sem_pipe_monitor`) inicializado con un valor de **1** (que actúa como un Mutex multi-proceso).
*   Antes de escribir en el pipe, el Agente invoca `sem_wait()`. Si el valor es 1, se decrementa a 0 y el agente entra a su sección crítica. Si el valor es 0, el agente se bloquea en el planificador del SO.
*   Tras escribir la línea completa y el salto de línea `\n`, el agente llama a `sem_post()`, incrementando el semáforo y despertando a cualquier otro agente en espera. Esto garantiza la **atomaticidad** de cada lectura enviada al pipe.

### C. El Buffer Circular Compartido en el Monitor
El buffer en el Monitor se comparte entre el **Hilo Recolector** (Productor) y el **Hilo Procesador** (Consumidor). Esta estructura evita cuellos de botella y desacopla la velocidad de red/disco del procesamiento interno:

```c
typedef struct {
    Estacion *datos;      // Arreglo dinámico de lecturas
    int capacidad;       // Tamaño máximo del buffer (indicado por `-b`)
    int inicio;          // Índice para extraer (Consumidor)
    int fin;             // Índice para insertar (Productor)
    int cantidad;        // Cantidad de elementos actuales en el buffer
    int terminado;       // Bandera booleana que indica fin de transmisión
    
    pthread_mutex_t mutex;     // Garantiza exclusión mutua al modificar el buffer
    pthread_cond_t noLleno;    // Señal para el Productor cuando el buffer se libera
    pthread_cond_t noVacio;    // Señal para el Consumidor cuando entran nuevos datos
} BufferEstaciones;
```

#### Flujo detallado del Productor (`insertarBuffer`):
1.  Adquiere el cerrojo del mutex: `pthread_mutex_lock(&buffer->mutex);`
2.  Evalúa en un bucle `while` si el buffer está lleno (`cantidad == capacidad`). Si está lleno, se suspende liberando el mutex mediante `pthread_cond_wait(&buffer->noLleno, &buffer->mutex);`.
3.  Una vez despertado (porque se extrajo un elemento) y habiendo re-adquirido el mutex, inserta la lectura en `datos[fin]`.
4.  Avanza el puntero de escritura de forma circular: `fin = (fin + 1) % capacidad`.
5.  Incrementa la variable de control `cantidad++`.
6.  Notifica al consumidor que el buffer ya no está vacío: `pthread_cond_signal(&buffer->noVacio);`.
7.  Libera el mutex: `pthread_mutex_unlock(&buffer->mutex);`.

#### Flujo detallado del Consumidor (`sacarBuffer`):
1.  Adquiere el mutex: `pthread_mutex_lock(&buffer->mutex);`.
2.  Evalúa si el buffer está vacío (`cantidad == 0`) y no se ha marcado el fin de la ejecución (`!terminado`). Si se cumple, se bloquea durmiendo en `pthread_cond_wait(&buffer->noVacio, &buffer->mutex);`.
3.  Si el buffer se vacía definitivamente y se levantó la bandera `terminado`, libera el mutex y retorna `false` indicando que no hay más datos por consumir.
4.  Si hay datos, extrae la lectura de `datos[inicio]`.
5.  Avanza de manera circular el puntero: `inicio = (inicio + 1) % capacidad`.
6.  Decrementa `cantidad--`.
7.  Notifica al recolector que hay espacio disponible: `pthread_cond_signal(&buffer->noLleno);`.
8.  Libera el mutex y retorna `true`.

---

## 4. Guía de Compilación y Ejecución

El proyecto incluye un `Makefile` robusto para automatizar los procesos de construcción:

### Compilación:
En la raíz del proyecto, ejecute:
```bash
make
```
Esto creará el directorio `build/` con los archivos objeto e instanciará los dos binarios listos para ejecutarse:
*   `./monitor`
*   `./agente`

Para limpiar los binarios y archivos temporales compilados:
```bash
make clean
```

### Protocolo de Ejecución Paso a Paso:

**Paso 1: Iniciar el Monitor**
El monitor debe inicializarse **primero** para crear la tubería y preparar los descriptores e hilos.
```bash
./monitor -b 4 -p canalMeteorologico
```
*   `-b 4`: Inicializa el buffer circular en memoria con capacidad para 4 lecturas concurrentes.
*   `-p canalMeteorologico`: Crea la tubería nominal en `/tmp/canalMeteorologico`.

**Paso 2: Iniciar múltiples Agentes Concurrentes**
En terminales separadas, ejecute diferentes instancias de agentes lectores para simular sensores paralelos:

```bash
# Terminal 2 - Simula sensor en zona lluviosa enviando datos cada 1 segundo
./agente -f lluvioso.cvs -t 1 -p canalMeteorologico

# Terminal 3 - Simula sensor en zona fresca enviando datos cada 2 segundos
./agente -f fresco.cvs -t 2 -p canalMeteorologico

# Terminal N - Simula sensor genérico enviando datos cada 3 segundos
./agente -f varias_estaciones.cvs -t 3 -p canalMeteorologico
```

*(Nota: Los agentes buscarán por defecto los archivos indicados bajo la carpeta `docs/` en la raíz del proyecto).*

---

## 5. Respuestas de Oro para la Sustentación Académica 🎓

A continuación se listan las preguntas más probables que te harán los evaluadores durante la defensa del proyecto final, junto con sus respuestas técnicas detalladas y fundamentadas en teoría de Sistemas Operativos:

### P1: ¿Por qué es necesario usar un semáforo nombrado (`/sem_pipe_monitor`) si los pipes nominales ya coordinan la lectura y escritura?
**Respuesta:**
Las tuberías nominales (FIFOs) garantizan consistencia a nivel de bytes, pero no imponen atomicidad a nivel de líneas o paquetes estructurados si múltiples procesos independientes escriben a la vez.
Si el Agente A escribe `"EK,90,9,750,08:00:00\n"` y el Agente B escribe `"BOG,70,4,755,08:01:00\n"` en el mismo pipe al mismo tiempo, el stream podría intercalarse en el kernel y recibir algo corrupto como `"EK,90,BOG,70,9,750... \n"`.
El semáforo nombrado actúa como un cerrojo de exclusión mutua compartido a nivel de sistema operativo global. Garantiza que toda la cadena formateada de un agente se escriba de manera **atómica** e indivisible en el buffer del kernel del pipe antes de permitir que otro proceso comience su escritura.

### P2: ¿Qué sucede en el sistema si el buffer circular compartido se llena por completo (`cantidad == capacidad`)? Explica el flujo de bloqueo hacia atrás (Backpressure).
**Respuesta:**
Si el buffer circular se llena, ocurre un efecto dominó muy interesante llamado contrapresión o *backpressure*:
1.  El **Hilo Recolector** intenta hacer `insertarBuffer`. Al evaluar `cantidad == capacidad`, se bloquea en la llamada `pthread_cond_wait(&buffer->noLleno, &buffer->mutex)`, liberando el mutex.
2.  Dado que el Hilo Recolector está bloqueado y no consume datos del pipe nominal, el Hilo Recolector deja de ejecutar llamadas a `fgets` (o `read`) sobre la tubería `/tmp/canalMeteorologico`.
3.  El buffer de la tubería nominal en el kernel del sistema operativo (que típicamente mide 64 KB en Linux) comienza a llenarse rápidamente con los datos de los Agentes.
4.  Cuando el buffer de la tubería del kernel se llena por completo, cualquier llamada `write()` subsiguiente realizada por un Agente se bloqueará en el espacio del kernel del sistema operativo.
5.  Los procesos de los Agentes se suspenden automáticamente por el planificador del sistema operativo, liberando la CPU y deteniendo el consumo de energía y procesamiento hasta que el Hilo Procesador en el Monitor consuma un elemento del buffer circular, despierte al Recolector y este vuelva a leer del pipe, drenando la tubería y liberando a los agentes.

### P3: ¿Por qué es fundamental que la condición del buffer circular se verifique en un bucle `while` en lugar de una estructura `if`?
**Respuesta:**
Es un requisito de seguridad crítico debido a los **despertares espurios** (spurious wakeups) y el comportamiento multi-hilo de POSIX.
Cuando un hilo despierta de `pthread_cond_wait`, no se le garantiza que la condición física siga siendo verdadera en ese instante preciso.
Si tuviéramos múltiples hilos consumidores, un hilo procesador podría despertarse por una señal de `noVacio`, pero antes de que adquiera el mutex, otro hilo procesador que ya estaba listo podría adelantarse, tomar el mutex, extraer la única lectura del buffer y decrementar `cantidad` a 0.
Si usáramos un `if`, el primer hilo reanudaría su ejecución suponiendo que hay datos e intentaría leer de un buffer vacío, corrompiendo la memoria o causando una violación de segmento. El bucle `while` obliga al hilo a re-evaluar la condición física del recurso una vez despierto y asegurar que la precondición se mantenga antes de proceder.

### P4: ¿Cuál es la diferencia entre usar tuberías con nombre (FIFOs) y tuberías sin nombre (pipes comunes) en Unix? ¿Por qué se seleccionó FIFO aquí?
**Respuesta:**
*   Las **Tuberías Sin Nombre** (`pipe()`) se implementan puramente en la memoria del kernel y carecen de un descriptor en el sistema de archivos. Por ende, solo pueden comunicar procesos emparentados que comparten descriptores de archivos a través de un ancestro común mediante la llamada al sistema `fork()`.
*   Las **Tuberías Con Nombre** o **Nominales** (`mkfifo()`) tienen una entrada y presencia física en el sistema de archivos (se representan con el tipo de archivo `p`). Esto permite que procesos completamente ajenos e independientes (como lo son nuestros Agentes y el Monitor), iniciados en terminales distintas y en diferentes instantes de tiempo, puedan abrir el archivo especial por su ruta y establecer un canal IPC común.

### P5: ¿Cómo maneja el Monitor el cierre ordenado de sus hilos una vez que todos los agentes han terminado de enviar datos?
**Respuesta:**
El cierre se maneja mediante la detección de **EOF (End Of File)** en el canal de comunicación:
1.  Cuando todos los agentes cierran sus descriptores de escritura de la FIFO nominal, el extremo de lectura del pipe en el Monitor recibe un fin de archivo (EOF). Esto provoca que la llamada `fgets` en el **Hilo Recolector** retorne `NULL`.
2.  Al salir del bucle de lectura, el Hilo Recolector cierra el stream de la tubería e invoca a la función `finalizarBuffer()`.
3.  `finalizarBuffer()` establece la variable booleana `buffer->terminado = 1` y realiza un `pthread_cond_broadcast(&buffer->noVacio)`. Esto despierta de forma forzada a cualquier hilo consumidor que estuviese dormido en la variable de condición.
4.  El **Hilo Procesador** despierta. Al entrar a `sacarBuffer`, la función detecta que `cantidad == 0` y `terminado == 1`, liberando el mutex inmediatamente y retornando `false`.
5.  El Hilo Procesador sale de su bucle de procesamiento y termina pacíficamente llamando a `pthread_exit()`.
6.  El hilo principal de ejecución (`main`), que estaba esperando en `pthread_join(recolector, ...)` y `pthread_join(procesador, ...)`, se desbloquea, ejecuta las rutinas de liberación de memoria dinámica y cierra ordenadamente el semáforo y la tubería.

### P6: Si quisieras escalar este proyecto para soportar múltiples Hilos Procesadores (Consumidores Concurrentes), ¿qué cambios tendrías que hacer en el código actual?
**Respuesta:**
¡Ninguno a nivel del Buffer Circular ni de sincronización! La arquitectura actual está diseñada de forma **completamente segura para hilos (Thread-Safe)**.
Dado que la inserción (`insertarBuffer`) y la extracción (`sacarBuffer`) están protegidas de principio a fin por un mutex (`buffer->mutex`) y utilizan correctamente variables de condición estructuradas en bucles `while`, múltiples consumidores pueden competir por vaciar el buffer sin causar condiciones de carrera.
El único cambio necesario sería en el hilo principal (`main`):
1.  Declarar un arreglo de hilos procesadores: `pthread_t procesadores[N]`.
2.  Crear los `N` hilos procesadores apuntando a la misma función `hiloProcesador` en un bucle `for`.
3.  Hacer `pthread_join` de cada uno de los `N` hilos al final del programa.
Esto permitiría distribuir de forma paralela la clasificación y procesamiento meteorológico si estuviéramos consumiendo millones de registros concurrentes en sistemas multi-núcleo.

### P7: ¿Para qué sirve y por qué es importante limpiar el semáforo con `sem_unlink` al arrancar el Monitor?
**Respuesta:**
Los semáforos nombrados de POSIX tienen **persistencia de kernel**. Esto significa que una vez creados, continúan existiendo en el sistema operativo incluso si el proceso que los creó termina o falla abruptamente (a menos que se destruyan explícitamente).
Si el Monitor sufre un fallo o es terminado forzadamente con `Ctrl+C` en una ejecución anterior sin liberar los recursos, el semáforo binario podría quedar guardado con un valor de 0 (bloqueado). En la siguiente ejecución, cualquier agente que intente escribir se bloqueará de forma indefinida en un **interbloqueo (deadlock)**.
Ejecutar `sem_unlink(NOMBRE_SEMAFORO)` al inicio del Monitor le indica al kernel del sistema operativo que destruya la instancia persistente previa del semáforo para poder instanciarlo nuevamente en un estado limpio, garantizando que comience con su valor inicializado de 1.