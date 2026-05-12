# Proyecto Final SO — Monitor y Agentes de Mediciones

Este proyecto implementa un sistema concurrente en C para enviar mediciones de estaciones meteorológicas desde varios procesos **Agente** hacia un proceso **Monitor** usando un **pipe nominal**.

El Monitor recibe las lecturas, las almacena temporalmente en un buffer compartido manejado por hilos, procesa los datos y genera un archivo consolidado con las mediciones recibidas.

---

## Estructura del proyecto

```text
.
├── docs
│   ├── archivo-consolidado.cvs
│   ├── documentoTexto2.cvs
│   ├── documentoTexto3.cvs
│   ├── documentoTexto.cvs
│   ├── fresco.cvs
│   ├── lluvioso.cvs
│   ├── mixto.cvs
│   ├── nublado.cvs
│   ├── sin_categoria.cvs
│   └── varias_estaciones.cvs
├── LICENSE
├── Makefile
├── README.md
└── src
    ├── agenteMediciones.c
    ├── agenteMediciones.h
    ├── estacion.h
    ├── monitor.c
    └── monitor.h
```

---

## Descripción general

El sistema está compuesto por dos programas principales:

### Monitor

El Monitor es el primer proceso que debe ejecutarse.

Sus responsabilidades son:

- Crear el pipe nominal.
- Crear un buffer de tamaño configurable.
- Crear el Hilo Recolector.
- Crear el Hilo Procesador.
- Recibir las lecturas enviadas por los agentes.
- Guardar las mediciones recibidas en `docs/archivo-consolidado.cvs`.
- Calcular estadísticas de humedad, rocío y presión.
- Clasificar las lecturas como lluvioso, nublado, fresco o sin categoría.

### Agente

El Agente lee un archivo `.cvs` con mediciones de una estación y envía cada línea al Monitor a través del pipe nominal.

Sus responsabilidades son:

- Leer un archivo de mediciones desde la carpeta `docs`.
- Enviar una lectura cada cierto número de segundos.
- Usar el mismo pipe nominal creado por el Monitor.
- Permitir que varios agentes se ejecuten al mismo tiempo.

---

## Formato de los archivos `.cvs`

Cada archivo de entrada debe tener líneas con el siguiente formato:

```text
nombreEstacion,humedad,rocio,presion,hora
```

Ejemplo:

```text
EK,90,9,750,08:00:00
EK,90,9,750,09:01:00
EK,89,8,749,10:00:00
EK,79,4,720,11:00:00
.
```

El carácter `.` indica el final del archivo de mediciones.

---

## Compilación

Desde la raíz del proyecto, ejecutar:

```bash
make
```

Esto genera los ejecutables:

```text
./monitor
./agente
```

Para limpiar los archivos generados:

```bash
make clean
```

---

## Ejecución

Primero debe ejecutarse el Monitor:

```bash
./monitor -b 2 -p ola
```

Donde:

- `-b 2` indica que el buffer tendrá capacidad para 2 lecturas.
- `-p ola` indica que el pipe nominal se llamará `ola`.

Internamente, el pipe se crea en:

```text
/tmp/ola
```

Luego, en otra terminal, se ejecuta un Agente:

```bash
./agente -f lluvioso.cvs -t 1 -p ola
```

Donde:

- `-f lluvioso.cvs` indica el archivo de mediciones.
- `-t 1` indica que el agente espera 1 segundo entre cada lectura enviada.
- `-p ola` indica el nombre del pipe nominal usado por el Monitor.

El agente buscará automáticamente el archivo dentro de la carpeta:

```text
docs/
```

Por ejemplo:

```bash
./agente -f lluvioso.cvs -t 1 -p ola
```

lee realmente:

```text
docs/lluvioso.cvs
```

---

## Ejecución con varios agentes

Para probar concurrencia, se pueden ejecutar varios agentes al mismo tiempo en diferentes terminales:

```bash
./agente -f lluvioso.cvs -t 1 -p ola
```

```bash
./agente -f fresco.cvs -t 1 -p ola
```

```bash
./agente -f nublado.cvs -t 1 -p ola
```

También se pueden lanzar en segundo plano desde una sola terminal:

```bash
./agente -f lluvioso.cvs -t 1 -p ola &
./agente -f fresco.cvs -t 1 -p ola &
./agente -f nublado.cvs -t 1 -p ola &
```

---

## Salida del Monitor

El Monitor muestra por terminal las lecturas recibidas y procesadas.

También imprime un resumen con:

- Cantidad de lecturas procesadas.
- Promedio de humedad.
- Mínimo y máximo de humedad.
- Promedio de rocío.
- Mínimo y máximo de rocío.
- Promedio de presión.
- Mínimo y máximo de presión.
- Categoría predominante.

Ejemplo de salida:

```text
===== RESUMEN DE MEDICIONES =====
Cantidad de lecturas procesadas: 4

Humedad:
  Promedio: 87.00
  Mínimo: 79
  Máximo: 90

Rocío:
  Promedio: 7.50
  Mínimo: 4
  Máximo: 9

Presión:
  Promedio: 742.25
  Mínimo: 720
  Máximo: 750
=================================
```

---

## Archivo consolidado

El Monitor genera el archivo:

```text
docs/archivo-consolidado.cvs
```

Este archivo contiene las lecturas recibidas por el Monitor.

Ejemplo:

```text
tipo,nombreEstacion,humedad,rocio,presion,hora
LECTURA,EK,90,9,750,08:00:00
LECTURA,EK,90,9,750,09:01:00
LECTURA,EK,89,8,749,10:00:00
LECTURA,EK,79,4,720,11:00:00
```

---

## Reglas de categorización

El Monitor clasifica las lecturas usando las siguientes reglas:

### Lluvioso

```text
humedad > 90
rocio > 9
presion < 750
```

### Nublado

```text
80 <= humedad <= 95
rocio > 8
presion == 751
```

### Fresco

```text
humedad < 80
5 <= rocio <= 8
presion > 754
```

Si una lectura no cumple ninguna regla, se clasifica como `sin categoría`.

---

## Limpieza manual del pipe

Si el programa se cierra de forma inesperada, puede quedar un pipe creado en `/tmp`.

Para eliminarlo manualmente:

```bash
rm /tmp/ola
```

También se puede revisar si existe con:

```bash
ls -l /tmp/ola
```

---

## Notas importantes

- El Monitor debe ejecutarse antes que los agentes.
- Todos los agentes deben usar el mismo nombre de pipe que el Monitor.
- Los archivos `.cvs` deben estar dentro de la carpeta `docs`.
- El proyecto debe ejecutarse desde la raíz del repositorio.
- El Monitor usa hilos para manejar el buffer compartido.
- Los agentes son procesos independientes que pueden ejecutarse concurrentemente.