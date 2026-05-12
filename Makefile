CC = gcc

SRC_DIR = src
BUILD_DIR = build

CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -I$(SRC_DIR) -pthread
DOCS_DIR = docs

AGENTE_SRC = $(SRC_DIR)/agenteMediciones.c
MONITOR_SRC = $(SRC_DIR)/monitor.c

AGENTE_OBJ = $(BUILD_DIR)/agenteMediciones.o
MONITOR_OBJ = $(BUILD_DIR)/monitor.o

AGENTE_BIN = agente
MONITOR_BIN = monitor

all: dirs $(AGENTE_BIN) $(MONITOR_BIN)

dirs:
	mkdir -p $(BUILD_DIR)

$(AGENTE_BIN): $(AGENTE_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(MONITOR_BIN): $(MONITOR_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/agenteMediciones.o: $(AGENTE_SRC) $(SRC_DIR)/agenteMediciones.h $(SRC_DIR)/estacion.h
	$(CC) $(CFLAGS) -DCARPETA_DOCS=\"$(DOCS_DIR)\" -c $< -o $@

$(BUILD_DIR)/monitor.o: $(MONITOR_SRC) $(SRC_DIR)/monitor.h $(SRC_DIR)/estacion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) agente monitor

run-monitor:
	./monitor -b 2 -p ola

run-agente:
	./agente -f lluvioso.cvs -t 2 -p ola

.PHONY: all dirs clean run-monitor run-agente
