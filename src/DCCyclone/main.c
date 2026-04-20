#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {
    READY,
    RUNNING,
    WAITING,
    FINISHED,
    DEAD,
    NOT_ARRIVED
} State;

typedef struct {
    char name[20];
    int pid;
    int t_inicio;      
    int t_cpu_burst;      
    int n_bursts;      
    int io_wait;         
    int t_deadline;        
    State state;
    int rafagas_completadas;       
    int progreso_rafaga_actual; 
    int progreso_io_actual;   
    int quantum_consumido;       
    int t_lcpu;          
    int first_time_cpu;         
    int waiting_time;           
    int interrupciones;          
} Process;

typedef struct {
    Process **procesos; 
    int tamaño;            
    int capacidad;        
} Queue;

Queue* crear_queue(int capacidad) {
    Queue *q = (Queue*)malloc(sizeof(Queue));
    q->procesos = (Process**)malloc(sizeof(Process*) * capacidad);
    q->tamaño = 0;
    q->capacidad = capacidad;
    return q;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        printf("Error al abrir archivo de entrada\n");
        return 1;
    }

    FILE *out = fopen(argv[2], "w");
    if (!out) {
        printf("Error al abrir archivo de salida\n");
        return 1;
    }

    int q, agingThreshold, nProcesses;

    fscanf(file, "%d", &q);
    fscanf(file, "%d", &agingThreshold);
    fscanf(file, "%d", &nProcesses);

    char name[20];
    int pid, start, burstTime, burstQty, ioTime, deadline;

    for (int i = 0; i < nProcesses; i++) {
        fscanf(file, "%s %d %d %d %d %d %d",
               name, &pid, &start, &burstTime,
               &burstQty, &ioTime, &deadline);
    }

    fclose(file);

    // Completa el codigo. Exito :D
};